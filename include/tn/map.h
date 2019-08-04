#ifndef TN_MAP_H
#define TN_MAP_H

#include <stdint.h>

typedef struct tn_map_s {
    void *priv;
    uint64_t capacity;
} tn_map_t;

int tn_map_setup(tn_map_t *map, uint64_t capacity);
void tn_map_cleanup(tn_map_t *map);
int tn_map_get(tn_map_t *map, void *key, void **out_value);
int tn_map_set(tn_map_t *map, void *key, void *value);
int tn_map_remove(tn_map_t *map, void *key);
void tn_map_clear(tn_map_t *map);

#endif
