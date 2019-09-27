#include "tn/heap.h"

// --------------------------------------------------------------------------------------------------------------
int tn_heap_setup(tn_heap_t *heap, size_t size_in_bytes)
{
    TN_ASSERT(heap);
    TN_ASSERT(size_in_bytes);

    heap->capacity = size_in_bytes;
    TN_GUARD_NULL(heap->base = TN_MEM_ACQUIRE(heap->capacity));

    tn_heap_reset(heap);

    return TN_SUCCESS;
}

// --------------------------------------------------------------------------------------------------------------
void tn_heap_cleanup(tn_heap_t *heap)
{
    TN_ASSERT(heap);
    TN_MEM_RELEASE(heap->base);
}

// --------------------------------------------------------------------------------------------------------------
void *tn_heap_acquire(tn_heap_t *heap, size_t size_in_bytes)
{
    TN_ASSERT(heap);
#ifdef TN_HEAP_ALIGNMENT
    size_in_bytes = TN_HEAP_ALIGNED_PTR(size_in_bytes);
#endif
    const uint64_t head = tn_atomic_fetch_add(&heap->head, size_in_bytes);
    if (heap->capacity < head + size_in_bytes) return NULL;
    return (void *)(heap->base + head);
}

// --------------------------------------------------------------------------------------------------------------
void *tn_heap_acquire_aligned(tn_heap_t *heap, size_t size_in_bytes, size_t align_size)
{
    TN_ASSERT(heap);
    const uint64_t head = tn_atomic_fetch_add(&heap->head, size_in_bytes);
    if (heap->capacity < head + size_in_bytes) return NULL;
    return (void *)(heap->base + head);
}

// --------------------------------------------------------------------------------------------------------------
void tn_heap_reset(tn_heap_t *heap)
{
    TN_ASSERT(heap);
#ifdef TN_HEAP_ALIGNMENT
    tn_atomic_store(&heap->head, TN_HEAP_ALIGNED_PTR((uintptr_t)heap->base) - (uintptr_t)heap->base);
#else
    tn_atomic_store(&heap->head, 0);
#endif
}
