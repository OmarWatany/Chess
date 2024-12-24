#ifndef QUEUE_H
#define QUEUE_H

#include "gds_types.h"

#ifdef __cplusplus
extern "C" {
#endif // cpp

queue_t *queue_create(size_t item_size);
void     queue_init(queue_t *queue, size_t item_size);

int16_t enqueue(queue_t *queue, gdata_t data);
int16_t enqueue_safe(queue_t *queue, size_t item_size, gdata_t data);
int16_t dequeue(queue_t *queue);
size_t  queue_length(queue_t *queue);

gdata_t queue_front(queue_t *queue);
gdata_t queue_back(queue_t *queue);

bool queue_empty(queue_t *queue);
bool queue_find(queue_t *heystack, gdata_t needle, bool (*search_fun)(gdata_t d1, gdata_t needle));

void queue_dump(queue_t *queue, void (*_print_data)(gdata_t data));
void queue_destroy(queue_t *queue);

#ifdef __cplusplus
}
#endif // cpp
#endif // !QUEUE_H
