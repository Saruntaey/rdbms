#ifndef _QEP_
#define _QEP_

#include "BPlusTree.h"
#include "rdbms_struct.hpp"
#include "sql_const.h"
#include "catalog.h"

typedef struct qep {
	struct {
		int n;
		struct {
			catalog_rec *c_rec;
			char name[TABLE_NAME_SIZE];
		} tables[MAX_TABLE_IN_JOIN_LIST];
	} join;

	struct {
		int n;
		qp_col cols[MAX_COL_IN_SELECT_LIST];
	} select;

	struct {
		sql_predicate_exp_tree *tree;
		DType *computed_val;
	} where;

	struct {
		int n;
		struct {
			BPluskey_t *key;
			void *rec;
		} table[MAX_TABLE_IN_JOIN_LIST];
	} joined_rows;

	struct {
		int n;
		struct {
			BPlusTreeNode *node;
			int offset;
		} table[MAX_TABLE_IN_JOIN_LIST];
	} iterators;

	bool is_begin;
	bool is_end;
} qep;

void qep_destroy(qep *p);
void qp_col_destroy(qp_col *p);

#endif
