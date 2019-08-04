#include "tn/buffer.h"

#include "tn/allocator.h"
#include "tn/buffer_pool.h"
#include "tn/error.h"
#include "tn/log.h"

// --------------------------------------------------------------------------------------------------------------
int tn_buffer_setup(tn_buffer_t *buffer, void *src, size_t capacity)
{
    TN_ASSERT(buffer && src && capacity);

    buffer->pool = NULL;
    buffer->buf = aws_byte_buf_from_empty_array(src, capacity);
    buffer->pos = aws_byte_cursor_from_buf(&buffer->buf);
    return TN_SUCCESS;
}

// --------------------------------------------------------------------------------------------------------------
int tn_buffer_read(tn_buffer_t *buffer, void *dst_buffer, size_t len)
{
    TN_ASSERT(buffer);
    TN_ASSERT(dst_buffer);
    if (!aws_byte_cursor_read(&buffer->pos, dst_buffer, len)) return TN_ERROR;
    return TN_SUCCESS;
}

// --------------------------------------------------------------------------------------------------------------
int tn_buffer_read_seek(tn_buffer_t *buffer, tn_buffer_span_t *span)
{
    TN_ASSERT(buffer);
    TN_ASSERT(span && span->ptr && span->len);
    TN_ASSERT((span->ptr + span->len) < (buffer->buf.buffer + buffer->buf.capacity));
    buffer->pos.ptr = span->ptr;
    buffer->pos.len = span->len;
    return TN_SUCCESS;
}

// --------------------------------------------------------------------------------------------------------------
int tn_buffer_read_skip(tn_buffer_t *buffer, size_t len)
{
    TN_ASSERT(buffer);
    struct aws_byte_cursor span = aws_byte_cursor_advance(&buffer->pos, len);
    if (span.ptr && span.len) {
        return TN_SUCCESS;
    }
    return TN_ERROR;
}

// --------------------------------------------------------------------------------------------------------------
int tn_buffer_read_u8(tn_buffer_t *buffer, uint8_t *out_val)
{
    TN_ASSERT(buffer);
    TN_ASSERT(out_val);
    if (!aws_byte_cursor_read_u8(&buffer->pos, out_val)) return TN_ERROR;
    return TN_SUCCESS;
}

// --------------------------------------------------------------------------------------------------------------
int tn_buffer_read_be16(tn_buffer_t *buffer, uint16_t *out_val)
{
    TN_ASSERT(buffer);
    TN_ASSERT(out_val);
    if (!aws_byte_cursor_read_be16(&buffer->pos, out_val)) return TN_ERROR;
    return TN_SUCCESS;
}

// --------------------------------------------------------------------------------------------------------------
int tn_buffer_read_be32(tn_buffer_t *buffer, uint32_t *out_val)
{
    TN_ASSERT(buffer);
    TN_ASSERT(out_val);
    if (!aws_byte_cursor_read_be32(&buffer->pos, out_val)) return TN_ERROR;
    return TN_SUCCESS;
}

// --------------------------------------------------------------------------------------------------------------
int tn_buffer_read_be64(tn_buffer_t *buffer, uint64_t *out_val)
{
    TN_ASSERT(buffer);
    TN_ASSERT(out_val);
    if (!aws_byte_cursor_read_be64(&buffer->pos, out_val)) return TN_ERROR;
    return TN_SUCCESS;
}

// --------------------------------------------------------------------------------------------------------------
int tn_buffer_read_buffer(tn_buffer_t *buffer, tn_buffer_t *dst_buffer, size_t len)
{
    TN_ASSERT(buffer && dst_buffer);

    if (!len) {
        len = buffer->pos.len;
    } else if (len > buffer->pos.len) {
        return TN_ERROR;
    }

    TN_GUARD(tn_buffer_write(dst_buffer, buffer->pos.ptr, len));

    return TN_SUCCESS;
}

// --------------------------------------------------------------------------------------------------------------
int tn_buffer_write(tn_buffer_t *buffer, void *src_buffer, size_t len)
{
    TN_ASSERT(buffer);
    if (!aws_byte_buf_write(&buffer->buf, src_buffer, len)) return TN_ERROR;
    buffer->pos.len += len;
    return TN_SUCCESS;
}

// --------------------------------------------------------------------------------------------------------------
int tn_buffer_write_u8(tn_buffer_t *buffer, uint8_t val)
{
    TN_ASSERT(buffer);
    if (!aws_byte_buf_write_u8(&buffer->buf, val)) return TN_ERROR;
    buffer->pos.len += sizeof(val);
    return TN_SUCCESS;
}

