#include "BPlusTree.h"
#include "catalog.h"
#include "dtype.h"
#include "math_exp_enum.h"
#include "qep.hpp"
#include "rdbms_struct.hpp"
#include "sql_const.h"
#include <cassert>
#include <cstdio>
#include <cstring>
#include <unordered_map>

#define COLUMN_WIDTH 20

extern BPlusTree_t catalog;

static bool init_qep(qep *q);
static void do_select(qep *q);
static bool get_next_row(qep *q);
static bool iterate_begin(qep *q);
static bool iterate_next(qep *q, int n);
static bool check_predicate(qep *q);
static void print_header(qep *q);
static void print_line(int cols);
static void print_row(qep *q);
static DType *(*make_get_val(qep *q))(const char *);

typedef DType *(*get_val_fn)(const char *c);

void process_select_query(qep *q) {
  if (!init_qep(q)) {
    printf("Fail to init query execution plan\n");
    return;
  }
  do_select(q);
}

bool init_select(qep *q, get_val_fn get_val) {
	qp_col *col;
	std::string var_name;
	BPluskey_t key;
	schema_rec *schm_rec;
	catalog_rec *cat_rec;
	bool found;
	int i;
	int j;
	const char *dot_ptr;
	char *buffer;
	int n;
	char *table_alias;
	char *col_name;


	key.key_size = COL_NAME_SIZE;
	for (i = 0; i < q->select.n; i++) {
		col = &q->select.cols[i];
		col->tree->tree->setGetVal(get_val);
		col->computed_row = -1;
		for (auto it = col->tree->tree->vars.begin(); it != col->tree->tree->vars.end(); it++) {
			var_name = (*it)->get_name();
			if (q->col_alias->find(var_name) != q->col_alias->end()) continue;
			if (q->table_map->find(var_name) != q->table_map->end()) continue;
			if (dot_ptr = strchr(var_name.c_str(), '.'); dot_ptr) {
				n = var_name.length();
				buffer = (char *) malloc(n+1);
				strncpy(buffer, var_name.c_str(), n);
				buffer[n] = '\0';
				dot_ptr = strchr(buffer, '.');
				table_alias = buffer;
				col_name = (char *) dot_ptr;
				*col_name = '\0';
				col_name++;
				auto ta_it = q->table_map->find(table_alias);
				if (ta_it == q->table_map->end()) {
					printf("undefined table alias for %s\n", var_name.c_str());
					return false;
				}
				j = ta_it->second.tableIdx;
				key.key = col_name;
				cat_rec = q->join.tables[j].c_rec;
				schm_rec = (schema_rec *)BPlusTree_Query_Key(cat_rec->schema_table, &key);
				if (!schm_rec) {
					printf("no col %s on table %s\n", col_name, cat_rec->table_name);
					return false;
				}
				q->table_map->insert({var_name, (data_src) {.tableIdx=j, .offset=schm_rec->offset, .dtype=schm_rec->dtype}});
				free(buffer);
				buffer = nullptr;
			} else {
				found = false;
				for (j = 0; j < q->join.n; j++) {
					key.key = (void *) var_name.c_str();
					cat_rec = q->join.tables[j].c_rec;
					schm_rec = (schema_rec *)BPlusTree_Query_Key(cat_rec->schema_table, &key);
					if (!schm_rec) continue;
					if (found) {
						printf("col name %s ambiguous\n", var_name.c_str());
						return false;
					}
					found = true;
					q->table_map->insert({var_name, (data_src) {.tableIdx=j, .offset=schm_rec->offset, .dtype=schm_rec->dtype}});
				}
				if (!found) {
					printf("col name %s not exist in any tables\n", var_name.c_str());
					return false;
				}
			}
		}
	}
	return true;
}

bool init_qep(qep *q) {
  catalog_rec *c_rec;
  BPluskey_t key;
  int i;
  get_val_fn get_val;

  get_val = make_get_val(q);

  if (q->where.tree) {
	q->where.tree->tree->setGetVal(get_val);
  }

  key.key_size = TABLE_NAME_SIZE;
  for (i = 0; i < q->join.n; i++) {
    key.key = q->join.tables[i].name;
    c_rec = (catalog_rec *)BPlusTree_Query_Key(&catalog, &key);
    if (!c_rec) {
      printf("Not found: table %s\n", q->join.tables[i].name);
      return false;
    }
    q->join.tables[i].c_rec = c_rec;
  }

  if (!init_select(q, get_val)) {
	printf("fail to init select\n");
	return false;
  }

  q->joined_rows.n = q->join.n;
  q->iterators.n = q->join.n;
  q->curr_row = -1;
  return true;
}

void do_select(qep *q) {
	int i;
	int rows;

	rows = 0;
	print_header(q);
	while (get_next_row(q)) {
		q->curr_row++;
		if (!check_predicate(q)) continue;
		for (i = 0; i < q->select.n; i++) {
			if (q->select.cols[i].computed_row != q->curr_row) {
				free(q->select.cols[i].computed_val);
				q->select.cols[i].computed_val =
					resolve_sql_exp_tree(q->select.cols[i].tree);
				if (!q->select.cols[i].computed_val) {
					printf("Error: cannot resolve col %d\n", i);
					return;
				}
				q->select.cols[i].computed_row = q->curr_row;
			}
		}
		print_row(q);
		rows++;
	}
	print_line(q->select.n);
	printf("(%d rows)\n", rows);
}

