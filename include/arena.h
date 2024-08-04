#ifndef ARENA_H
#define ARENA_H

#include <stdlib.h>
#include <string.h>

#define CLUSTERS 5

typedef struct {
    void *buf;
    size_t size;
} Cluster;

typedef struct {
    Cluster clusters[CLUSTERS];
    size_t capacity, clusters_count;
} Arena;

Arena *arena_create(size_t buffer_capacity);
void arena_init(Arena *arena, size_t capacity);
void *arena_alloc(Arena *arena, size_t size);
void arena_destroy(Arena *arena);
size_t arena_capacity(Arena *arena);

#ifdef ARENA_IMPLEMENTATION
static int expand(Arena *arena, size_t size);

Cluster cluster_create(size_t capacity) {
    Cluster clstr = {0};
    clstr.buf = malloc(capacity);
    memset(clstr.buf, 0, capacity);
    clstr.size = 0;
    return clstr;
}

Arena *arena_create(size_t capacity) {
    Arena *arena = malloc(sizeof(Arena));
    arena_init(arena, capacity);
    return arena;
}

void arena_init(Arena *arena, size_t capacity) {
    arena->capacity = capacity;
    arena->clusters[0] = cluster_create(capacity);
    arena->clusters_count = 1;
}

void *arena_alloc(Arena *arena, size_t size) {
    size_t expand_size = arena->capacity, n = arena->clusters_count;
    if (size >= arena->capacity) {
        expand_size = size;
    } else {
        for (size_t j = 0; j < arena->clusters_count; j++)
            if (arena->clusters[j].size + size < arena->capacity) n = j;
    }
    if (!arena->clusters[n].buf)
        if (expand(arena, expand_size)) return NULL;

    Cluster *clstr = &arena->clusters[n];
    void *new_buf = clstr->buf + clstr->size;
    clstr->size += size;
    return new_buf;
}

void arena_destroy(Arena *arena) {
    for (size_t j = 0; j < arena->clusters_count; j++)
        free(arena->clusters[j].buf);
}

int expand(Arena *arena, size_t size) {
    if (arena->clusters_count >= CLUSTERS) return -1;
    arena->clusters[arena->clusters_count].buf = malloc(size);
    arena->clusters_count++;
    return 0;
}

size_t arena_capacity(Arena *arena) {
    return arena->capacity;
}

#endif // DEBUG
#endif
