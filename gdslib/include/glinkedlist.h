#ifndef LINKED_LIST_H
#define LINKED_LIST_H

#ifdef __cplusplus
extern "C" {
#endif // cpp

#include "gds_types.h"
#include <stdlib.h>

// it's recommended to set item_size to 0 if you are going to save strings
void    list_init(list_t *list, size_t item_size);
list_t *list_create(size_t item_size);

int16_t push_front_safe(list_t *list, size_t item_size, gdata_t data);
int16_t push_back_safe(list_t *list, size_t item_size, gdata_t data);
int16_t push_front(list_t *list, gdata_t data);
int16_t push_back(list_t *list, gdata_t data);
gdata_t peak_front(list_t *list);
gdata_t peak_back(list_t *list);

// it copies data into buffer if exist
int16_t pop_front(list_t *list, void *buffer);
int16_t pop_back(list_t *list, void *buffer);

// needs to be freed
char *strpop_front(list_t *list);
char *strpop_back(list_t *list);

void list_set_allocator(list_t *list, gdata_t (*allocator_fun)(gdata_t data));

void list_purge(list_t *list);
void list_destroy(list_t *list);

int16_t dump_list(list_t *list, void (*print_data)(gdata_t));
int16_t reverse_dump_list(list_t *list, void (*print_data)(gdata_t));

#ifdef __cplusplus
}
#endif // cpp
#endif // LINKED_LIST_H
