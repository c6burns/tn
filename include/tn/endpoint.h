#ifndef TN_ENDPOINT_H
#define TN_ENDPOINT_H

#include <stdint.h>
#include <stdbool.h>

#define TN_ENDPOINT_MAX_SIZE 28
#define TN_ENDPOINT_SOCKADDR_STORAGE_SIZE 128

typedef struct tn_sockaddr_storage_s {
	uint16_t family;
	uint8_t pad0[TN_ENDPOINT_SOCKADDR_STORAGE_SIZE - sizeof(uint16_t)];
} tn_sockaddr_storage_t;

typedef struct tn_sockaddr4_s {
	uint16_t family;
	uint16_t port;
	uint32_t addr;
} tn_sockaddr4_t;

typedef struct tn_sockaddr6_s {
	uint16_t family;
	uint16_t port;
	uint32_t  flowinfo;
	uint8_t addr[16];
	uint32_t scope_id;
} tn_sockaddr6_t;

typedef union tn_sockaddr_u {
	tn_sockaddr4_t addr4;
	tn_sockaddr6_t addr6;
} tn_sockaddr_t;

typedef enum tn_endpoint_type_e {
	TN_ENDPOINT_TYPE_NONE,
	TN_ENDPOINT_TYPE_IPV4,
	TN_ENDPOINT_TYPE_IPV6,
	TN_ENDPOINT_TYPE_PIPE,
	TN_ENDPOINT_TYPE_FILE,
} tn_endpoint_type_t;

typedef tn_sockaddr_t tn_endpoint_t;

bool tn_endpoint_is_ipv4(const tn_endpoint_t *endpoint);
bool tn_endpoint_is_ipv6(const tn_endpoint_t *endpoint);
int tn_endpoint_get(tn_endpoint_t *endpoint, uint16_t *port, char *buf, int buf_len);
int tn_endpoint_set(tn_endpoint_t *endpoint, const char *ip, uint16_t port);
int tn_endpoint_set_ip4(tn_endpoint_t *endpoint, const char *ip, uint16_t port);
int tn_endpoint_set_ip6(tn_endpoint_t *endpoint, const char *ip, uint16_t port);
int tn_endpoint_convert_from(tn_endpoint_t *endpoint, void *sockaddr);
bool tn_endpoint_is_equal(tn_endpoint_t *endpoint, void *sockaddr_storage);
int tn_endpoint_get_hash(tn_endpoint_t *endpoint, uint64_t *out_hash);

#endif