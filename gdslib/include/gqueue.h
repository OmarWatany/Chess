#include "glinkedlist.h"
#include <stdbool.h>
#include <stdlib.h>

#ifndef QUEUE_H
#define QUEUE_H

#ifdef __cplusplus
extern "C" {
#endif // cpp

typedef struct queue_t queue_t;

queue_t *create_queue(size_t item_size);

int16_t enqueue(queue_t *queue, gdata_t data);
int16_t dequeue(queue_t *queue);
size_t  queue_length(queue_t *queue);

gdata_t queue_front(queue_t *queue);
gdata_t queue_back(queue_t *queue);
bool    queue_is_empty(queue_t *queue);
bool    in_queue(queue_t *heystack, gdata_t needle, bool (*search_fun)(node_t *node, gdata_t data));

void dump_queue(queue_t *queue, void (*_print_data)(gdata_t data));
void destroy_queue(queue_t **queue);

#ifdef __cplusplus
}
#endif // cpp
#endif // !QUEUE_H