bool get_next_row(qep *q) {
  if (q->is_end)
    return false;
  if (!q->is_begin) {
    q->is_begin = true;
    if (!iterate_begin(q)) {
      q->is_end = true;
      return false;
    }
    return true;
  }
  if (!iterate_next(q, q->iterators.n - 1)) {
    q->is_end = true;
    return false;
  }
  return true;
}

bool iterate_begin(qep *q) {
  BPluskey_t *key;
  void *rec;

  for (int i = 0; i < q->iterators.n; i++) {
    assert(!q->iterators.table[i].node);
    assert(!q->iterators.table[i].offset);
    rec = BPlusTree_get_next_record(q->join.tables[i].c_rec->record_table,
                                    &q->iterators.table[i].node,
                                    &q->iterators.table[i].offset, &key);
    if (!rec)
      return false;
    q->joined_rows.table[i].key = key;
    q->joined_rows.table[i].rec = rec;
  }
  return true;
}

bool iterate_next(qep *q, int i) {
  BPluskey_t *key;
  void *rec;

  rec = BPlusTree_get_next_record(q->join.tables[i].c_rec->record_table,
                                  &q->iterators.table[i].node,
                                  &q->iterators.table[i].offset, &key);
  if (rec) {
    q->joined_rows.table[i].key = key;
    q->joined_rows.table[i].rec = rec;
    return true;
  }
  if (i == 0)
    return false;
  if (!iterate_next(q, i - 1)) {
    return false;
  }
  q->iterators.table[i].node = NULL;
  q->iterators.table[i].offset = 0;
  rec = BPlusTree_get_next_record(q->join.tables[i].c_rec->record_table,
                                  &q->iterators.table[i].node,
                                  &q->iterators.table[i].offset, &key);
  assert(rec);
  q->joined_rows.table[i].key = key;
  q->joined_rows.table[i].rec = rec;
  return true;
}

static bool check_predicate(qep *q) {
	DTypeBool *res;

	if (!q->where.tree) return true;
	free(q->where.computed_val);
	q->where.computed_val = q->where.tree->tree->eval();
	res = dynamic_cast<DTypeBool *>(q->where.computed_val);
	assert(res);
	return res->val;
}

void print_line(int cols) {
	printf("+");
	for (int i = 0; i < cols; i++) {
		for (int j = 0; j < COLUMN_WIDTH; j++) {
			printf("-");
		}
		printf("+");
  	}
	printf("\n");
}

void print_header(qep *q) {
	print_line(q->select.n);
	printf("|");
	for (int i = 0; i < q->select.n; i++) {
		printf("%-*s|", COLUMN_WIDTH, q->select.cols[i].display_name);
	}
	printf("\n");
	print_line(q->select.n);
}

void print_row(qep *q) {
	DType *val;

	printf("|");
	for (int i = 0; i < q->select.n; i++) {
		val = q->select.cols[i].computed_val;
		switch (val->id) {
			case MATH_INTEGER_VALUE:
				printf("%-*d|", COLUMN_WIDTH, dynamic_cast<DTypeInt *>(val)->val);
				break;
			case MATH_DOUBLE_VALUE:
				printf("%-*f|", COLUMN_WIDTH, dynamic_cast<DTypeDouble *>(val)->val);
				break;
			case MATH_STRING_VALUE:
				printf("%-*s|", COLUMN_WIDTH, dynamic_cast<DTypeStr *>(val)->val.c_str());
				break;
			default:
				assert(0);
		}
	}
	printf("\n");
}

qep *_q;

int get_table_index(const char *c) { return 0; }

const char *get_col_name(const char *c) { return c; }

DType *_get_val(const char *c) {
	void *rec;
	void *val;
	data_src *dsrc;

	if (auto search = _q->col_alias->find(c); search != _q->col_alias->end()) {
		int colIdx = search->second;
		qp_col *col;

		col = &_q->select.cols[colIdx];
		if (col->computed_row != _q->curr_row) {
			col->computed_row = _q->curr_row;
			delete col->computed_val;
			col->computed_val = col->tree->tree->eval();
		}
		return col->computed_val->clone();
	}
	auto search = _q->table_map->find(c);
	assert(search != _q->table_map->end());
	dsrc = &search->second;
	rec = _q->joined_rows.table[dsrc->tableIdx].rec;
	val = (char *)rec + dsrc->offset;
	switch (dsrc->dtype) {
		case SQL_STRING:
			return new DTypeStr((char*) val);
		case SQL_INT:
			return new DTypeInt(*(int *)val);
		case SQL_DOUBLE:
			return new DTypeDouble(*(double *)val);
		case SQL_BOOL:
			return new DTypeBool(*(bool *)val);
		default:
			return nullptr;
	}
}

DType *(*make_get_val(qep *q))(const char *) {
  _q = q;
  return &_get_val;
}
