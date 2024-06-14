#ifndef GALLOCATOR_T
#define GALLOCATOR_T

#ifdef __cplusplus
extern "C" {
#endif // cpp

#include <stdlib.h>

typedef void *gdata_t;
typedef gdata_t (*allocator_fun_t)(gdata_t data);

gdata_t default_allocator(size_t item_size, gdata_t data);
gdata_t str_allocator(gdata_t str);

#ifdef __cplusplus
}
#endif // cpp
#endif // !GALLOCATOR_T
