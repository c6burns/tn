#include <string.h>

#include "tn/test_harness.h"
#include "kn/client_list.h"

TN_TEST_CASE_BEGIN(client_list_create)
	kn_client_list_t list;

	ASSERT_SUCCESS(kn_client_list_setup(&list, 128));
	ASSERT_TRUE(list.capacity == 128);
	kn_client_list_cleanup(&list);

	ASSERT_SUCCESS(kn_client_list_setup(&list, 299));
	ASSERT_TRUE(list.capacity == 299);
	kn_client_list_cleanup(&list);

	return TN_SUCCESS;
}

TN_TEST_CASE_BEGIN(client_list_fullandempty)
	kn_client_list_t list;
	uint64_t capacity = 123;
	kn_client_t client = {
		.state = KN_CLIENT_OPEN,
	};
	kn_client_t *out_client;

	ASSERT_SUCCESS(kn_client_list_setup(&list, capacity));
	ASSERT_TRUE(list.capacity == capacity);

	ASSERT_FAILS(kn_client_list_close(&list, &client));

	for (uint64_t i = 0; i < capacity; i++) {
		ASSERT_SUCCESS(kn_client_list_open(&list, &out_client));
	}

	ASSERT_FAILS(kn_client_list_open(&list, &out_client));

	client.state = KN_CLIENT_CLOSED;
	ASSERT_SUCCESS(kn_client_list_close(&list, &client));

	ASSERT_SUCCESS(kn_client_list_open(&list, &out_client));
	ASSERT_TRUE(out_client->state == KN_CLIENT_CLOSED);

	for (uint64_t i = 0; i < capacity; i++) {
		client.state = KN_CLIENT_OPEN;
		ASSERT_SUCCESS(kn_client_list_close(&list, &client));
	}

	client.state = KN_CLIENT_OPEN;
	ASSERT_FAILS(kn_client_list_close(&list, &client));

	kn_client_list_cleanup(&list);

	return TN_SUCCESS;
}

TN_TEST_CASE_BEGIN(client_list_ops)


	return TN_SUCCESS;
}

TN_TEST_CASE(test_client_list_create, client_list_create);
TN_TEST_CASE(test_client_list_fullandempty, client_list_fullandempty);
TN_TEST_CASE(test_client_list_ops, client_list_ops);
