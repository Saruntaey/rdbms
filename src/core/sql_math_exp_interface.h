#ifndef _SQL_MATH_EXP_INTERFACE_
#define _SQL_MATH_EXP_INTERFACE_

#include "math_expr_tree.h"

typedef struct sql_exp_tree {
	MathExprTree *tree;
} sql_exp_tree;

sql_exp_tree *create_sql_exp_tree();
void sql_exp_tree_destroy(sql_exp_tree *t);

#endif
