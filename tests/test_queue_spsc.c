#include "tn/test_harness.h"

#include "tn/error.h"
#include "tn/allocator.h"
#include "tn/log.h"
#include "tn/atomic.h"
#include "tn/thread.h"
#include "tn/queue_spsc.h"


static uint64_t queue_capacity = (1 << 25);// (1 << 10);
static uint64_t msgs_limit = 10000000;
static uint64_t err_read, err_write;
static uint64_t msgs_processed;
static int threads_quit = 0;
static tn_atomic_t test_ready;
static tn_atomic_t threads_ready;

void consume_spsc_thread_run(void *priv)
{
	uintptr_t prev;
	uint64_t i, count;
	tn_queue_spsc_t *queue = priv;

	const uint64_t capacity = 100000;
	const uint16_t block_size = sizeof(void *);
	const uint64_t capacity_bytes = block_size * capacity;

	uint64_t *num = TN_MEM_ACQUIRE(capacity_bytes);

	uint64_t ready = false;
	tn_atomic_fetch_add(&threads_ready, 1);
	while (!ready) {
		ready = tn_atomic_load(&test_ready);
	}

	err_read = prev = 0;
	while (ready) {
		count = capacity;
		if (tn_queue_spsc_pop_all(queue, (void **)num, &count)) {
			tn_thread_sleep_ms(16);
			err_read++;
			continue;
		}

		for (i = 0; i < count; i++) {
			if (num[i] != (prev + 1)) {
				tn_log_error("returned: %zu but %zu was expected", num[i], (prev + 1));
				ready = 0;
				break;
			}
			prev++;
			if (prev >= msgs_limit) {
				ready = 0;
				break;
			}
		}
	}

	TN_MEM_RELEASE(num);
}


void produce_spsc_thread_run(void *priv)
{
	uintptr_t num = 1;
	tn_queue_spsc_t *queue = priv;

	uint64_t ready = false;
	tn_atomic_fetch_add(&threads_ready, 1);
	while (!ready) {
		ready = tn_atomic_load(&test_ready);
	}

	err_write = 0;
	while (ready) {
		if (tn_queue_spsc_push(queue, (void *)num)) {
			err_write++;
			continue;
		}

		if (num >= msgs_limit) break;
		num++;
	}
}


TN_TEST_CASE_BEGIN(queue_spsc_stress)
	tn_queue_spsc_t queue;
	tn_thread_t consume_thread, produce_thread;
	static uint64_t tstamp_start, tstamp_end;

	assert(((sizeof(queue) % TN_CACHE_LINE_SIZE) == 0));

	msgs_processed = 0;
	err_read = 0;
	err_write = 0;
	threads_quit = 0;

	tn_atomic_store(&test_ready, 0);
	tn_atomic_store(&threads_ready, 0);

	TN_GUARD_CLEANUP(tn_queue_spsc_setup(&queue, queue_capacity));
	TN_GUARD_CLEANUP(tn_thread_launch(&produce_thread, produce_spsc_thread_run, &queue));
	TN_GUARD_CLEANUP(tn_thread_launch(&consume_thread, consume_spsc_thread_run, &queue));

	while (tn_atomic_load(&threads_ready) < 2) {};
	tn_atomic_store(&test_ready, 1);

	tstamp_start = tn_tstamp();

	TN_GUARD_CLEANUP(tn_thread_join(&produce_thread));
	TN_GUARD_CLEANUP(tn_thread_join(&consume_thread));

	tstamp_end = tn_tstamp();

	tn_queue_spsc_cleanup(&queue);

	uint64_t elapsed_ns = tstamp_end - tstamp_start;
	double elapsed_s = (double)elapsed_ns / TN_TIME_NS_PER_S;
	double msgs_per_second = (double)msgs_limit / elapsed_s;

	tn_log_info("tn_queue_spsc test results ---------------------------------------");
	tn_log_info("%zu intptrs transferred in %0.2f seconds: %0.2f per second", msgs_limit, elapsed_s, msgs_per_second);
	tn_log_info("%zu read errors / %zu write errors", err_read, err_write);

	return TN_SUCCESS;

cleanup:
	return TN_ERROR;
}

