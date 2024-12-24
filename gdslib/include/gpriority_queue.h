#ifndef PQUEUE_H
#define PQUEUE_H

#include "gallocator.h"
#include <sys/types.h>
#ifdef __cplusplus
extern "C" {
#endif // cpp

#include "gds_types.h"
#include "gnode.h"

typedef struct {
    long    priority;
    gdata_t data;
} pq_node;

pqueue_t *pq_create(size_t item_size, HEAP_TYPE type);

void    pq_init(pqueue_t *pqueue, size_t item_size, HEAP_TYPE type);
void    pq_set_allocator(pqueue_t *pqueue, allocator_fun_t allocator);
gdata_t pq_peak(pqueue_t *pqueue);
int16_t pq_enqueue_safe(pqueue_t *pqueue, size_t item_size, long int priority, gdata_t data);
int16_t pq_enqueue(pqueue_t *pqueue, long int priority, gdata_t data);
int16_t pq_dequeue(pqueue_t *pqueue);
void    pq_for_each(pqueue_t *pqueue, for_each_fn function);
void    pq_destroy(pqueue_t *pqueue);

#ifdef __cplusplus
}
#endif // cpp
#endif // PQUEUE_H
