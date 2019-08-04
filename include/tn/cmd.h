#ifndef TN_CMD_H
#define TN_CMD_H

#include <stdint.h>

#include "tn/buffer.h"
#include "tn/endpoint.h"
#include "tn/queue_spsc.h"

#define TN_CMD_MAX_SIZE 64
#define TN_CMD_PAD_SIZE (TN_CMD_MAX_SIZE) - sizeof(uint32_t) - sizeof(uint32_t)

#define TN_CMD_FIELDS \
    uint32_t id;      \
    uint32_t type;

typedef enum tn_cmd_type_e {
    TN_CMD_NONE,
    TN_CMD_CLIENT_OPEN,
    TN_CMD_CLIENT_CLOSE,
    TN_CMD_CLIENT_SEND,
} tn_cmd_type_t;

// base cmd
typedef struct tn_cmd_base_s {
    TN_CMD_FIELDS
    uint8_t pad[TN_CMD_PAD_SIZE];
} tn_cmd_base_t;

// create client connection
typedef struct tn_cmd_client_open_s {
    TN_CMD_FIELDS
    void *priv;
    tn_endpoint_t endpoint;
} tn_cmd_client_open_t;

// disconnect client
typedef struct tn_cmd_client_close_s {
    TN_CMD_FIELDS
    uint64_t client_id;
} tn_cmd_client_close_t;

// send bytes
typedef struct tn_cmd_client_send_s {
    TN_CMD_FIELDS
    uint64_t client_id;
    uint8_t *buffer;
    size_t len;
    void *priv;
    uint8_t channel;
    uint8_t flags;
} tn_cmd_client_send_t;

typedef struct tn_cmd_list_s {
    tn_cmd_base_t *tn_cmds;
    tn_queue_spsc_t tn_cmds_free;
    tn_queue_spsc_t tn_cmds_ready;
    uint64_t capacity;
} tn_cmd_list_t;

TN_STATIC_ASSERT(sizeof(tn_cmd_base_t) == TN_CMD_MAX_SIZE);
TN_STATIC_ASSERT(sizeof(tn_cmd_client_open_t) <= TN_CMD_MAX_SIZE);
TN_STATIC_ASSERT(sizeof(tn_cmd_client_close_t) <= TN_CMD_MAX_SIZE);
TN_STATIC_ASSERT(sizeof(tn_cmd_client_send_t) <= TN_CMD_MAX_SIZE);

int tn_cmd_list_setup(tn_cmd_list_t *list, uint64_t capacity);
void tn_cmd_list_cleanup(tn_cmd_list_t *list);
int tn_cmd_list_reset(tn_cmd_list_t *list);

int tn_cmd_list_free_peek(tn_cmd_list_t *list, tn_cmd_base_t **out_evt);
int tn_cmd_list_free_pop(tn_cmd_list_t *list);
void tn_cmd_list_free_pop_cached(tn_cmd_list_t *list);
int tn_cmd_list_free_pop_back(tn_cmd_list_t *list, tn_cmd_base_t **out_evt);
int tn_cmd_list_free_pop_open(tn_cmd_list_t *list, tn_cmd_client_open_t **out_evt);
int tn_cmd_list_free_pop_close(tn_cmd_list_t *list, tn_cmd_client_close_t **out_evt);
int tn_cmd_list_free_pop_send(tn_cmd_list_t *list, tn_cmd_client_send_t **out_evt);
int tn_cmd_list_free_push(tn_cmd_list_t *list, void *evt);

int tn_cmd_list_ready_peek(tn_cmd_list_t *list, tn_cmd_base_t **out_evt);
int tn_cmd_list_ready_pop(tn_cmd_list_t *list);
void tn_cmd_list_ready_pop_cached(tn_cmd_list_t *list);
int tn_cmd_list_ready_pop_back(tn_cmd_list_t *list, tn_cmd_base_t **out_evt);
int tn_cmd_list_ready_pop_all(tn_cmd_list_t *list, tn_cmd_base_t **out_evt, uint64_t *out_count);
int tn_cmd_list_ready_push(tn_cmd_list_t *list, void *out_evt);

#endif
