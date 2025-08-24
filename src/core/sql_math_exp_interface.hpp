#ifndef _SQL_MATH_EXP_INTERFACE_
#define _SQL_MATH_EXP_INTERFACE_

#include "math_expr_tree.h"

typedef struct sql_exp_tree {
	MathExprTree *tree;
} sql_exp_tree;

typedef struct sql_predicate_exp_tree {
	MathExprTree *tree;
} sql_predicate_exp_tree;

sql_exp_tree *create_sql_exp_tree();
sql_predicate_exp_tree *create_sql_predicate_exp_tree();
void sql_exp_tree_destroy(sql_exp_tree *t);
void sql_predicate_exp_tree_destroy(sql_predicate_exp_tree *t);
DType *resolve_sql_exp_tree(sql_exp_tree *t);

#endif
