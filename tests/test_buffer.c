#include "tn/test_harness.h"
#include "tn/buffer.h"
#include "tn/buffer_pool.h"

TN_TEST_CASE_BEGIN(test_buffer)
	tn_buffer_pool_t pool;
	tn_buffer_t *buffer1 = NULL;
	tn_buffer_t *buffer2 = NULL;
	size_t blocks = 128;

	ASSERT_SUCCESS(tn_buffer_pool_setup(&pool, blocks, 8));
	tn_buffer_pool_cleanup(&pool);

	ASSERT_SUCCESS(tn_buffer_pool_setup(&pool, blocks, 8));
	for (int i = 0; i < blocks; i++) {
		ASSERT_SUCCESS(tn_buffer_pool_pop(&pool));
	}
	ASSERT_TRUE(0 != tn_buffer_pool_pop(&pool));
	tn_buffer_pool_cleanup(&pool);

	ASSERT_SUCCESS(tn_buffer_pool_setup(&pool, blocks, 8));
	ASSERT_SUCCESS(tn_buffer_pool_pop_back(&pool, &buffer1));
	ASSERT_TRUE(pool.blocks_inuse == 1);
	ASSERT_SUCCESS(tn_buffer_pool_peek(&pool, &buffer2));
	ASSERT_TRUE(pool.blocks_inuse == 1);
	ASSERT_SUCCESS(tn_buffer_pool_pop_back(&pool, &buffer2));
	ASSERT_TRUE(pool.blocks_inuse == 2);
	ASSERT_SUCCESS(tn_buffer_pool_push(&pool, buffer1));
	ASSERT_TRUE(pool.blocks_inuse == 1);
	ASSERT_SUCCESS(tn_buffer_release(buffer2));
	ASSERT_TRUE(pool.blocks_inuse == 0);
	ASSERT_TRUE(0 != tn_buffer_pool_push(&pool, buffer1));
	ASSERT_TRUE(pool.blocks_inuse == 0);
	ASSERT_TRUE(0 != tn_buffer_release(buffer2));
	ASSERT_TRUE(pool.blocks_inuse == 0);
	ASSERT_SUCCESS(0 != tn_buffer_pool_peek(&pool, &buffer1));
	ASSERT_TRUE(pool.blocks_inuse == 0);

	buffer1 = NULL;
	buffer2 = NULL;

	ASSERT_SUCCESS(tn_buffer_pool_pop_back(&pool, &buffer1));
	ASSERT_SUCCESS(tn_buffer_pool_pop_back(&pool, &buffer2));

	ASSERT_SUCCESS(tn_buffer_pool_push(&pool, buffer1));
	ASSERT_SUCCESS(tn_buffer_release(buffer2));
	tn_buffer_pool_cleanup(&pool);

	return TN_SUCCESS;
}

TN_TEST_CASE(test_buffer_pool, test_buffer)
