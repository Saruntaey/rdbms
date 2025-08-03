#ifndef _QEP_
#define _QEP_

#include "rdbms_struct.h"
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
} qep;

void qep_destroy(qep *p);
void qp_col_destroy(qp_col *p);

#endif
