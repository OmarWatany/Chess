#include "glinkedlist.h"
#include <stdbool.h>

#ifndef STACK_H
#define STACK_H

#ifdef __cplusplus
extern "C" {
#endif // cpp

typedef struct stack_t stack_t;

stack_t *create_stack(size_t item_size);

int16_t stack_push(stack_t *stack, gdata_t data);
void    stack_pop(stack_t *stack);
void   *stack_peak(stack_t *stack);
bool    stack_is_empty(stack_t *stack);

void destroy_stack(stack_t **stack);
void clear_stack(stack_t *stack);

#ifdef __cplusplus
}
#endif // cpp
#endif // !STACK_H
