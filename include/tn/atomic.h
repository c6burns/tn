#ifndef TN_ATOMIC_H
#define TN_ATOMIC_H


#include <stdint.h>


typedef struct tn_atomic_s {
	void *value;
} tn_atomic_t;


// atomic memory barriers
#define TN_ATOMIC_RELAXED 0
#define TN_ATOMIC_ACQUIRE 2
#define TN_ATOMIC_RELEASE 3
#define TN_ATOMIC_ACQ_REL 4
#define TN_ATOMIC_SEQ_CST 5


uint64_t tn_atomic_load(const volatile tn_atomic_t *a);
uint64_t tn_atomic_load_explicit(const volatile tn_atomic_t *a, int mem_order);

void *tn_atomic_load_ptr(const volatile tn_atomic_t *a);
void *tn_atomic_load_ptr_explicit(const volatile tn_atomic_t *a, int mem_order);

void tn_atomic_store(volatile tn_atomic_t *a, uint64_t val);
void tn_atomic_store_explicit(volatile tn_atomic_t *a, uint64_t val, int mem_order);

void tn_atomic_store_ptr(volatile tn_atomic_t *a, void *val);
void tn_atomic_store_ptr_explicit(volatile tn_atomic_t *a, void *val, int mem_order);

uint64_t tn_atomic_fetch_add(volatile tn_atomic_t *a, uint64_t val);
uint64_t tn_atomic_store_fetch_add(volatile tn_atomic_t *a, uint64_t val, int mem_order);

#endif