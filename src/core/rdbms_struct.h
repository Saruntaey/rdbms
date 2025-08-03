#ifndef _RDBMS_STRUCT_
#define _RDBMS_STRUCT_

#include "sql_math_exp_interface.h"

class DType;

typedef struct qp_col {
	sql_exp_tree *tree;
	DType *computed_val;
} qp_col;

#endif
