#ifndef TN_BUFFER_H
#define TN_BUFFER_H

#include <stdint.h>

#include "aws/common/byte_buf.h"
#include "aws/common/byte_order.h"

#include "tn/error.h"
#include "tn/mutex.h"

#if TN_PLATFORM_WINDOWS
#    define TN_BUFLEN_CAST(sz) ((unsigned int)sz)
#else
#    define TN_BUFLEN_CAST(sz) (sz)
#endif

// forwards
struct tn_buffer_pool_s;

typedef struct tn_buffer_span_s {
    size_t len;
    uint8_t *ptr;
} tn_buffer_span_t;

typedef struct tn_buffer_s {
    struct tn_buffer_pool_s *pool;
    struct aws_byte_buf buf;
    struct aws_byte_cursor pos;
    size_t capacity;
} tn_buffer_t;

int tn_buffer_setup(tn_buffer_t *buffer, void *src, size_t capacity);

int tn_buffer_read(tn_buffer_t *buffer, void *dst_buffer, size_t len);
int tn_buffer_read_seek(tn_buffer_t *buffer, tn_buffer_span_t *span);
int tn_buffer_read_skip(tn_buffer_t *buffer, size_t len);
int tn_buffer_read_u8(tn_buffer_t *buffer, uint8_t *out_val);
int tn_buffer_read_be16(tn_buffer_t *buffer, uint16_t *out_val);
int tn_buffer_read_be32(tn_buffer_t *buffer, uint32_t *out_val);
int tn_buffer_read_be64(tn_buffer_t *buffer, uint64_t *out_val);
int tn_buffer_read_buffer(tn_buffer_t *buffer, tn_buffer_t *dst_buffer, size_t len);
int tn_buffer_write(tn_buffer_t *buffer, const void *src_buffer, size_t len);
int tn_buffer_write_u8(tn_buffer_t *buffer, const uint8_t val);
int tn_buffer_write_be16(tn_buffer_t *buffer, const uint16_t val);
int tn_buffer_write_be32(tn_buffer_t *buffer, const uint32_t val);
int tn_buffer_write_be64(tn_buffer_t *buffer, const uint64_t val);
int tn_buffer_write_buffer(tn_buffer_t *buffer, tn_buffer_t *src_buffer, size_t len);

void tn_buffer_read_reset(tn_buffer_t *buffer);
void tn_buffer_write_reset(tn_buffer_t *buffer);
void tn_buffer_reset(tn_buffer_t *buffer);

size_t tn_buffer_length(tn_buffer_t *buffer);
size_t tn_buffer_read_length(tn_buffer_t *buffer);
int tn_buffer_set_length(tn_buffer_t *buffer, size_t len);
int tn_buffer_add_length(tn_buffer_t *buffer, size_t len);
size_t tn_buffer_remaining(tn_buffer_t *buffer);
size_t tn_buffer_capacity(tn_buffer_t *buffer);

void *tn_buffer_write_ptr(tn_buffer_t *buffer);
void *tn_buffer_read_ptr(tn_buffer_t *buffer);

int tn_buffer_release(tn_buffer_t *buffer);

#endif
