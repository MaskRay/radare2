/* radare - LGPL - Copyright 2017 - MaskRay */

#ifndef R2_INTERVAL_TREE_H
#define R2_INTERVAL_TREE_H

#include <stddef.h>
#include "r_util/r_rbtree.h"

// Can be zero initialized
typedef struct r_interval_node_t {
	RBNode rb;
	ut64 from, to, maxm1;
} RIntervalNode;

typedef struct r_interval_iter_t {
	int len;
	RIntervalNode *cur, *path[R_RBTREE_MAX_HEIGHT];
} RIntervalIter;

//R_API RIntervalIter r_interval_tree_first(RIntervalNode *root, ut64 from, ut64 to);
//R_API void r_interval_tree_insert(RIntervalNode **root, void *data, off_t node_offset, RIntervalComparator cmp);
//R_API void r_interval_tree_next(RIntervalIter *it, ut64 from, ut64 to);

#endif  // R2_INTERVAL_TREE_H
