#ifndef STACK_H
#define STACK_H

#ifdef __cplusplus
extern "C" {
#endif // cpp

#include "gds_types.h"

stack_t *stack_create(size_t item_size);

int16_t stack_push(stack_t *stack, gdata_t data);
void    stack_pop(stack_t *stack);
void   *stack_peak(stack_t *stack);
bool    stack_empty(stack_t *stack);

void stack_destroy(stack_t *stack);

#ifdef __cplusplus
}
#endif // cpp
#endif // !STACK_H
