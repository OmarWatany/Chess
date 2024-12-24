#ifndef GNODE_T
#define GNODE_T

#ifdef __cplusplus
extern "C" {
#endif // cpp

#include <stdint.h>

typedef void *gdata_t;

typedef struct {
    gdata_t data;
} gnode_t;

typedef struct {
    gnode_t  *data;
    uintptr_t link;
} lnode_t;

typedef struct tnode_t tnode_t;
struct tnode_t {
    gnode_t  *data;
    tnode_t **links;
};

typedef uint8_t anode_t;

#ifdef __cplusplus
}
#endif // cpp
#endif // GNODE_T
