#include "BPlusTree.h"
#include "catalog.h"
#include "dtype.h"
#include "qep.hpp"
#include "sql_const.h"
#include <cassert>
#include <cstdio>

#define COLUMN_WIDTH 20

extern BPlusTree_t catalog;

static bool init_qep(qep *q);
static void do_select(qep *q);
static bool get_next_row(qep *q);
static bool iterate_begin(qep *q);
static bool iterate_next(qep *q, int n);
static void print_row(qep *q);
static DType *get_val(const char *c);
static DType *(*make_get_val(qep *q))(const char *);

void process_select_query(qep *q) {
  if (!init_qep(q)) {
    printf("Fail to init query execution plan\n");
    return;
  }
  do_select(q);
}

bool init_qep(qep *q) {
  catalog_rec *c_rec;
  BPluskey_t key;
  int i;
  DType *(*get_val)(const char *c);

  // todo: remove when implement prefix-col name and alias
  if (q->join.n > 1) {
    printf("Error: join is not support yet\n");
    return false;
  }

  get_val = make_get_val(q);
  for (i = 0; i < q->select.n; i++) {
    q->select.cols[i].tree->tree->setGetVal(get_val);
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
  q->joined_rows.n = q->join.n;
  q->iterators.n = q->join.n;
  return true;
}

void do_select(qep *q) {
	int i;

	while (get_next_row(q)) {
		for (i = 0; i < q->select.n; i++) {
			free(q->select.cols[i].computed_val);
			q->select.cols[i].computed_val =
				resolve_sql_exp_tree(q->select.cols[i].tree);
			if (!q->select.cols[i].computed_val) {
				printf("Error: cannot resolve col %d\n", i);
				return;
			}
		}
		print_row(q);
	}
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

void print_row(qep *q) {
  DType *val;

  for (int i = 0; i < q->select.n; i++) {
	val = q->select.cols[i].computed_val;
	switch (val->id) {
			case MATH_INTEGER_VALUE:
				printf("%-*d|", COLUMN_WIDTH, dynamic_cast<DTypeInt *>(val)->val);
				break;
			case MATH_DOUBLE_VALUE:
				printf("%-*f|", COLUMN_WIDTH, dynamic_cast<DTypeDouble *>(val)->val);
				break;
	}
  }
  printf("\n");
}

qep *_q;

int get_table_index(const char *c) { return 0; }

const char *get_col_name(const char *c) { return c; }

DType *_get_val(const char *c) {
  int tableIdx;
  catalog_rec *cat_rec;
  schema_rec *schm_rec;
  void *rec;
  BPluskey_t key;
  void *val;

  tableIdx = get_table_index(c);
  key.key = (void *)get_col_name(c);
  key.key_size = COL_NAME_SIZE;
  rec = _q->joined_rows.table[tableIdx].rec;
  cat_rec = _q->join.tables[tableIdx].c_rec;
  schm_rec = (schema_rec *)BPlusTree_Query_Key(cat_rec->schema_table, &key);
  if (!schm_rec) {
    printf("Error: no col %s in table %s\n", (char *)key.key,
           cat_rec->table_name);
    return nullptr;
  }
  val = (void *)((char *)rec + schm_rec->offset);

  switch (schm_rec->dtype) {
  case SQL_STRING:
    printf("math expr not support string yet\n");
    assert(1);
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
