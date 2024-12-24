#ifndef GALLOCATOR_T
#define GALLOCATOR_T

#ifdef __cplusplus
extern "C" {
#endif // cpp

#include <stdlib.h>

typedef void *gdata_t;
typedef gdata_t (*allocator_fun_t)(gdata_t data);
typedef int (*deallocator_fun_t)(gdata_t data);

typedef struct {
    allocator_fun_t   allocator;
    deallocator_fun_t deallocator;
} Gallocator_t;

extern Gallocator_t *default_gallocator;

gdata_t default_allocator(size_t item_size, gdata_t data);
gdata_t default_safe_allocator(size_t allocation_size, size_t item_size, gdata_t data);
gdata_t str_allocator(gdata_t str);
int     default_deallocator(gdata_t data);

#ifdef __cplusplus
}
#endif // cpp
#endif // !GALLOCATOR_T
