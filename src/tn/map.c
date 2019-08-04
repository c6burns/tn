#include "tn/map.h"

#include "aws/common/hash_table.h"

#include "tn/allocator.h"
#include "tn/error.h"

// private ------------------------------------------------------------------------------------------------------
uint64_t tn_map_key_hash_fn(const void *key)
{
    return (uint64_t)key;
}

// private ------------------------------------------------------------------------------------------------------
bool tn_map_key_cmp_fn(const void *a, const void *b)
{
    return (a == b);
}

// --------------------------------------------------------------------------------------------------------------
int tn_map_setup(tn_map_t *map, uint64_t capacity)
{
    TN_ASSERT(map);
    memset(map, 0, sizeof(*map));

    map->capacity = capacity;

    struct aws_hash_table *priv = TN_MEM_ACQUIRE(sizeof(*priv));
    TN_GUARD_NULL(priv);

    TN_GUARD_NULL_CLEANUP(aws_hash_table_init(priv, &tn_aws_default_allocator, capacity, tn_map_key_hash_fn, tn_map_key_cmp_fn, NULL, NULL));

    map->priv = priv;
cleanup:
    TN_MEM_RELEASE(priv);
    return TN_ERROR;
}

// --------------------------------------------------------------------------------------------------------------
void tn_map_cleanup(tn_map_t *map)
{
    TN_ASSERT(map);
    aws_hash_table_clean_up((struct aws_hash_table *)map->priv);
    TN_MEM_RELEASE(map->priv);
}

// --------------------------------------------------------------------------------------------------------------
int tn_map_get(tn_map_t *map, void *key, void **out_value)
{
    TN_ASSERT(map);
    TN_ASSERT(map->priv);
    TN_ASSERT(out_value);

    *out_value = NULL;
    struct aws_hash_element *elem = NULL;
    TN_GUARD(aws_hash_table_find((struct aws_hash_table *)map->priv, key, &elem));

    TN_GUARD_NULL(elem);
    *out_value = elem->value;

    return TN_SUCCESS;
}

// --------------------------------------------------------------------------------------------------------------
int tn_map_set(tn_map_t *map, void *key, void *value)
{
    TN_ASSERT(map);
    TN_ASSERT(map->priv);
    TN_ASSERT(value);

    int created = 0;
    TN_GUARD(aws_hash_table_put((struct aws_hash_table *)map->priv, key, value, &created));
    TN_GUARD(created == 0);

    return TN_SUCCESS;
}

// --------------------------------------------------------------------------------------------------------------
int tn_map_remove(tn_map_t *map, void *key)
{
    TN_ASSERT(map);
    TN_ASSERT(map->priv);

    int removed = 0;
    TN_GUARD(aws_hash_table_remove((struct aws_hash_table *)map->priv, key, NULL, &removed));
    TN_GUARD(removed == 0);

    return TN_SUCCESS;
}

// --------------------------------------------------------------------------------------------------------------
void tn_map_clear(tn_map_t *map)
{
    TN_ASSERT(map);
    TN_ASSERT(map->priv);
    aws_hash_table_clear((struct aws_hash_table *)map->priv);
}
