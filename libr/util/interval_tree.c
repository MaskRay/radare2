/* radare - LGPL - Copyright 2017 - MaskRay */

#include <stdio.h>
#include "r_util/r_interval_tree.h"

#define LEFT(x) ((RIntervalNode *)(x)->rb.child[0])
#define RIGHT(x) ((RIntervalNode *)(x)->rb.child[1])

static int _cmp(const void *a_, const RBNode *b_) {
	ut64 from0 = ((const RIntervalNode *)a_)->from, from1 = ((const RIntervalNode *)b_)->from;
	ut64 to0 = ((const RIntervalNode *)a_)->from, to1 = ((const RIntervalNode *)b_)->from;
	return from0 < from1 ? -1 : from0 > from1 ? 1 : to0 < to1 ? -1 : to0 > to1 ? 1 : 0;
}

static void _calc_maxm1(RBNode *x_) {
	RIntervalNode *x = (RIntervalNode *)x_, *y;
	x->maxm1 = x->to - 1;
	for (int i = 0; i < 2; i++)
		if ((y = (RIntervalNode *)x_->child[i]) && y->maxm1 > x->maxm1) {
			x->maxm1 = y->maxm1;
		}
}

static RIntervalNode *_probe(RIntervalIter *it, RIntervalNode *x, ut64 from, ut64 to) {
	RIntervalNode *y;
	for (;;) {
		if ((y = LEFT (x)) && from <= y->maxm1) {
			it->path[it->len++] = x;
			x = y;
			continue;
		}
		if (x->from <= to - 1) {
			if (from <= x->to - 1) {
				return x;
			}
			if ((y = RIGHT (x))) {
				x = y;
				if (from <= x->maxm1) {
					continue;
				}
			}
		}
		return NULL;
	}
}

R_API bool r_interval_tree_delete(RIntervalNode **root, void *data) {
	return r_rbtree_aug_delete ((RBNode **)root, data, _cmp, (RBNodeFree)free, _calc_maxm1);
}

R_API RIntervalIter r_interval_tree_first(RIntervalNode *x, ut64 from, ut64 to) {
	RIntervalIter it;
	it.len = 0;
	if (x && from <= x->maxm1) {
		it.cur = _probe (&it, x, from, to);
	} else {
		it.cur = NULL;
	}
	return it;
}

R_API void r_interval_tree_insert(RIntervalNode **root, RIntervalNode *data) {
	r_rbtree_aug_insert ((RBNode **)root, data, (RBNode *)data, _cmp, _calc_maxm1);
}

static RIntervalNode *_next(RIntervalIter *it, ut64 from, ut64 to) {
	RIntervalNode *x = it->cur, *y;
	for (;;) {
		if ((y = RIGHT (x)) && from <= y->maxm1) {
			return _probe (it, y, from, to);
		}
		if (!it->len) {
			return NULL;
		}
		x = it->path[--it->len];
		if (to - 1 < x->from) {
			return NULL;
		}
		if (from <= x->to - 1) {
			return x;
		}
	}
}

R_API void r_interval_tree_next(RIntervalIter *it, ut64 from, ut64 to) {
	it->cur = _next (it, from, to);
}

void print(RIntervalNode *x, int d) {
	RBNode *x_ = (RBNode *)x;
	if (x) {
		print((RIntervalNode *)x_->child[0], d+1);
		printf("%*s(%llu, %llu) %llu\n", 2*d, "", x->from, x->to, x->maxm1);
		print((RIntervalNode *)x_->child[1], d+1);
	}
}

struct I {
	ut64 from, to;
} is[] = {
	{2, 8},
	{3, 5},
	{3, 7},
	{4, 6},
	{7, 9},
	{9, 11},
	{11, 14},
};

int main(void) {
	RIntervalNode *root = NULL, *x;
	for (int i = 0; i < sizeof(is)/sizeof(*is); i++) {
		x = R_NEW (RIntervalNode);
		x->from = is[i].from;
		x->to = is[i].to;
		r_interval_tree_insert (&root, x);
		printf("=== %d\n", i);
		print(root, 0);
	}

	ut64 from = 4, to = 8;
	RIntervalIter it = r_interval_tree_first (root, from, to);
	for (; it.cur; it.cur = _next (&it, from, to)) {
		printf("%llu %llu\n", it.cur->from, it.cur->to);
	}
}
