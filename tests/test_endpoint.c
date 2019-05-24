#include "uv.h"

#include "tn/test_harness.h"
#include "tn/endpoint.h"
#include "tn/error.h"
#include "tn/log.h"

TN_TEST_CASE_BEGIN(test_endpoint)
	uint16_t port;
	char buf[255];
	tn_endpoint_t endpoint;
	struct sockaddr_in6 sa6;
	struct sockaddr_in sa;

	// test that our size if correct (since we pass this to managed)
	ASSERT_INT_EQUALS(sizeof(endpoint), TN_ENDPOINT_MAX_SIZE);

	// test ipv4 conversion
	ASSERT_SUCCESS(uv_ip4_addr("1.2.3.4", 5354, &sa));
	ASSERT_SUCCESS(tn_endpoint_convert_from(&endpoint, &sa));
	ASSERT_SUCCESS(tn_endpoint_get(&endpoint, &port, buf, 255));
	ASSERT_SUCCESS(strcmp(buf, "1.2.3.4"));
	ASSERT_TRUE(port == 5354);
	ASSERT_TRUE(tn_endpoint_is_ipv4(&endpoint));

	// test ipv6 conversion
	ASSERT_SUCCESS(uv_ip6_addr("1123::4567:890a:bcde", 8045, &sa6));
	ASSERT_SUCCESS(tn_endpoint_convert_from(&endpoint, &sa6));
	ASSERT_SUCCESS(tn_endpoint_get(&endpoint, &port, buf, 255));
	ASSERT_SUCCESS(strcmp(buf, "1123::4567:890a:bcde"));
	ASSERT_TRUE(port == 8045);
	ASSERT_TRUE(tn_endpoint_is_ipv6(&endpoint));
	
	// test set ipv4
	ASSERT_SUCCESS(tn_endpoint_set_ip4(&endpoint, "0.0.0.0", 65432));
	ASSERT_SUCCESS(tn_endpoint_get(&endpoint, &port, buf, 255));
	ASSERT_SUCCESS(strcmp(buf, "0.0.0.0"));
	ASSERT_TRUE(port == 65432);
	ASSERT_TRUE(tn_endpoint_is_ipv4(&endpoint));

	// test set ipv6
	ASSERT_SUCCESS(tn_endpoint_set_ip6(&endpoint, "fe80::2c92:d74a:43ba", 12345));
	ASSERT_SUCCESS(tn_endpoint_get(&endpoint, &port, buf, 255));
	ASSERT_SUCCESS(strcmp(buf, "fe80::2c92:d74a:43ba"));
	ASSERT_TRUE(port == 12345);
	ASSERT_TRUE(tn_endpoint_is_ipv6(&endpoint));

	return TN_SUCCESS;
}

TN_TEST_CASE(test_endpoints, test_endpoint)
