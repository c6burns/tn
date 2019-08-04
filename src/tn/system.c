#include "tn/system.h"

#include "uv.h"

#include "tn/allocator.h"
#include "tn/error.h"

typedef struct tn_system_priv_s {
    uv_cpu_info_t *cpu_info;
    uint32_t cpu_count;
} tn_system_priv_t;

// --------------------------------------------------------------------------------------------------------------
int tn_system_setup(tn_system_t *system)
{
    TN_ASSERT(system);

    tn_system_priv_t *priv = NULL;
    TN_GUARD_NULL(priv = TN_MEM_ACQUIRE(sizeof(*priv)));

    memset(priv, 0, sizeof(*priv));
    system->priv = priv;

    int cpu_count = priv->cpu_count = 0;
    TN_GUARD(uv_cpu_info(&priv->cpu_info, &cpu_count));
    priv->cpu_count = (uint32_t)cpu_count;

    return TN_SUCCESS;
}

// --------------------------------------------------------------------------------------------------------------
void tn_system_cleanup(tn_system_t *system)
{
    TN_ASSERT(system);

    tn_system_priv_t *priv = system->priv;
    if (priv->cpu_info) uv_free_cpu_info(priv->cpu_info, priv->cpu_count);

    TN_MEM_RELEASE(system->priv);
}

// --------------------------------------------------------------------------------------------------------------
uint32_t tn_system_cpu_count(tn_system_t *system)
{
    TN_ASSERT(system && system->priv);

    return ((tn_system_priv_t *)system->priv)->cpu_count;

    return TN_SUCCESS;
}
