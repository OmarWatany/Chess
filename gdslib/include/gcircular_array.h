#ifndef ringbfr_H
#define ringbfr_H

#include "gallocator.h"
#ifdef __cplusplus
extern "C" {
#endif // cpp

#include "gds_types.h"
#include "gnode.h"
#include <stdbool.h>
#include <stdlib.h>

// TODO : check for errors
circular_array_t *carray_create(size_t item_size, size_t capacity);

int16_t carray_init(circular_array_t *ringbfr, size_t item_size, size_t capacity);
int16_t carray_write_safe(circular_array_t *list, size_t item_size, gdata_t data);
int16_t carray_write(circular_array_t *list, gdata_t data);
int16_t carray_overwrite_safe(circular_array_t *list, size_t item_size, gdata_t data);
int16_t carray_overwrite(circular_array_t *list, gdata_t data);
gdata_t carray_read(circular_array_t *list);
size_t  carray_size(circular_array_t *list);
size_t  carray_capacity(circular_array_t *list);
void    carray_set_allocator(circular_array_t *ringbfr, gdata_t (*allocator_fun)(gdata_t data));
bool    carray_empty(circular_array_t *list);
void    carray_destroy(circular_array_t *list);

#ifdef __cplusplus
}
#endif // cpp
#endif // !ringbfr_H
