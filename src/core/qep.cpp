#include "qep.h"

void qp_col_destroy(qp_col *p) {
	if (!p) return;
	delete p->tree;
	delete p->computed_val;
	p->tree = nullptr;
	p->computed_val = nullptr;
}

void qep_destroy(qep *p) {
	int i;

	if (!p) return;
	for(i = 0; i < p->select.n; i++) {
		qp_col_destroy(&p->select.cols[i]);
	}
}
