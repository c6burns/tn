#include <string.h>

#include "tn/test_harness.h"
#include "tn/list_block.h"

TN_TEST_CASE_BEGIN(tn_list_block_create)
	tn_list_block_t list;

	ASSERT_SUCCESS(tn_list_block_setup(&list, 128, sizeof(uintptr_t)));
	ASSERT_TRUE(list.capacity == 128);
	ASSERT_TRUE(list.item_size == sizeof(uintptr_t));
	tn_list_block_cleanup(&list);

	ASSERT_SUCCESS(tn_list_block_setup(&list, 8, 128));
	ASSERT_TRUE(list.capacity == 8);
	ASSERT_TRUE(list.item_size == 128);
	tn_list_block_cleanup(&list);

	return TN_SUCCESS;
}

TN_TEST_CASE_BEGIN(tn_list_block_ops)
	tn_list_block_t list;
	uint64_t data0 = 12983712837;
	uint64_t data1 = 97829736300;
	uint64_t *p0 = &data0;
	uint64_t *p1 = &data1;
	uint64_t *out = NULL;
	uint64_t count = 0;
	uint64_t index = 0;

	ASSERT_SUCCESS(tn_list_block_setup(&list, 128, sizeof(uintptr_t)));

	ASSERT_SUCCESS(tn_list_block_count(&list, &count));
	ASSERT_TRUE(count == 0);

	ASSERT_SUCCESS(tn_list_block_push_back(&list, (void **)&p0, &index));
	ASSERT_SUCCESS(tn_list_block_count(&list, &count));
	ASSERT_TRUE(count == 1);

	ASSERT_SUCCESS(tn_list_block_push_back(&list, (void **)&p1, &index));
	ASSERT_SUCCESS(tn_list_block_count(&list, &count));
	ASSERT_TRUE(count == 2);

	ASSERT_SUCCESS(tn_list_block_remove(&list, 0));
	ASSERT_SUCCESS(tn_list_block_count(&list, &count));
	ASSERT_TRUE(count == 1);

	ASSERT_SUCCESS(tn_list_block_pop_back(&list, (void **)&out));
	ASSERT_SUCCESS(tn_list_block_count(&list, &count));
	ASSERT_TRUE(count == 0);
	ASSERT_TRUE(out == p1);

	tn_list_block_cleanup(&list);

	return TN_SUCCESS;
}

TN_TEST_CASE(tn_list_block_create);
TN_TEST_CASE(tn_list_block_ops);
