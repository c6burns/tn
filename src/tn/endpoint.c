#include "tn/endpoint.h"

#include "uv.h"

#include "tn/error.h"

#include "aws/common/byte_buf.h"
#include "aws/common/hash_table.h"

// --------------------------------------------------------------------------------------------------------------
int tn_endpoint_is_ipv4(const tn_endpoint_t *endpoint)
{
	TN_ASSERT(endpoint);
	return (endpoint->type == TN_ENDPOINT_TYPE_IPV4);
}

// --------------------------------------------------------------------------------------------------------------
int tn_endpoint_is_ipv6(const tn_endpoint_t *endpoint)
{
	TN_ASSERT(endpoint);
	return (endpoint->type == TN_ENDPOINT_TYPE_IPV6);
}

// --------------------------------------------------------------------------------------------------------------
int tn_endpoint_get(tn_endpoint_t *endpoint, uint16_t *port, char *buf, int buf_len)
{
	TN_ASSERT(endpoint);
	TN_ASSERT(port);
	TN_ASSERT(buf);

	*port = 0;
	memset(buf, 0, buf_len);

	char ipbuf[255];
	memset(ipbuf, 0, 255);

	const struct sockaddr_storage *sockaddr = (struct sockaddr_storage *)&endpoint->sockaddr;
	if (sockaddr->ss_family == AF_INET6) {
		struct sockaddr_in6 *sa = (struct sockaddr_in6 *)sockaddr;
		uv_ip6_name(sa, ipbuf, 255);
		*port = ntohs(sa->sin6_port);
	} else if (sockaddr->ss_family == AF_INET) {
		struct sockaddr_in *sa = (struct sockaddr_in *)sockaddr;
		uv_ip4_name(sa, ipbuf, 255);
		*port = ntohs(sa->sin_port);
	} else {
		return TN_ERROR;
	}

	sprintf(buf, "%s", ipbuf);
	return TN_SUCCESS;
}

// --------------------------------------------------------------------------------------------------------------
int tn_endpoint_set(tn_endpoint_t *endpoint, const char *ip, uint16_t port)
{
	TN_ASSERT(endpoint);
	TN_ASSERT(ip);
	if (tn_endpoint_set_ip6(endpoint, ip, port)) {
		return tn_endpoint_set_ip4(endpoint, ip, port);
	}

	return TN_SUCCESS;
}

// --------------------------------------------------------------------------------------------------------------
int tn_endpoint_set_ip4(tn_endpoint_t *endpoint, const char *ip, uint16_t port)
{
	TN_ASSERT(endpoint);
	TN_ASSERT(ip);

	int ret;

	if (!endpoint) return TN_ERROR;
	if (!ip) return TN_ERROR;

	memset(endpoint, 0, sizeof(*endpoint));
	if ((ret = uv_ip4_addr(ip, port, (struct sockaddr_in *)&endpoint->sockaddr))) return TN_ERROR;
	endpoint->type = TN_ENDPOINT_TYPE_IPV4;

	return TN_SUCCESS;
}

// --------------------------------------------------------------------------------------------------------------
int tn_endpoint_set_ip6(tn_endpoint_t *endpoint, const char *ip, uint16_t port)
{
	TN_ASSERT(endpoint);
	TN_ASSERT(ip);

	int ret;

	if (!endpoint) return TN_ERROR;
	if (!ip) return TN_ERROR;

	memset(endpoint, 0, sizeof(*endpoint));
	if ((ret = uv_ip6_addr(ip, port, (struct sockaddr_in6 *)&endpoint->sockaddr))) return TN_ERROR;
	endpoint->type = TN_ENDPOINT_TYPE_IPV6;

	return TN_SUCCESS;
}

// --------------------------------------------------------------------------------------------------------------
int tn_endpoint_convert_from(tn_endpoint_t *endpoint, void *sockaddr_void)
{
	if (!endpoint) return TN_ERROR;
	if (!sockaddr_void) return TN_ERROR;

	struct sockaddr_storage *sockaddr = sockaddr_void;

	memset(endpoint, 0, sizeof(*endpoint));
	if (sockaddr->ss_family == AF_INET6) {
		memcpy(&endpoint->sockaddr, (struct sockaddr_in6 *)sockaddr, sizeof(struct sockaddr_in6));
		endpoint->type = TN_ENDPOINT_TYPE_IPV6;
	} else if (sockaddr->ss_family == AF_INET) {
		memcpy(&endpoint->sockaddr, (struct sockaddr_in *)sockaddr, sizeof(struct sockaddr_in));
		endpoint->type = TN_ENDPOINT_TYPE_IPV4;
	} else {
		return TN_ERROR;
	}

	return TN_SUCCESS;
}

// --------------------------------------------------------------------------------------------------------------
int tn_endpoint_equal_addr(tn_endpoint_t *endpoint, void *sockaddr_storage)
{
	tn_sockaddr_storage_t *addr = sockaddr_storage;
	if (addr->family != endpoint->sockaddr.family) return 0;

	if (addr->family == AF_INET6) {
		struct sockaddr_in6 *ep_addr6 = (struct sockaddr_in6 *)&endpoint->sockaddr;
		struct sockaddr_in6 *addr6 = (struct sockaddr_in6 *)addr;
		if (memcmp(&endpoint->sockaddr, addr, sizeof(struct sockaddr_in6))) return 0;
		return 1;
	} else if (addr->family == AF_INET) {
		tn_sockaddr4_t *ep_addr4 = (tn_sockaddr4_t *)&endpoint->sockaddr;
		tn_sockaddr4_t *addr4 = (tn_sockaddr4_t *)addr;
		if (addr4->addr != ep_addr4->addr) return 0;
		if (addr4->port != ep_addr4->port) return 0;
		return 1;
	}

	return 0;
}

// --------------------------------------------------------------------------------------------------------------
int tn_endpoint_get_hash(tn_endpoint_t *endpoint, uint64_t *out_hash)
{
	TN_ASSERT(out_hash);

	*out_hash = 0;
	if (endpoint->sockaddr.family == AF_INET6) {
		struct aws_byte_cursor bc = aws_byte_cursor_from_array(&endpoint->sockaddr, sizeof(struct sockaddr_in6));
		*out_hash = aws_hash_byte_cursor_ptr(&bc);
		return TN_SUCCESS;
	} else if (endpoint->sockaddr.family == AF_INET) {
		tn_sockaddr4_t *addr = (tn_sockaddr4_t *)&endpoint->sockaddr;
		*out_hash = 0x0000ffffffffffffULL & ((addr->port << sizeof(uint32_t)) | addr->addr);
		return TN_SUCCESS;
	}

	return TN_ERROR;
}
