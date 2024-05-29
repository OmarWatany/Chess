#ifndef ALIST_H
#define ALIST_H

#ifdef __cplusplus
extern "C" {
#endif // cpp

#include <stdbool.h>
#include <stdlib.h>

typedef void *gdata_t;

typedef struct alist_t alist_t;
typedef struct anode_t anode_t;

anode_t *create_anode();
gdata_t anode_data(anode_t *node);
int16_t anode_set_data(anode_t *node, gdata_t data);
void destroy_anode(anode_t **node);

alist_t *create_alist(size_t item_size);
int16_t alist_push(alist_t *list, gdata_t data);
int16_t alist_pop(alist_t *list);
int16_t alist_set_at(alist_t *list, size_t pos, gdata_t data);
int16_t alist_rm_at(alist_t *list, size_t pos);
gdata_t alist_at(alist_t *list, size_t pos);
void alist_reserve(alist_t *list, size_t size);
void alist_set_allocator(alist_t *alist, gdata_t (*allocator_fun)(gdata_t data));
bool alist_is_empty(alist_t *list);
size_t alist_size(alist_t *list);
size_t alist_capacity(alist_t *list);

void clear_alist(alist_t *list);
void destroy_alist(alist_t **list);

#ifdef __cplusplus
}
#endif // cpp
#endif // !ALIST_H
