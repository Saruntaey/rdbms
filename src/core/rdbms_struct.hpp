#ifndef _RDBMS_STRUCT_
#define _RDBMS_STRUCT_

#include "sql_math_exp_interface.hpp"

typedef struct qp_col {
	sql_exp_tree *tree;
	DType *computed_val;
} qp_col;

#endif
