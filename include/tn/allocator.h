#ifndef TN_ALLOCATOR_H
#define TN_ALLOCATOR_H

#include <stdlib.h>

#include "aws/common/common.h"

#define TN_MEM_DEBUG 0
#define TN_MEM_1K 1024
#define TN_MEM_1M TN_MEM_1K * 1024
#define TN_MEM_1G TN_MEM_1M * 1024

#if TN_MEM_DEBUG
#    define TN_MEM_ACQUIRE(sz) aws_mem_acquire(&tn_aws_default_allocator, sz, __FILENAME__, __LINE__, __FUNCTION__);
#    define TN_MEM_RELEASE(ptr) aws_mem_release(&tn_aws_default_allocator, ptr, __FILENAME__, __LINE__, __FUNCTION__);
#else
#    define TN_MEM_ACQUIRE(sz) tn_allocator_acquire(&tn_default_allocator, sz)
#    define TN_MEM_RELEASE(ptr) tn_allocator_release(&tn_default_allocator, ptr)
#    define TN_MEM_RELEASE_PTR(ptr) tn_allocator_release_ptr(&tn_default_allocator, ptr)
#endif

typedef struct tn_allocator_config_s {
    void *priv;
} tn_allocator_config_t;

struct tn_allocator_s;
#if TN_MEM_DEBUG
typedef void *(*tn_allocator_aquire_fn)(const struct tn_allocator_s *allocator, size_t sz, const char *file_name, const int line, const char *function_name);
typedef void (*tn_allocator_release_fn)(const struct tn_allocator_s *allocator, void *ptr, const char *file_name, const int line, const char *function_name);
typedef void (*tn_allocator_release_ptr_fn)(const struct tn_allocator_s *allocator, void **ptr, const char *file_name, const int line, const char *function_name);
#else
typedef void *(*tn_allocator_aquire_fn)(const struct tn_allocator_s *allocator, size_t sz);
typedef void (*tn_allocator_release_fn)(const struct tn_allocator_s *allocator, void *ptr);
typedef void (*tn_allocator_release_ptr_fn)(const struct tn_allocator_s *allocator, void **ptr);
#endif

typedef struct aws_allocator aws_allocator_t;
typedef struct tn_allocator_s {
    tn_allocator_aquire_fn acquire_fn;
    tn_allocator_release_fn release_fn;
    tn_allocator_release_ptr_fn release_ptr_fn;
    tn_allocator_config_t config;
    void *priv;
} tn_allocator_t;

tn_allocator_t *tn_allocator_new(void);
void tn_allocator_delete(tn_allocator_t **ptr_allocator);
int tn_allocator_setup(tn_allocator_t *allocator, const tn_allocator_config_t *config, tn_allocator_aquire_fn acquire_fn, tn_allocator_release_fn release_fn);
int tn_allocator_cleanup(tn_allocator_t *allocator);

#if TN_MEM_DEBUG
void *tn_allocator_acquire(const tn_allocator_t *allocator, size_t sz, const char *file_name, const int line, const char *function_name);
void tn_allocator_release(const tn_allocator_t *allocator, void *ptr, const char *file_name, const int line, const char *function_name);
void tn_allocator_release_ptr(const tn_allocator_t *allocator, void **ptr, const char *file_name, const int line, const char *function_name);
#else
void *tn_allocator_acquire(const tn_allocator_t *allocator, size_t sz);
void tn_allocator_release(const tn_allocator_t *allocator, void *ptr);
void tn_allocator_release_ptr(const tn_allocator_t *allocator, void **ptr);
#endif

// these are now private within the translation unit
//void *tn_aws_default_acquire(struct aws_allocator *allocator, size_t size);
//void *tn_aws_default_realloc(struct aws_allocator *allocator, void **ptr, size_t oldsize, size_t newsize);
//void tn_aws_default_release(struct aws_allocator *allocator, void *ptr);

// this provides a default allocator we can use right away without creating anything custom
tn_allocator_t tn_default_allocator;

aws_allocator_t tn_aws_default_allocator;

#endif