// --------------------------------------------------------------------------------------------------------------
int tn_buffer_write_be16(tn_buffer_t *buffer, uint16_t val)
{
    TN_ASSERT(buffer);
    if (!aws_byte_buf_write_be16(&buffer->buf, val)) return TN_ERROR;
    buffer->pos.len += sizeof(val);
    return TN_SUCCESS;
}

// --------------------------------------------------------------------------------------------------------------
int tn_buffer_write_be32(tn_buffer_t *buffer, uint32_t val)
{
    TN_ASSERT(buffer);
    if (!aws_byte_buf_write_be32(&buffer->buf, val)) return TN_ERROR;
    buffer->pos.len += sizeof(val);
    return TN_SUCCESS;
}

// --------------------------------------------------------------------------------------------------------------
int tn_buffer_write_be64(tn_buffer_t *buffer, uint64_t val)
{
    TN_ASSERT(buffer);
    if (!aws_byte_buf_write_be64(&buffer->buf, val)) return TN_ERROR;
    buffer->pos.len += sizeof(val);
    return TN_SUCCESS;
}

// --------------------------------------------------------------------------------------------------------------
int tn_buffer_write_buffer(tn_buffer_t *buffer, tn_buffer_t *src_buffer, size_t len)
{
    return tn_buffer_read_buffer(src_buffer, buffer, len);
}

// --------------------------------------------------------------------------------------------------------------
void tn_buffer_read_reset(tn_buffer_t *buffer)
{
    TN_ASSERT(buffer);
    buffer->pos.ptr = buffer->buf.buffer;
    buffer->pos.len = buffer->buf.len;
}

// --------------------------------------------------------------------------------------------------------------
void tn_buffer_write_reset(tn_buffer_t *buffer)
{
    TN_ASSERT(buffer);
    buffer->buf.len = 0;
    tn_buffer_read_reset(buffer);
}

// --------------------------------------------------------------------------------------------------------------
void tn_buffer_reset(tn_buffer_t *buffer)
{
    tn_buffer_write_reset(buffer);
}

// --------------------------------------------------------------------------------------------------------------
size_t tn_buffer_length(tn_buffer_t *buffer)
{
    TN_ASSERT(buffer);
    return buffer->buf.len;
}

// --------------------------------------------------------------------------------------------------------------
size_t tn_buffer_read_length(tn_buffer_t *buffer)
{
    TN_ASSERT(buffer);
    return buffer->pos.len;
}

// --------------------------------------------------------------------------------------------------------------
int tn_buffer_set_length(tn_buffer_t *buffer, size_t len)
{
    TN_ASSERT(buffer);
    TN_GUARD(len > (buffer->buf.capacity - buffer->buf.len));
    buffer->buf.len = len;
    buffer->pos.len = len;
    return TN_SUCCESS;
}

// --------------------------------------------------------------------------------------------------------------
int tn_buffer_add_length(tn_buffer_t *buffer, size_t len)
{
    TN_ASSERT(buffer);
    TN_GUARD(len > (buffer->buf.capacity - buffer->buf.len));
    buffer->buf.len += len;
    buffer->pos.len += len;
    return TN_SUCCESS;
}

// --------------------------------------------------------------------------------------------------------------
size_t tn_buffer_remaining(tn_buffer_t *buffer)
{
    TN_ASSERT(buffer);
    return buffer->buf.capacity - buffer->buf.len;
}

// --------------------------------------------------------------------------------------------------------------
size_t tn_buffer_capacity(tn_buffer_t *buffer)
{
    TN_ASSERT(buffer);
    return buffer->pool->block_size;
}

// --------------------------------------------------------------------------------------------------------------
void *tn_buffer_write_ptr(tn_buffer_t *buffer)
{
    TN_ASSERT(buffer);
    return buffer->buf.buffer + buffer->buf.len;
}

// --------------------------------------------------------------------------------------------------------------
void *tn_buffer_read_ptr(tn_buffer_t *buffer)
{
    TN_ASSERT(buffer);
    return buffer->pos.ptr;
}

// --------------------------------------------------------------------------------------------------------------
int tn_buffer_release(tn_buffer_t *buffer)
{
    TN_ASSERT(buffer);
    TN_GUARD_NULL(buffer->pool);

    return tn_buffer_pool_push(buffer->pool, buffer);
}
