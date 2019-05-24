#include "tn/queue_spsc.h"

#include "aws/common/atomics.h"

#include "tn/error.h"
#include "tn/allocator.h"
#include "tn/log.h"
#include "tn/atomic.h"


// --------------------------------------------------------------------------------------------------------------
int tn_queue_spsc_setup(tn_queue_spsc_t *q, uint64_t capacity)
{
	TN_ASSERT(q);
	TN_ASSERT(capacity > 0);

	if (capacity & (capacity - 1)) {
		capacity--;
		capacity |= capacity >> 1;
		capacity |= capacity >> 2;
		capacity |= capacity >> 4;
		capacity |= capacity >> 8;
		capacity |= capacity >> 16;
		capacity |= capacity >> 32;
		capacity++;
	}
	TN_ASSERT(!(capacity & (capacity - 1)));

	int ret = TN_ERROR_NOMEM;
	q->buffer = NULL;
	q->buffer = TN_MEM_ACQUIRE(sizeof(*q->buffer) * (1 + capacity));
	TN_GUARD_NULL(q->buffer);

	tn_atomic_store(&q->head, 0);
	tn_atomic_store(&q->tail, 0);
	q->tail_cache = 0;
	q->capacity = capacity;
	q->mask = capacity - 1;

	return TN_SUCCESS;
}

// --------------------------------------------------------------------------------------------------------------
void tn_queue_spsc_cleanup(tn_queue_spsc_t *q)
{
	TN_ASSERT(q);
	TN_MEM_RELEASE(q->buffer);
}

// --------------------------------------------------------------------------------------------------------------
uint64_t tn_queue_spsc_count(tn_queue_spsc_t *q)
{
	TN_ASSERT(q);
	const uint64_t head = tn_atomic_load_explicit(&q->head, TN_ATOMIC_RELAXED);
	const uint64_t tail = tn_atomic_load(&q->tail);
	return (head - tail);
}

// --------------------------------------------------------------------------------------------------------------
uint64_t tn_queue_spsc_capacity(tn_queue_spsc_t *q)
{
	TN_ASSERT(q);
	return q->capacity;
}

// --------------------------------------------------------------------------------------------------------------
int tn_queue_spsc_empty(tn_queue_spsc_t *q)
{
	return (tn_queue_spsc_count(q) == 0);
}

// --------------------------------------------------------------------------------------------------------------
int tn_queue_spsc_full(tn_queue_spsc_t *q)
{
	return (tn_queue_spsc_count(q) == q->capacity);
}

// --------------------------------------------------------------------------------------------------------------
int tn_queue_spsc_push(tn_queue_spsc_t *q, void *ptr)
{
	TN_ASSERT(q);

	const uint64_t head = tn_atomic_load_explicit(&q->head, TN_ATOMIC_RELAXED);
	const uint64_t tail = tn_atomic_load(&q->tail);
	const uint64_t size = head - tail;

	if (size == q->capacity) return TN_QUEUE_FULL;

	q->buffer[head & q->mask] = (uintptr_t)ptr;
	tn_atomic_store(&q->head, head + 1);

	return TN_SUCCESS;
}

// --------------------------------------------------------------------------------------------------------------
int tn_queue_spsc_peek(tn_queue_spsc_t *q, void **out_ptr)
{
	TN_ASSERT(q);
	TN_ASSERT(out_ptr);

	*out_ptr = NULL;

	const uint64_t tail = tn_atomic_load_explicit(&q->tail, TN_ATOMIC_RELAXED);
	const uint64_t head = tn_atomic_load(&q->head);

	if (head == tail) return TN_QUEUE_EMPTY;

	*out_ptr = (void *)q->buffer[tail & q->mask];
	q->tail_cache = tail;

	return TN_SUCCESS;
}

// --------------------------------------------------------------------------------------------------------------
int tn_queue_spsc_pop(tn_queue_spsc_t *q)
{
	TN_ASSERT(q);

	const uint64_t tail = tn_atomic_load_explicit(&q->tail, TN_ATOMIC_RELAXED);
	const uint64_t head = tn_atomic_load(&q->head);

	if (head == tail) return TN_QUEUE_EMPTY;

	tn_atomic_store(&q->tail, tail + 1);

	return TN_SUCCESS;
}

// --------------------------------------------------------------------------------------------------------------
void tn_queue_spsc_pop_cached(tn_queue_spsc_t *q)
{
	tn_atomic_store(&q->tail, q->tail_cache + 1);
}

// --------------------------------------------------------------------------------------------------------------
int tn_queue_spsc_pop_back(tn_queue_spsc_t *q, void **out_ptr)
{
	TN_ASSERT(q);
	TN_ASSERT(out_ptr);

	*out_ptr = NULL;

	const uint64_t tail = tn_atomic_load_explicit(&q->tail, TN_ATOMIC_RELAXED);
	const uint64_t head = tn_atomic_load(&q->head);

	if (head == tail) return TN_QUEUE_EMPTY;

	*out_ptr = (void *)q->buffer[tail & q->mask];

	tn_atomic_store(&q->tail, tail + 1);

	return TN_SUCCESS;
}

// --------------------------------------------------------------------------------------------------------------
int tn_queue_spsc_pop_all(tn_queue_spsc_t *q, void **out_ptr, uint64_t *out_count)
{
	TN_ASSERT(q);
	TN_ASSERT(out_ptr);
	TN_ASSERT(out_count);
	TN_ASSERT(*out_count > 0);

	const uint64_t mask = q->mask;
	const uint64_t tail = tn_atomic_load_explicit(&q->tail, TN_ATOMIC_RELAXED);
	const uint64_t head = tn_atomic_load(&q->head);
	const uint64_t size = (head - tail);

	if (!size) return TN_QUEUE_EMPTY;

	if (size < *out_count) *out_count = size;
	for (uint64_t off = 0; off < *out_count; off++) {
		out_ptr[off] = (void *)q->buffer[(tail + off) & q->mask];
	}

	tn_atomic_store_explicit(&q->tail, tail + *out_count, TN_ATOMIC_RELAXED);

	return TN_SUCCESS;
}
