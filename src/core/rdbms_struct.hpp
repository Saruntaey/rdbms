#ifndef _RDBMS_STRUCT_
#define _RDBMS_STRUCT_

#include "sql_enum.h"
#include "sql_math_exp_interface.hpp"

typedef struct qp_col {
	sql_exp_tree *tree;
	DType *computed_val;
	int computed_row;
	char *display_name;
} qp_col;

typedef struct data_src {
	int tableIdx;
	int offset;
	sql_dtype_t dtype;
} data_src;

#endif
