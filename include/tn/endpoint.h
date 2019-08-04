#ifndef TN_ENDPOINT_H
#define TN_ENDPOINT_H

#include "tn/error.h"

#include <stdbool.h>
#include <stdint.h>

#define TN_ENDPOINT_MAX_SIZE 28

typedef struct tn_sockaddr4_s {
    uint16_t family;
    uint16_t port;
    uint32_t addr;
} tn_sockaddr4_t;

typedef struct tn_sockaddr6_s {
    uint16_t family;
    uint16_t port;
    uint32_t flowinfo;
    uint8_t addr[16];
    uint32_t scope_id;
} tn_sockaddr6_t;

typedef union tn_sockaddr_u {
    tn_sockaddr4_t addr4;
    tn_sockaddr6_t addr6;
} tn_sockaddr_t;

typedef tn_sockaddr_t tn_endpoint_t;

bool tn_endpoint_is_ipv4(const tn_endpoint_t *endpoint);
bool tn_endpoint_is_ipv6(const tn_endpoint_t *endpoint);
uint16_t tn_endpoint_af_get(const tn_endpoint_t *endpoint);
void tn_endpoint_af_set(tn_endpoint_t *endpoint, uint16_t af);
uint16_t tn_endpoint_port_get(tn_endpoint_t *endpoint);
void tn_endpoint_from_short(tn_endpoint_t *endpoint, uint16_t port, uint16_t s0, uint16_t s1, uint16_t s2, uint16_t s3, uint16_t s4, uint16_t s5, uint16_t s6, uint16_t s7);
void tn_endpoint_from_byte(tn_endpoint_t *endpoint, uint16_t port, uint8_t b0, uint8_t b1, uint8_t b2, uint8_t b3);
void tn_endpoint_port_set(tn_endpoint_t *endpoint, uint16_t port);
int tn_endpoint_string_get(tn_endpoint_t *endpoint, uint16_t *port, char *buf, int buf_len);
int tn_endpoint_from_string(tn_endpoint_t *endpoint, const char *ip, uint16_t port);
int tn_endpoint_set_ip4(tn_endpoint_t *endpoint, const char *ip, uint16_t port);
int tn_endpoint_set_ip6(tn_endpoint_t *endpoint, const char *ip, uint16_t port);
int tn_endpoint_convert_from(tn_endpoint_t *endpoint, void *sockaddr);
bool tn_endpoint_equal_addr(tn_endpoint_t *endpoint, void *sockaddr);
int tn_endpoint_get_hash(tn_endpoint_t *endpoint, uint64_t *out_hash);

#endif
