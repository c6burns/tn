#ifndef TN_ENDPOINT_H
#define TN_ENDPOINT_H

#include <stdint.h>

#define TN_ENDPOINT_MAX_SIZE 256
#define TN_ENDPOINT_SOCKADDR_STORAGE_SIZE 128

typedef struct tn_sockaddr_storage_s {
	uint16_t family;
	uint8_t pad0[TN_ENDPOINT_SOCKADDR_STORAGE_SIZE - sizeof(uint16_t)];
} tn_sockaddr_storage_t;

typedef struct tn_sockaddr4_s {
	uint16_t family;
	uint16_t port;
	uint32_t addr;
	uint8_t pad0[TN_ENDPOINT_SOCKADDR_STORAGE_SIZE - sizeof(uint16_t) - sizeof(uint16_t) - sizeof(uint32_t)];
} tn_sockaddr4_t;

typedef struct tn_sockaddr6_s {
	uint16_t family;
	uint16_t port;
	uint32_t  flowinfo;
	uint8_t addr[16];
	uint32_t scope;
	uint8_t pad0[TN_ENDPOINT_SOCKADDR_STORAGE_SIZE - sizeof(uint16_t) - sizeof(uint16_t) - sizeof(uint32_t) - sizeof(uint32_t) - (sizeof(uint8_t) * 16)];
} tn_sockaddr6_t;


typedef enum tn_endpoint_type_e {
	TN_ENDPOINT_TYPE_NONE,
	TN_ENDPOINT_TYPE_IPV4,
	TN_ENDPOINT_TYPE_IPV6,
	TN_ENDPOINT_TYPE_PIPE,
	TN_ENDPOINT_TYPE_FILE,
} tn_endpoint_type_t;

typedef struct tn_endpoint_s {
	uint64_t type; // 64 bit (vs anything lower) won't mess up alignment of structs coming after (eg. sockaddr)
	tn_sockaddr_storage_t sockaddr;
	uint8_t padding[TN_ENDPOINT_MAX_SIZE - sizeof(uint64_t) - sizeof(tn_sockaddr_storage_t)];
} tn_endpoint_t;

typedef struct tn_endpoint_ipv4_s {
	uint64_t type;
	tn_sockaddr4_t sockaddr;
} tn_endpoint_ip4_t;

typedef struct tn_endpoint_ipv6_s {
	uint64_t type;
	tn_sockaddr6_t sockaddr;
} tn_endpoint_ip6_t;

typedef struct tn_endpoint_file_s {
	uint64_t type;
	char file_name[TN_ENDPOINT_MAX_SIZE - sizeof(uint64_t)];
} tn_endpoint_file_t;

int tn_endpoint_is_ipv4(const tn_endpoint_t *endpoint);
int tn_endpoint_is_ipv6(const tn_endpoint_t *endpoint);
int tn_endpoint_get(tn_endpoint_t *endpoint, uint16_t *port, char *buf, int buf_len);
int tn_endpoint_set(tn_endpoint_t *endpoint, const char *ip, uint16_t port);
int tn_endpoint_set_ip4(tn_endpoint_t *endpoint, const char *ip, uint16_t port);
int tn_endpoint_set_ip6(tn_endpoint_t *endpoint, const char *ip, uint16_t port);
int tn_endpoint_convert_from(tn_endpoint_t *endpoint, void *sockaddr);
int tn_endpoint_is_equal(tn_endpoint_t *endpoint, void *sockaddr_storage);
int tn_endpoint_get_hash(tn_endpoint_t *endpoint, uint64_t *out_hash);

#endif