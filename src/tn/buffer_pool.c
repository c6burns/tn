#include "tn/buffer_pool.h"

#include "tn/allocator.h"
#include "tn/buffer.h"
#include "tn/error.h"
#include "tn/log.h"

// --------------------------------------------------------------------------------------------------------------
int tn_buffer_pool_setup(tn_buffer_pool_t *pool, uint64_t block_count, uint64_t block_size)
{
    TN_ASSERT(pool);
    TN_ASSERT(block_count > 0);
    TN_ASSERT(block_size > 0);

    pool->block_count = block_count;
    pool->block_size = block_size;
    pool->blocks_inuse = 0;
    pool->bytes_inuse = 0;
    pool->tn_buffers = NULL;
    pool->allocation = NULL;

    TN_GUARD_NULL_CLEANUP(pool->allocation = TN_MEM_ACQUIRE(block_count * block_size));
    TN_GUARD_NULL_CLEANUP(pool->tn_buffers = TN_MEM_ACQUIRE(block_count * sizeof(*pool->tn_buffers)));
    TN_GUARD_CLEANUP(tn_queue_spsc_setup(&pool->tn_buffers_free, block_count));

    tn_buffer_t *tn_buffer;
    for (uint64_t i = 0; i < block_count; i++) {
        tn_buffer = &pool->tn_buffers[i];

        uint8_t *buf = pool->allocation + ((block_count - 1 - i) * block_size);

        TN_GUARD_CLEANUP(tn_buffer_setup(tn_buffer, buf, block_size));
        tn_buffer->pool = pool;

        TN_GUARD_CLEANUP(tn_queue_spsc_push(&pool->tn_buffers_free, tn_buffer));
    }

    return TN_SUCCESS;

cleanup:
    TN_MEM_RELEASE(pool->tn_buffers);
    TN_MEM_RELEASE(pool->allocation);

    return TN_ERROR;
}

// --------------------------------------------------------------------------------------------------------------
void tn_buffer_pool_cleanup(tn_buffer_pool_t *pool)
{
    if (!pool) return;

    pool->blocks_inuse = 0;
    pool->bytes_inuse = 0;

    tn_queue_spsc_cleanup(&pool->tn_buffers_free);
    TN_MEM_RELEASE(pool->tn_buffers);
    TN_MEM_RELEASE(pool->allocation);
}

// --------------------------------------------------------------------------------------------------------------
int tn_buffer_pool_push(tn_buffer_pool_t *pool, tn_buffer_t *buffer)
{
    TN_ASSERT(pool);
    TN_ASSERT(buffer);

    TN_GUARD(tn_queue_spsc_push(&pool->tn_buffers_free, buffer));

    pool->blocks_inuse--;
    pool->bytes_inuse -= pool->block_size;

    return TN_SUCCESS;
}

// --------------------------------------------------------------------------------------------------------------
int tn_buffer_pool_peek(tn_buffer_pool_t *pool, tn_buffer_t **out_buffer)
{
    TN_ASSERT(pool);
    TN_ASSERT(out_buffer);

    *out_buffer = NULL;

    TN_GUARD(tn_queue_spsc_peek(&pool->tn_buffers_free, (void **)out_buffer));
    TN_ASSERT(*out_buffer);

    tn_buffer_reset(*out_buffer);

    return TN_SUCCESS;
}

// --------------------------------------------------------------------------------------------------------------
int tn_buffer_pool_pop(tn_buffer_pool_t *pool)
{
    TN_ASSERT(pool);

    TN_GUARD(tn_queue_spsc_pop(&pool->tn_buffers_free));

    pool->blocks_inuse++;
    pool->bytes_inuse += pool->block_size;

    return TN_SUCCESS;
}

// --------------------------------------------------------------------------------------------------------------
void tn_buffer_pool_pop_cached(tn_buffer_pool_t *pool)
{
    TN_ASSERT(pool);

    tn_queue_spsc_pop_cached(&pool->tn_buffers_free);

    pool->blocks_inuse++;
    pool->bytes_inuse += pool->block_size;
}

// --------------------------------------------------------------------------------------------------------------
int tn_buffer_pool_pop_back(tn_buffer_pool_t *pool, tn_buffer_t **out_buffer)
{
    TN_ASSERT(pool);
    TN_ASSERT(out_buffer);

    *out_buffer = NULL;

    TN_GUARD(tn_queue_spsc_pop_back(&pool->tn_buffers_free, (void **)out_buffer));
    TN_ASSERT(*out_buffer);

    pool->blocks_inuse++;
    pool->bytes_inuse += pool->block_size;

    tn_buffer_reset(*out_buffer);

    return TN_SUCCESS;
}
