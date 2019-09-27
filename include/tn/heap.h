#ifndef TN_HEAP_H
#define TN_HEAP_H

#include "tn/error.h"
#include "tn/allocator.h"
#include "tn/atomic.h"

#define TN_HEAP_ALIGNMENT 64
#define TN_HEAP_ALIGNED_PTR(ptr) ((ptr + (TN_HEAP_ALIGNMENT - 1)) & -TN_HEAP_ALIGNMENT)

typedef struct tn_heap_s {
    uint8_t *base;
    size_t capacity;
    tn_atomic_t head;
} tn_heap_t;

int tn_heap_setup(tn_heap_t *heap, size_t size_in_bytes);
void tn_heap_cleanup(tn_heap_t *heap);
void *tn_heap_acquire(tn_heap_t *heap, size_t size_in_bytes);
void *tn_heap_acquire_aligned(tn_heap_t *heap, size_t size_in_bytes, size_t align_size);
void tn_heap_reset(tn_heap_t *heap);

#endif
