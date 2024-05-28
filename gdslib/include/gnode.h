#ifndef NODE_T_H
#define NODE_T_H

#ifdef __cplusplus
extern "C" {
#endif // cpp

#include <stdint.h>
#include <stdlib.h>

typedef void *gdata_t;

typedef struct node_t node_t;

node_t *create_node();

size_t node_link(node_t *node);
void   node_set_link(node_t *node, size_t new_link);

gdata_t node_data(node_t *node);
int16_t node_set_data(node_t *node, size_t item_size, gdata_t data);

void destroy_node(node_t **node);

#ifdef __cplusplus
}
#endif // cpp
#endif // NODE_T_H
