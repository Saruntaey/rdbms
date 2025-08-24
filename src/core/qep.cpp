#include "qep.hpp"
#include "sql_math_exp_interface.hpp"

void qp_col_destroy(qp_col *p) {
	if (!p) return;
	sql_exp_tree_destroy(p->tree);
	free(p->tree);
	delete p->computed_val;
	free(p->display_name);
	p->tree = nullptr;
	p->computed_val = nullptr;
	p->display_name = nullptr;
}

void qep_destroy(qep *p) {
	int i;

	if (!p) return;
	for (i = 0; i < p->select.n; i++) {
		qp_col_destroy(&p->select.cols[i]);
	}

	sql_predicate_exp_tree_destroy(p->where.tree);
	free(p->where.tree);
	p->where.tree = nullptr;
	delete p->where.computed_val;
	p->where.computed_val = nullptr;

	p->col_alias->clear();
	free(p->col_alias);
	p->col_alias = nullptr;

	p->table_map->clear();
	free(p->table_map);
	p->table_map = nullptr;
}
