#ifndef _GRINGBUFFER_H_
#define _GRINGBUFFER_H_

#define BUFFER_BACKEND_LIBC_MALLOC        0
#define BUFFER_BACKEND_LINUX_MMAP         1
#define BUFFER_BACKEND_WIN32_VIRTUALALLOC 2

#ifndef BUFFER_BACKEND
#if defined(_WIN32)
#define BUFFER_BACKEND BUFFER_BACKEND_WIN32_VIRTUALALLOC
#else
#define BUFFER_BACKEND BUFFER_BACKEND_LINUX_MMAP
#endif
#endif // BUFFER_BACKEND

#include "gds_types.h"
#include <unistd.h>

void *ringbuffer_alloc(size_t *);

void ring_init(ringbuffer *buffer, size_t size);

int ring_write(ringbuffer *buffer, gdata_t data, size_t size);

int ring_read(ringbuffer *buffer, void *target, size_t size);

gdata_t ring_read_return(ringbuffer *buffer, size_t size);

void ring_reset(ringbuffer *buffer);

void ring_destroy(ringbuffer *buffer);

#define ringbuffer_commit_write(B, S) ((B)->write_idx = ((B)->write_idx + (S)) % (B)->size)
#define ringbuffer_commit_read(B, S)  ((B)->read_idx = ((B)->read_idx + (S)) % (B)->size)
#define ringbuffer_write_idx(B)       ((B)->buffer + (B)->write_idx)
#define ringbuffer_read_idx(B)        ((B)->buffer + (B)->read_idx)
#define ringbuffer_space_until_end(B) ((B)->size - (B)->write_idx)

#define ringbuffer_remaining_sapce(B)                                                              \
    (((B)->write_idx >= (B)->read_idx) ? ((B)->size - ((B)->write_idx - (B)->read_idx))            \
                                       : ((B)->read_idx - (B)->write_idx))

#define ringbuffer_available_data(B)                                                               \
    (((B)->write_idx > (B)->read_idx) ? (((B)->write_idx - (B)->read_idx))                         \
                                      : ((B)->size - (B)->read_idx - (B)->write_idx))
#endif // _GRINGBUFFER_H_
