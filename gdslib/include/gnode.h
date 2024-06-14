#ifndef GNODE_H
#define GNODE_H

#ifdef __cplusplus
extern "C" {
#endif // cpp

#include "gnodes_types.h"
#include <stdbool.h>

anode_t *anode_create();
int16_t  anode_init(anode_t *node);
gdata_t  anode_data(anode_t *node);
int16_t  anode_set_data(anode_t *node, gdata_t data);
void     anode_destroy(anode_t *node);

lnode_t *lnode_create();
int16_t  lnode_init(lnode_t *node);
gdata_t  lnode_data(lnode_t *node);
int16_t  lnode_set_data(lnode_t *node, gdata_t data);
size_t   lnode_link(lnode_t *node);
void     lnode_set_link(lnode_t *node, uintptr_t new_link);
void     lnode_destroy(lnode_t *node);

tnode_t *tnode_create(size_t links_count);
int16_t  tnode_init(tnode_t *node, size_t links_count);
gdata_t  tnode_data(tnode_t *node);
int16_t  tnode_set_data(tnode_t *node, gdata_t data);
tnode_t *tnode_child(tnode_t *node, size_t n);
tnode_t *tnode_link(tnode_t *node, size_t link_num);
void     tnode_set_link(tnode_t *node, size_t link_num, tnode_t *new_link);
void     tnode_destroy(tnode_t *node);

tnode_t **tnode_grand_children(tnode_t *node, int nk, size_t lvl);

#ifdef __cplusplus
}
#endif // cpp
#endif // GNODE_H
