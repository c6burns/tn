#include "tn/test_harness.h"
#include "tn/event.h"
#include "tn/endpoint.h"

#include <string.h>

TN_TEST_CASE_BEGIN(test_event_list)
	tn_event_list_t event_list;
	tn_event_base_t *evt;
	tn_event_client_open_t *evt_open;
	tn_event_client_open_t *evt_open2;
	uint64_t max_size = TN_EVENT_MAX_SIZE;
	char ipstr[255];
	uint16_t port;

	// test that our sizes are correct (since we pass these to managed)
	ASSERT_TRUE(sizeof(tn_event_base_t) == max_size);
	ASSERT_TRUE(sizeof(tn_event_error_t) <= max_size);
	ASSERT_TRUE(sizeof(tn_event_client_open_t) <= max_size);
	ASSERT_TRUE(sizeof(tn_event_client_close_t) <= max_size);
	ASSERT_TRUE(sizeof(tn_event_client_read_t) <= max_size);
	ASSERT_TRUE(sizeof(tn_event_stats_t) <= max_size);

	ASSERT_SUCCESS(tn_event_list_setup(&event_list, 100));
	tn_event_list_cleanup(&event_list);

	ASSERT_SUCCESS(tn_event_list_setup(&event_list, 100));
	ASSERT_SUCCESS(tn_event_list_free_pop_open(&event_list, &evt_open));
	evt_open->client_id = 874;
	ASSERT_SUCCESS(tn_endpoint_set_ip4(&evt_open->host, "123.56.89.101", 65432));

	ASSERT_TRUE(tn_queue_spsc_count(&event_list.tn_events_free) == event_list.tn_events_free.capacity - 1);
	ASSERT_TRUE(tn_queue_spsc_count(&event_list.tn_events_ready) == 0);

	ASSERT_SUCCESS(tn_event_list_ready_push(&event_list, evt_open));
	ASSERT_TRUE(tn_queue_spsc_count(&event_list.tn_events_free) == event_list.tn_events_free.capacity - 1);
	ASSERT_TRUE(tn_queue_spsc_count(&event_list.tn_events_ready) == 1);

	ASSERT_SUCCESS(tn_event_list_ready_peek(&event_list, &evt));
	ASSERT_TRUE(evt->type == TN_EVENT_CLIENT_OPEN);
	evt_open2 = (tn_event_client_open_t *)evt;
	ASSERT_TRUE(evt_open2->type == TN_EVENT_CLIENT_OPEN);
	ASSERT_TRUE(evt_open2->client_id == 874);
	ASSERT_SUCCESS(tn_endpoint_get(&evt_open2->host, &port, ipstr, sizeof(ipstr)));
	ASSERT_TRUE(port == 65432);
	ASSERT_SUCCESS(strcmp(ipstr, "123.56.89.101"));

	ASSERT_TRUE(tn_queue_spsc_count(&event_list.tn_events_free) == event_list.tn_events_free.capacity - 1);
	ASSERT_TRUE(tn_queue_spsc_count(&event_list.tn_events_ready) == 1);

	tn_event_list_ready_pop_cached(&event_list);
	ASSERT_TRUE(tn_queue_spsc_count(&event_list.tn_events_free) == event_list.tn_events_free.capacity - 1);
	ASSERT_TRUE(tn_queue_spsc_count(&event_list.tn_events_ready) == 0);

	ASSERT_SUCCESS(tn_event_list_free_push(&event_list, evt));
	ASSERT_TRUE(tn_queue_spsc_count(&event_list.tn_events_free) == event_list.tn_events_free.capacity);
	ASSERT_TRUE(tn_queue_spsc_count(&event_list.tn_events_ready) == 0);

	tn_event_list_cleanup(&event_list);


	ASSERT_SUCCESS(tn_event_list_setup(&event_list, 100));
	uint64_t start = 0, end = event_list.tn_events_free.capacity;
	for (int i = 0; i < event_list.tn_events_free.capacity; i++) {
		ASSERT_SUCCESS(tn_event_list_free_pop_back(&event_list, &evt));
		ASSERT_SUCCESS(tn_event_list_ready_push(&event_list, evt));

		start++;
		end--;
		ASSERT_TRUE(tn_queue_spsc_count(&event_list.tn_events_free) == end);
		ASSERT_TRUE(tn_queue_spsc_count(&event_list.tn_events_ready) == start);
	}

	ASSERT_TRUE(0 != tn_event_list_ready_push(&event_list, evt));
	ASSERT_TRUE(0 != tn_event_list_free_pop_back(&event_list, &evt));

	tn_event_list_cleanup(&event_list);


	return TN_SUCCESS;
}

TN_TEST_CASE(test_events, test_event_list);
