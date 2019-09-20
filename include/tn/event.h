#ifndef TN_EVENT_H
#define TN_EVENT_H

#include <stdint.h>

#include "tn/buffer.h"
#include "tn/endpoint.h"
#include "tn/queue_spsc.h"

#define TN_EVENT_MAX_SIZE 64
#define TN_EVENT_PAD_SIZE (TN_EVENT_MAX_SIZE) - sizeof(uint32_t) - sizeof(uint32_t) - sizeof(uint64_t)

#define TN_EVENT_FIELDS \
    uint32_t id;        \
    uint32_t type;      \
    uint64_t cmd_id;

typedef enum tn_event_type_e {
    TN_EVENT_NONE,
    TN_EVENT_START,
    TN_EVENT_STOP,
    TN_EVENT_IOERROR,
    TN_EVENT_CLIENT_OPEN,
    TN_EVENT_CLIENT_CLOSE,
    TN_EVENT_CLIENT_READ,
    TN_EVENT_STATS,
} tn_event_type_t;

/* base event */
typedef struct tn_event_base_s {
    TN_EVENT_FIELDS
    uint8_t pad[TN_EVENT_PAD_SIZE];
} tn_event_base_t;

// service started
typedef struct tn_event_service_start_s {
    TN_EVENT_FIELDS
    void *service;
} tn_event_service_start_t;

// service stopped
typedef struct tn_event_service_stop_s {
    TN_EVENT_FIELDS
    void *service;
} tn_event_service_stop_t;

// error event
typedef struct tn_event_error_s {
    TN_EVENT_FIELDS
    uint64_t client_id;
    int32_t error_code;
} tn_event_error_t;

// client connected
typedef struct tn_event_client_open_s {
    TN_EVENT_FIELDS
    uint64_t client_id;
    tn_endpoint_t host;
    uint8_t client_type;
} tn_event_client_open_t;

// client disconnected
typedef struct tn_event_client_close_s {
    TN_EVENT_FIELDS
    uint64_t client_id;
    int32_t error_code;
} tn_event_client_close_t;

// client recv bytes
typedef struct tn_event_client_read_s {
    TN_EVENT_FIELDS
    uint64_t client_id;
    uint8_t *buffer;
    uint32_t len;
    uint32_t channel;
    void *priv;
} tn_event_client_read_t;

// IO stats
typedef struct tn_event_stats_s {
    TN_EVENT_FIELDS
    uint32_t clients_total;
    uint32_t recv_msgs;
    uint32_t recv_bytes;
    uint32_t send_msgs;
    uint32_t send_bytes;
    uint32_t events_total;
    uint32_t events_inuse;
    uint32_t events_free;
    uint32_t buffers_total;
    uint32_t buffers_inuse;
    uint32_t buffers_free;
} tn_event_stats_t;

typedef struct tn_event_list_s {
    tn_event_base_t *tn_events;
    tn_queue_spsc_t tn_events_free;
    tn_queue_spsc_t tn_events_ready;
    uint64_t capacity;
} tn_event_list_t;

TN_STATIC_ASSERT(sizeof(tn_event_base_t) == TN_EVENT_MAX_SIZE);
TN_STATIC_ASSERT(sizeof(tn_event_service_start_t) <= TN_EVENT_MAX_SIZE);
TN_STATIC_ASSERT(sizeof(tn_event_service_stop_t) <= TN_EVENT_MAX_SIZE);
TN_STATIC_ASSERT(sizeof(tn_event_error_t) <= TN_EVENT_MAX_SIZE);
TN_STATIC_ASSERT(sizeof(tn_event_client_open_t) <= TN_EVENT_MAX_SIZE);
TN_STATIC_ASSERT(sizeof(tn_event_client_close_t) <= TN_EVENT_MAX_SIZE);
TN_STATIC_ASSERT(sizeof(tn_event_client_read_t) <= TN_EVENT_MAX_SIZE);
TN_STATIC_ASSERT(sizeof(tn_event_stats_t) <= TN_EVENT_MAX_SIZE);

int tn_event_list_setup(tn_event_list_t *list, uint64_t capacity);
void tn_event_list_cleanup(tn_event_list_t *list);
int tn_event_list_reset(tn_event_list_t *list);

int tn_event_list_free_pop_back(tn_event_list_t *list, tn_event_base_t **out_evt);
int tn_event_list_free_pop_error(tn_event_list_t *list, tn_event_error_t **out_evt);
int tn_event_list_free_pop_open(tn_event_list_t *list, tn_event_client_open_t **out_evt);
int tn_event_list_free_pop_close(tn_event_list_t *list, tn_event_client_close_t **out_evt);
int tn_event_list_free_pop_read(tn_event_list_t *list, tn_event_client_read_t **out_evt);
int tn_event_list_ready_push(tn_event_list_t *list, void *out_evt);

int tn_event_list_ready_peek(tn_event_list_t *list, tn_event_base_t **out_evt);
int tn_event_list_ready_pop(tn_event_list_t *list);
void tn_event_list_ready_pop_cached(tn_event_list_t *list);
int tn_event_list_ready_pop_back(tn_event_list_t *list, tn_event_base_t **out_evt);
int tn_event_list_ready_pop_all(tn_event_list_t *list, tn_event_base_t **out_evt, uint64_t *out_count);
int tn_event_list_free_push(tn_event_list_t *list, void *evt);

#endif
