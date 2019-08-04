#ifndef TN_BUFFER_POOL_H
#define TN_BUFFER_POOL_H

#include <stdint.h>

#include "aws/common/byte_buf.h"
#include "aws/common/byte_order.h"

#include "tn/buffer.h"
#include "tn/mutex.h"
#include "tn/queue_spsc.h"

typedef struct tn_buffer_pool_s {
    uint64_t blocks_inuse;
    uint64_t bytes_inuse;
    uint64_t block_size;
    uint64_t block_count;
    tn_buffer_t *tn_buffers;
    uint8_t *allocation;
    tn_queue_spsc_t tn_buffers_free;
} tn_buffer_pool_t;

int tn_buffer_pool_setup(tn_buffer_pool_t *pool, uint64_t block_count, uint64_t block_size);
void tn_buffer_pool_cleanup(tn_buffer_pool_t *pool);

int tn_buffer_pool_push(tn_buffer_pool_t *pool, tn_buffer_t *buffer);
int tn_buffer_pool_peek(tn_buffer_pool_t *pool, tn_buffer_t **out_buffer);
int tn_buffer_pool_pop(tn_buffer_pool_t *pool);
void tn_buffer_pool_pop_cached(tn_buffer_pool_t *pool);
int tn_buffer_pool_pop_back(tn_buffer_pool_t *pool, tn_buffer_t **out_buffer);

#endif