TN_TEST_CASE_BEGIN(queue_spsc_empty)
	uintptr_t intptr;
	uintptr_t intptr2;
	tn_queue_spsc_t queue;

	ASSERT_SUCCESS(tn_queue_spsc_setup(&queue, 8));
	ASSERT_TRUE(TN_QUEUE_EMPTY == tn_queue_spsc_pop(&queue));
	ASSERT_TRUE(TN_QUEUE_EMPTY == tn_queue_spsc_peek(&queue, (void **)&intptr));
	ASSERT_NULL(intptr);
	ASSERT_TRUE(TN_QUEUE_EMPTY == tn_queue_spsc_pop_back(&queue, (void **)&intptr));
	ASSERT_NULL(intptr);

	intptr = 235235;
	intptr2 = 0;
	ASSERT_SUCCESS(tn_queue_spsc_push(&queue, (void *)intptr));
	ASSERT_SUCCESS(tn_queue_spsc_pop_back(&queue, (void **)&intptr2));
	ASSERT_TRUE(intptr2 == intptr);

	intptr = 34754678;
	intptr2 = 0;
	ASSERT_SUCCESS(tn_queue_spsc_push(&queue, (void *)intptr));
	ASSERT_SUCCESS(tn_queue_spsc_peek(&queue, (void **)&intptr2));
	ASSERT_TRUE(intptr2 == intptr);

	ASSERT_SUCCESS(tn_queue_spsc_pop(&queue));
	ASSERT_TRUE(TN_QUEUE_EMPTY == tn_queue_spsc_pop(&queue));
	ASSERT_TRUE(TN_QUEUE_EMPTY == tn_queue_spsc_peek(&queue, (void **)&intptr2));
	ASSERT_NULL(intptr2);
	ASSERT_TRUE(TN_QUEUE_EMPTY == tn_queue_spsc_pop_back(&queue, (void **)&intptr2));
	ASSERT_NULL(intptr2);

	return TN_SUCCESS;
}

TN_TEST_CASE_BEGIN(queue_spsc_full)
	uintptr_t intptr = 97234;
	uintptr_t intptr2 = 83214;
	tn_queue_spsc_t queue;
	ASSERT_SUCCESS(tn_queue_spsc_setup(&queue, 8));

	for (int i = 0; i < 8; i++) {
		ASSERT_SUCCESS(tn_queue_spsc_push(&queue, (void *)intptr));
	}

	intptr2 = 0;
	ASSERT_TRUE(TN_QUEUE_FULL == tn_queue_spsc_push(&queue, (void *)intptr));
	ASSERT_SUCCESS(tn_queue_spsc_pop_back(&queue, (void **)&intptr2));
	ASSERT_TRUE(intptr2 == intptr);

	intptr2 = 0;
	ASSERT_SUCCESS(tn_queue_spsc_push(&queue, (void *)intptr));
	ASSERT_TRUE(TN_QUEUE_FULL == tn_queue_spsc_push(&queue, (void *)intptr));
	ASSERT_SUCCESS(tn_queue_spsc_peek(&queue, (void **)&intptr2));
	ASSERT_TRUE(intptr2 == intptr);

	ASSERT_TRUE(TN_QUEUE_FULL == tn_queue_spsc_push(&queue, (void *)intptr));

	return TN_SUCCESS;
}

TN_TEST_CASE_BEGIN(queue_spsc_npot)
	tn_queue_spsc_t queue;
	ASSERT_SUCCESS(TN_ERROR_INVAL == tn_queue_spsc_setup(&queue, 11));
	ASSERT_TRUE(16 == tn_queue_spsc_capacity(&queue));
	tn_queue_spsc_cleanup(&queue);

	ASSERT_SUCCESS(tn_queue_spsc_setup(&queue, 16));
	ASSERT_TRUE(16 == tn_queue_spsc_capacity(&queue));
	tn_queue_spsc_cleanup(&queue);

	ASSERT_SUCCESS(tn_queue_spsc_setup(&queue, 17));
	ASSERT_TRUE(32 == tn_queue_spsc_capacity(&queue));
	tn_queue_spsc_cleanup(&queue);

	return TN_SUCCESS;
}

TN_TEST_CASE(test_queue_spsc_stress, queue_spsc_stress)
TN_TEST_CASE(test_queue_spsc_empty, queue_spsc_empty)
TN_TEST_CASE(test_queue_spsc_full, queue_spsc_full)
TN_TEST_CASE(test_queue_spsc_npot, queue_spsc_npot)
