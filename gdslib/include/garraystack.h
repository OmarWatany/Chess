#ifndef ASTACK_H
#define ASTACK_H

#ifdef __cplusplus
extern "C" {
#endif // cpp

#include "gds_types.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#define astack_push_str(L, S) (astack_push_safe((L), strlen((S)), (S)))
// #define astack_dpush_str(L, S) (alist_dpush_safe((L), strlen((S)), (S)))

astack_t *astack_create(size_t item_size);

int16_t astack_init(astack_t *stack, size_t item_size);
int16_t astack_push(astack_t *stack, gdata_t data);
int16_t astack_push_safe(astack_t *stack, size_t item_size, gdata_t data);
int16_t astack_pop(astack_t *stack);
size_t  astack_size(astack_t *stack);
void   *astack_peak(astack_t *stack);
bool    astack_empty(astack_t *stack);
void    astack_set_allocator(astack_t *stack, allocator_fun_t allocator);

void astack_destroy(astack_t *stack);

#ifdef __cplusplus
}
#endif // cpp
#endif // !STACK_H
