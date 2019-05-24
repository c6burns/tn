#include "tn/atomic.h"

#include "aws/common/atomics.h"


uint64_t tn_atomic_load(const volatile tn_atomic_t *a) { return aws_atomic_load_int_explicit((const volatile struct aws_atomic_var *)a, TN_ATOMIC_ACQUIRE); }
uint64_t tn_atomic_load_explicit(const volatile tn_atomic_t *a, int mem_order) { return aws_atomic_load_int_explicit((const volatile struct aws_atomic_var *)a, mem_order); }

void *tn_atomic_load_ptr(const volatile tn_atomic_t *a) { return aws_atomic_load_ptr_explicit((const volatile struct aws_atomic_var *)a, TN_ATOMIC_ACQUIRE); }
void *tn_atomic_load_ptr_explicit(const volatile tn_atomic_t *a, int mem_order) { return aws_atomic_load_ptr_explicit((const volatile struct aws_atomic_var *)a, mem_order); }

void tn_atomic_store(volatile tn_atomic_t *a, uint64_t val) { aws_atomic_store_int_explicit((volatile struct aws_atomic_var *)a, val, TN_ATOMIC_RELEASE); }
void tn_atomic_store_explicit(volatile tn_atomic_t *a, uint64_t val, int mem_order) { aws_atomic_store_int_explicit((volatile struct aws_atomic_var *)a, val, mem_order); }

void tn_atomic_store_ptr(volatile tn_atomic_t *a, void *val) { aws_atomic_store_ptr_explicit((volatile struct aws_atomic_var *)a, val, TN_ATOMIC_RELEASE); }
void tn_atomic_store_ptr_explicit(volatile tn_atomic_t *a, void *val, int mem_order) { aws_atomic_store_ptr_explicit((volatile struct aws_atomic_var *)a, val, mem_order); }

uint64_t tn_atomic_fetch_add(volatile tn_atomic_t *a, uint64_t val) { return aws_atomic_fetch_add_explicit((volatile struct aws_atomic_var *)a, val, TN_ATOMIC_ACQ_REL); }
uint64_t tn_atomic_store_fetch_add(volatile tn_atomic_t *a, uint64_t val, int mem_order) { return aws_atomic_fetch_add_explicit((volatile struct aws_atomic_var *)a, val, TN_ATOMIC_ACQ_REL); }
