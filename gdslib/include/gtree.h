#ifndef GTREE_T
#define GTREE_T

#ifdef __cplusplus
extern "C" {
#endif // cpp

#include "gallocator.h"
#include "gnode.h"
#include "gnodes_types.h"
#include <stdbool.h>

typedef struct ktree_t     ktree_t;
typedef ktree_t            btree_t;
typedef struct ktree_itr_t ktree_itr_t;
typedef void (*for_each_fn)(tnode_t *node, size_t level);

typedef enum {
    PRE_ORDER,
    IN_ORDER,
    POST_ORDER,
    BREADTH_FIRST_ORDER,
} TRAVERSE_ORDER;

struct ktree_t {
    tnode_t *root;
    size_t   item_size, k;
    size_t   size;
    int (*cmp_fun)(gdata_t data1, gdata_t data2); // data1 > data2 = 1
    allocator_fun_t allocator_fun;
};

struct ktree_itr_t {
    ktree_t       *tree;
    TRAVERSE_ORDER order;
};

ktree_t *kt_create(size_t item_size, size_t k);

void kt_init(ktree_t *tree, size_t item_size, size_t k);
void kt_set_allocator(ktree_t *tree, allocator_fun_t allocator_fun);
void kt_add(ktree_t *tree, gdata_t data);
void kt_destroy(ktree_t *tree);
void kt_for_each(ktree_t *tree, TRAVERSE_ORDER order, for_each_fn function);

tnode_t **kt_grand_childrens(ktree_t *tree, size_t lvl);

btree_t *btr_create(size_t item_size);
void     btr_init(ktree_t *tree, size_t item_size, int (*cmp_fun)(gdata_t data1, gdata_t data2));

void bst_add(btree_t *tree, gdata_t data);
bool bst_find(btree_t *heystack, gdata_t needle);
void bst_delete(btree_t *tree, gdata_t data);

gdata_t *bst_min(ktree_t *tree);
gdata_t *bst_max(ktree_t *tree);

void bst_destroy(btree_t *tree);

#ifdef __cplusplus
}
#endif // cpp
#endif // GTREE_T
