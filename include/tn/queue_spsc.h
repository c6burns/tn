#ifndef TN_QUEUE_SPSC_H
#define TN_QUEUE_SPSC_H

#include <stdint.h>

#include "tn/atomic.h"

#define TN_CACHE_LINE_SIZE 64

typedef struct tn_queue_spsc_s {
    uint64_t capacity;
    uint64_t mask;
    uintptr_t *buffer;
    uint8_t cache_line_pad0[TN_CACHE_LINE_SIZE - sizeof(uint64_t) - sizeof(uint64_t) - sizeof(uintptr_t *)];

    tn_atomic_t head;
    uint8_t cache_line_pad1[TN_CACHE_LINE_SIZE - sizeof(tn_atomic_t)];

    tn_atomic_t tail;
    uint8_t cache_line_pad2[TN_CACHE_LINE_SIZE - sizeof(tn_atomic_t)];

    uint64_t tail_cache;
    uint8_t cache_line_pad3[TN_CACHE_LINE_SIZE - sizeof(uint64_t)];
} tn_queue_spsc_t;

int tn_queue_spsc_setup(tn_queue_spsc_t *q, uint64_t capacity);
void tn_queue_spsc_cleanup(tn_queue_spsc_t *q);
uint64_t tn_queue_spsc_count(tn_queue_spsc_t *q);
uint64_t tn_queue_spsc_capacity(tn_queue_spsc_t *q);
int tn_queue_spsc_empty(tn_queue_spsc_t *q);
int tn_queue_spsc_full(tn_queue_spsc_t *q);
int tn_queue_spsc_push(tn_queue_spsc_t *q, void *ptr);
int tn_queue_spsc_peek(tn_queue_spsc_t *q, void **out_ptr);
int tn_queue_spsc_pop(tn_queue_spsc_t *q);
void tn_queue_spsc_pop_cached(tn_queue_spsc_t *q);
int tn_queue_spsc_pop_back(tn_queue_spsc_t *q, void **out_ptr);
int tn_queue_spsc_pop_all(tn_queue_spsc_t *q, void **out_ptr, uint64_t *out_count);

#endif
