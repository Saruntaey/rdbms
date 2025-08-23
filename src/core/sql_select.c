#include "catalog.h"
#include "BPlusTree.h"
#include "qep.hpp" 
#include <cassert>
#include <cstdio>

extern BPlusTree_t catalog;

static bool init_qep(qep *q);
static void do_select(qep *q);
static bool get_next_row(qep *q);
static bool iterate_begin(qep *q);
static bool iterate_next(qep *q, int n);
static void print_row_debug(qep *q);

void process_select_query(qep *q) {
	if (!init_qep(q)) {
		printf("Fail to init query execution plan\n");
		return;
	}
	do_select(q);
}

bool init_qep(qep * q) {
	catalog_rec *c_rec;
	BPluskey_t key;

	key.key_size = TABLE_NAME_SIZE;
	for (int i = 0; i < q->join.n; i++) {
		key.key = q->join.tables[i].name;
		c_rec = (catalog_rec *) BPlusTree_Query_Key(&catalog, &key);
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
	while (get_next_row(q)) {
		print_row_debug(q);
	}
}

bool get_next_row(qep *q) {
	if (q->is_end) return false;
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
		rec = BPlusTree_get_next_record(
			q->join.tables[i].c_rec->record_table,
			&q->iterators.table[i].node,
			&q->iterators.table[i].offset,
			&key
		);
		if (!rec) return false;
		q->joined_rows.table[i].key = key;
		q->joined_rows.table[i].rec = rec;
	}
	return true;
}

bool iterate_next(qep *q, int i) {
	BPluskey_t *key;
	void *rec;

	rec = BPlusTree_get_next_record(
		q->join.tables[i].c_rec->record_table,
		&q->iterators.table[i].node,
		&q->iterators.table[i].offset,
		&key
	);
	if (rec) {
		q->joined_rows.table[i].key = key;
		q->joined_rows.table[i].rec = rec;
		return true;
	}
	if (i == 0) return false;
	if (!iterate_next(q, i-1)) {
		return false;
	}
	q->iterators.table[i].node = NULL;
	q->iterators.table[i].offset = 0;
	rec = BPlusTree_get_next_record(
		q->join.tables[i].c_rec->record_table,
		&q->iterators.table[i].node,
		&q->iterators.table[i].offset,
		&key
	);
	assert(rec);
	q->joined_rows.table[i].key = key;
	q->joined_rows.table[i].rec = rec;
	return true;
}

void print_row_debug(qep *q) {
	for (int i = 0; i < q->joined_rows.n; i++) {
		printf("%s ", (char *) q->joined_rows.table[i].rec);
	}
	printf("\n");
}
