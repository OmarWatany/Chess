#ifndef GTREE_T
#define GTREE_T

#include "gnodes_types.h"
#ifdef __cplusplus
extern "C" {
#endif // cpp

#include "gds_types.h"
#include "gnode.h"
#include <stdbool.h>

typedef struct {
    gdata_t node;
    size_t  lvl;
} tree_for_data;

ktree_t *kt_create(size_t item_size, size_t k);

void kt_init(ktree_t *tree, size_t item_size, size_t k);
void kt_set_allocator(ktree_t *tree, allocator_fun_t allocator_fun);
void kt_add_safe(ktree_t *tree, gdata_t data); // TODO: implement
void kt_add(ktree_t *tree, gdata_t data);
void kt_destroy(ktree_t *tree);
void kt_for_each(ktree_t *tree, TRAVERSE_ORDER order, for_each_fn function);

tnode_t **kt_grand_childrens(ktree_t *tree, size_t lvl);

heap_t *heap_create(size_t item_size, size_t k, HEAP_TYPE type);

void heap_init(heap_t *heap, size_t item_size, size_t k, HEAP_TYPE type);
void heap_set_allocator(heap_t *heap, allocator_fun_t allocator_fun);
void heap_set_cmp_fun(heap_t *heap, cmp_fun cmp);
void heap_add_safe(heap_t *heap, size_t item_size, gdata_t data);
void heap_add(heap_t *heap, gdata_t data);
void heap_pop(heap_t *heap);
bool valid_heap(heap_t *heap, size_t pos);
void heap_for_each_order(heap_t *heap, TRAVERSE_ORDER order, for_each_fn function);
void heap_for_each(heap_t *heap, for_each_fn function);
void heap_destroy(heap_t *heap);

gdata_t heap_peak(heap_t *heap);

btree_t *bt_create(size_t item_size);

void bt_init(btree_t *tree, size_t item_size, cmp_fun cmp);
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
