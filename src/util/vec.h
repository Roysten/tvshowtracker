#pragma once

#include <stdint.h>
#include <stdlib.h>

typedef struct Vec Vec;
struct Vec
{
    size_t len;
    size_t capacity;
    void *data;
};

#define INIT_CAPACITY 10
#define vec_create(type) (Vec) {.len = 0, .capacity = INIT_CAPACITY, .data = realloc(NULL, INIT_CAPACITY * sizeof(type))}

#define vec_push(type, vec, val) do { \
    if ((vec).len == (vec).capacity) { \
        size_t new_capacity = (vec).capacity * 3 / 2; \
        (vec).data = realloc((vec).data, new_capacity * sizeof(type)); \
        (vec).capacity = new_capacity; \
        if ((vec).data == NULL) exit(42); \
    } \
    ((type*) (vec).data)[(vec).len++] = val; \
} while (false)

#define vec_size(vec) (vec).len

#define vec_destroy(vec) do { \
    free((vec).data); \
    (vec).data = NULL; \
} while (false)

#define vec_get(type, vec, idx) ((type *) &((type*) (vec).data)[(idx)])

#define vec_back(type, vec) (vec_get(type, (vec), (vec).len - 1))
