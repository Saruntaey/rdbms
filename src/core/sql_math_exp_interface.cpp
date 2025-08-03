#include <stdlib.h>
#include "sql_math_exp_interface.h"
#include "parser_export.h"

extern parse_status E();
extern lex_data **infix_to_postfix(lex_data *infix, int size_in, int *size_out);
static MathExprTree *create_math_exp_tree();

sql_exp_tree *create_sql_exp_tree() {
	sql_exp_tree *t;
	MathExprTree *mt;

	mt = create_math_exp_tree();
	if (!mt) return nullptr;
	t = (sql_exp_tree *) calloc(1, sizeof(*t));
	t->tree = mt;
	return t;
}

void sql_exp_tree_destroy(sql_exp_tree *t) {
	if (!t) return;
	delete t->tree;
	t->tree = nullptr;
}

MathExprTree *create_math_exp_tree() {
	MathExprTree *tree;
	int exp_start;
	parse_status s;
	int postfix_size;
	lex_data **postfix;

	exp_start = lex_stack.top + 1;
	s = E();
	if (s == PARSE_ERROR) return nullptr;
	postfix = infix_to_postfix(&lex_stack.data[exp_start], lex_stack.top - exp_start + 1, &postfix_size);
	tree = new MathExprTree(postfix, postfix_size);
	if (!tree->valid()) {
		delete tree;
		tree = nullptr;
	}
	free(postfix);
	return tree;
}
