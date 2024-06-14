#ifndef LINKED_LIST_H
#define LINKED_LIST_H

#ifdef __cplusplus
extern "C" {
#endif // cpp

#include "gds_types.h"
#include <stdlib.h>

llist_t    *create_list(size_t item_size);
list_itr_t *create_list_iterator(llist_t *list);

void init_list(llist_t *list, size_t item_size);
void init_list_iterator(list_itr_t *it, llist_t *list);

size_t   list_item_size(llist_t *list);
lnode_t *list_head(llist_t *list);
lnode_t *list_tail(llist_t *list);
lnode_t *next(list_itr_t *iterator);
lnode_t *prev(list_itr_t *iterator);
lnode_t *itr_begin(list_itr_t *iterator);
lnode_t *itr_end(list_itr_t *iterator);

int16_t push_front(llist_t *list, gdata_t data);
int16_t push_back(llist_t *list, gdata_t data);
int16_t pop_front(llist_t *list);
int16_t pop_back(llist_t *list);
gdata_t peak_front(llist_t *list);
gdata_t peak_back(llist_t *list);

void list_destroy(llist_t *list);
void llist_set_allocator(llist_t *list, gdata_t (*allocator_fun)(gdata_t data));
void itr_set_from(list_itr_t *iterator, lnode_t *from);
void itr_set_begin(list_itr_t *iterator, lnode_t *begin_node);
void itr_set_end(list_itr_t *iterator, lnode_t *end_node);

llist_t *itr_list(list_itr_t *iterator);
size_t   itr_t_size();

int16_t dump_list(llist_t *list, void (*print_data)(gdata_t));
int16_t reverse_dump_list(llist_t *list, void (*print_data)(gdata_t));

#ifdef __cplusplus
}
#endif // cpp
#endif // LINKED_LIST_H
