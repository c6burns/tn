#include "tn/dso.h"
#include "tn/allocator.h"
#include "tn/error.h"
#include "uv.h"

// --------------------------------------------------------------------------------------------------------------
tn_dso_state_t tn_dso_state(tn_dso_t *dso)
{
    TN_ASSERT(dso);
    return dso->state;
}

// private ------------------------------------------------------------------------------------------------------
static inline void tn_dso_state_set(tn_dso_t *dso, tn_dso_state_t state)
{
    TN_ASSERT(dso);
    dso->state = state;
}

// private ------------------------------------------------------------------------------------------------------
static inline bool tn_dso_state_valid(tn_dso_state_t state)
{
    switch (state) {
    case TN_DSO_STATE_UNLOADED:
    case TN_DSO_STATE_LOADED:
        return true;
    }
    return false;
}

// --------------------------------------------------------------------------------------------------------------
int tn_dso_setup(tn_dso_t *dso, const char *so_name)
{
    TN_ASSERT(dso);
    TN_GUARD(tn_dso_state(dso) != TN_DSO_STATE_UNLOADED);
    TN_GUARD_NULL(dso->lib = TN_MEM_ACQUIRE(sizeof(uv_lib_t)));
    TN_GUARD(uv_dlopen(so_name, (uv_lib_t *)dso->lib));
    tn_dso_state_set(dso, TN_DSO_STATE_LOADED);
    return TN_SUCCESS;
}

// --------------------------------------------------------------------------------------------------------------
int tn_dso_symbol(tn_dso_t *dso, const char *sym_name, void **out_fn_ptr)
{
    TN_ASSERT(dso);
    TN_ASSERT(out_fn_ptr);
    TN_GUARD(tn_dso_state(dso) != TN_DSO_STATE_LOADED);
    TN_GUARD(uv_dlsym((uv_lib_t *)dso->lib, sym_name, out_fn_ptr));
    TN_GUARD_NULL(*out_fn_ptr);
    return TN_SUCCESS;
}

// --------------------------------------------------------------------------------------------------------------
int tn_dso_cleanup(tn_dso_t *dso)
{
    TN_ASSERT(dso);
    TN_GUARD(tn_dso_state(dso) != TN_DSO_STATE_LOADED);
    uv_dlclose((uv_lib_t *)dso->lib);
    tn_dso_state_set(dso, TN_DSO_STATE_UNLOADED);
    TN_MEM_RELEASE(dso->lib);
    return TN_SUCCESS;
}
