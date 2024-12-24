#ifndef GTYPES_H
#define GTYPES_H

#ifdef __cplusplus
extern "C" {
#endif // cpp

#include "gallocator.h"
#include "gnodes_types.h"
#include <stdbool.h>
#include <stdlib.h>

typedef void (*for_each_fn)(gdata_t);
typedef int (*cmp_fun)(gdata_t data1, gdata_t data2); // data1 > data2 = 1

typedef enum {
    PRE_ORDER,
    IN_ORDER,
    POST_ORDER,
    BREADTH_FIRST_ORDER,
} TRAVERSE_ORDER;

typedef enum {
    MIN_HEAP,
    MAX_HEAP,
} HEAP_TYPE;

typedef struct {
    gdata_t begin, from, end;
} itr_ctx_t;

typedef struct gitr_t gitr_t;
typedef struct {
    gdata_t (*next)(gitr_t *);
    gdata_t (*prev)(gitr_t *);
    gdata_t (*begin)(gitr_t *);
} gitr_vtable;

struct gitr_t {
    itr_ctx_t   *context;
    gitr_vtable *vtable;
};

typedef struct {
    tnode_t *root;
    size_t   item_size, k, size;
    // function pointers
    cmp_fun         cmp_fun;
    allocator_fun_t allocator_fun;
} ktree_t;

typedef struct {
    ktree_t       *tree;
    TRAVERSE_ORDER order;
} ktree_itr_t;

typedef struct {
    anode_t *buf;
    size_t   item_size, capacity, size;
    // function pointers
    allocator_fun_t allocator_fun;
    Gallocator_t   *gallocator;
} alist_t;

typedef struct {
    anode_t *buf;
    size_t   read_pointer, write_pointer, capacity, size, item_size;
    // function pointers
    allocator_fun_t allocator_fun;
} circular_array_t;

typedef struct {
    uint8_t *buffer;
    size_t   size, write_idx, read_idx;
} ringbuffer;

typedef struct {
    alist_t buf;
} astack_t;

typedef struct {
    lnode_t *head, *tail;
    size_t   item_size;
    // function pointers
    allocator_fun_t allocator_fun;
} list_t;

typedef struct {
    list_t list;
    size_t length;
} queue_t;

typedef struct {
    alist_t   buf;
    size_t    k;
    HEAP_TYPE type;
    // function pointers
    cmp_fun cmp_fun;
} heap_t;

typedef ktree_t btree_t;

typedef struct {
    heap_t h;
    size_t item_size;
} pqueue_t;

typedef struct {
    list_t list;
    size_t length;
} deque_t;

typedef struct {
    list_t list;
} stack_t;

#ifdef __cplusplus
}
#endif // cpp
#endif // GTYPES_H
