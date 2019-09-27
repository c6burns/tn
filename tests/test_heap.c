#include <string.h>

#include "tn/test_harness.h"
#include "tn/heap.h"

TN_TEST_CASE_BEGIN(tn_heap_ops)
	tn_heap_t heap;
    uint8_t *u8;
    uint16_t *u16;
    uint32_t *u32;
    uint64_t *u64;
    uint64_t expected_head = 0;

	ASSERT_SUCCESS(tn_heap_setup(&heap, 4096));
    expected_head = TN_HEAP_ALIGNED_PTR((uintptr_t)heap.base) - (uintptr_t)heap.base;
    ASSERT_NOT_NULL(heap.base);
	ASSERT_TRUE(heap.capacity == 4096);
	ASSERT_TRUE(tn_atomic_load(&heap.head) == expected_head);

    expected_head += TN_HEAP_ALIGNED_PTR(sizeof(*u8));
    ASSERT_NOT_NULL(u8 = tn_heap_acquire(&heap, sizeof(*u8)));
    ASSERT_TRUE(tn_atomic_load(&heap.head) == expected_head);
    *u8 = 1;

    expected_head += TN_HEAP_ALIGNED_PTR(sizeof(*u16));
    ASSERT_NOT_NULL(u16 = tn_heap_acquire(&heap, sizeof(*u16)));
    ASSERT_TRUE(tn_atomic_load(&heap.head) == expected_head);
    *u16 = 2;

    expected_head += TN_HEAP_ALIGNED_PTR(sizeof(*u32));
    ASSERT_NOT_NULL(u32 = tn_heap_acquire(&heap, sizeof(*u32)));
    ASSERT_TRUE(tn_atomic_load(&heap.head) == expected_head);
    *u32 = 3;

    expected_head += TN_HEAP_ALIGNED_PTR(sizeof(*u64));
    ASSERT_NOT_NULL(u64 = tn_heap_acquire(&heap, sizeof(*u64)));
    ASSERT_TRUE(tn_atomic_load(&heap.head) == expected_head);
    *u64 = 4;

    expected_head = TN_HEAP_ALIGNED_PTR((uintptr_t)heap.base) - (uintptr_t)heap.base;
    tn_heap_reset(&heap);
    ASSERT_TRUE(tn_atomic_load(&heap.head) == expected_head);

    ASSERT_TRUE(1 == *(uint8_t *)tn_heap_acquire(&heap, sizeof(*u8)));
    ASSERT_TRUE(2 == *(uint16_t *)tn_heap_acquire(&heap, sizeof(*u16)));
    ASSERT_TRUE(3 == *(uint32_t *)tn_heap_acquire(&heap, sizeof(*u32)));
    ASSERT_TRUE(4 == *(uint64_t *)tn_heap_acquire(&heap, sizeof(*u64)));

	return TN_SUCCESS;
}

TN_TEST_CASE(tn_heap_ops);
