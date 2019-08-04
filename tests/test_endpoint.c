#include "uv.h"

#include "tn/endpoint.h"
#include "tn/error.h"
#include "tn/log.h"
#include "tn/test_harness.h"

TN_TEST_CASE_BEGIN(test_endpoint)
    uint16_t port;
    char buf[255];
    tn_endpoint_t endpoint;
    struct sockaddr_in6 sa6;
    struct sockaddr_in sa;

    // test that our size if correct (since we pass this to managed)
    ASSERT_INT_EQUALS(sizeof(endpoint), TN_ENDPOINT_MAX_SIZE);

    // test ipv4 conversion
    memset(&endpoint, 0, sizeof(endpoint));
    ASSERT_FALSE(tn_endpoint_is_ipv4(&endpoint));
    ASSERT_FALSE(tn_endpoint_is_ipv6(&endpoint));
    ASSERT_SUCCESS(uv_ip4_addr("1.2.3.4", 5354, &sa));
    ASSERT_SUCCESS(tn_endpoint_convert_from(&endpoint, &sa));
    ASSERT_TRUE(tn_endpoint_is_ipv4(&endpoint));
    ASSERT_FALSE(tn_endpoint_is_ipv6(&endpoint));
    ASSERT_SUCCESS(tn_endpoint_string_get(&endpoint, &port, buf, sizeof(buf)));
    ASSERT_SUCCESS(strcmp(buf, "1.2.3.4"));
    ASSERT_TRUE(port == 5354);
    ASSERT_TRUE(tn_endpoint_is_ipv4(&endpoint));

    // test ipv6 conversion
    memset(&endpoint, 0, sizeof(endpoint));
    ASSERT_SUCCESS(uv_ip6_addr("1123::4567:890a:bcde", 8045, &sa6));
    ASSERT_SUCCESS(tn_endpoint_convert_from(&endpoint, &sa6));
    \tASSERT_FALSE(tn_endpoint_is_ipv4(&endpoint));
    ASSERT_TRUE(tn_endpoint_is_ipv6(&endpoint));
    ASSERT_SUCCESS(tn_endpoint_string_get(&endpoint, &port, buf, sizeof(buf)));
    ASSERT_SUCCESS(strcmp(buf, "1123::4567:890a:bcde"));
    ASSERT_TRUE(port == 8045);
    ASSERT_TRUE(tn_endpoint_is_ipv6(&endpoint));

    // test set ipv4
    memset(&endpoint, 0, sizeof(endpoint));
    ASSERT_SUCCESS(tn_endpoint_set_ip4(&endpoint, "0.0.0.0", 65432));
    ASSERT_TRUE(tn_endpoint_is_ipv4(&endpoint));
    ASSERT_FALSE(tn_endpoint_is_ipv6(&endpoint));
    ASSERT_SUCCESS(tn_endpoint_string_get(&endpoint, &port, buf, sizeof(buf)));
    ASSERT_SUCCESS(strcmp(buf, "0.0.0.0"));
    ASSERT_TRUE(port == 65432);
    ASSERT_TRUE(tn_endpoint_is_ipv4(&endpoint));

    // test set ipv6
    memset(&endpoint, 0, sizeof(endpoint));
    ASSERT_SUCCESS(tn_endpoint_set_ip6(&endpoint, "fe80::2c92:d74a:43ba", 12345));
    ASSERT_FALSE(tn_endpoint_is_ipv4(&endpoint));
    ASSERT_TRUE(tn_endpoint_is_ipv6(&endpoint));
    ASSERT_SUCCESS(tn_endpoint_string_get(&endpoint, &port, buf, sizeof(buf)));
    ASSERT_SUCCESS(strcmp(buf, "fe80::2c92:d74a:43ba"));
    ASSERT_TRUE(port == 12345);
    ASSERT_TRUE(tn_endpoint_is_ipv6(&endpoint));

    // test set ipv4 from byte
    memset(&endpoint, 0, sizeof(endpoint));
    tn_endpoint_from_byte(&endpoint, 65432, 1, 2, 3, 4);
    ASSERT_TRUE(tn_endpoint_is_ipv4(&endpoint));
    ASSERT_FALSE(tn_endpoint_is_ipv6(&endpoint));
    ASSERT_SUCCESS(tn_endpoint_string_get(&endpoint, &port, buf, sizeof(buf)));
    ASSERT_SUCCESS(strcmp(buf, "1.2.3.4"));
    ASSERT_TRUE(port == 65432);
    ASSERT_TRUE(tn_endpoint_is_ipv4(&endpoint));

    // test set ipv6
    memset(&endpoint, 0, sizeof(endpoint));
    tn_endpoint_from_short(&endpoint, 12345, 0xfe80, 0x0000, 0x0000, 0x0000, 0x0000, 0x2c92, 0xd74a, 0x43ba);
    ASSERT_FALSE(tn_endpoint_is_ipv4(&endpoint));
    ASSERT_TRUE(tn_endpoint_is_ipv6(&endpoint));
    ASSERT_SUCCESS(tn_endpoint_string_get(&endpoint, &port, buf, sizeof(buf)));
    ASSERT_SUCCESS(strcmp(buf, "fe80::2c92:d74a:43ba"));
    ASSERT_TRUE(port == 12345);
    ASSERT_TRUE(tn_endpoint_is_ipv6(&endpoint));

    // test af set/get
    memset(&endpoint, 0, sizeof(endpoint));
    tn_endpoint_af_set(&endpoint, AF_INET);
    ASSERT_TRUE(tn_endpoint_is_ipv4(&endpoint));
    ASSERT_FALSE(tn_endpoint_is_ipv6(&endpoint));
    ASSERT_INT_EQUALS(tn_endpoint_af_get(&endpoint), AF_INET);
    tn_endpoint_af_set(&endpoint, AF_INET6);
    ASSERT_FALSE(tn_endpoint_is_ipv4(&endpoint));
    ASSERT_TRUE(tn_endpoint_is_ipv6(&endpoint));
    ASSERT_INT_EQUALS(tn_endpoint_af_get(&endpoint), AF_INET6);

    // test port set/get
    memset(&endpoint, 0, sizeof(endpoint));
    tn_endpoint_port_set(&endpoint, 7463);
    ASSERT_INT_EQUALS(tn_endpoint_port_get(&endpoint), 7463);

    return TN_SUCCESS;
}

TN_TEST_CASE(test_endpoints, test_endpoint)
