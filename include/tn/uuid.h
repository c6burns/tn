#ifndef TN_UUID_H
#define TN_UUID_H

#include <stdint.h>

typedef struct tn_uuid_s {
	uint8_t uuid_data[16];
} tn_uuid_t;

tn_uuid_t *tn_uuid_new(void);
void tn_uuid_delete(tn_uuid_t **ptr_uuid);
int tn_uuid_generate(tn_uuid_t *uuid);
int tn_uuid_clear(tn_uuid_t *uuid);
int tn_uuid_compare(tn_uuid_t *uuid1, tn_uuid_t *uuid2);
#define tn_uuid_cmp(uuid1, uuid2) tn_uuid_compare(uuid1, uuid2)

#endif