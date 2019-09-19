#ifndef TN_DSO_H
#define TN_DSO_H

typedef enum tn_dso_state_e {
    TN_DSO_STATE_UNLOADED,
    TN_DSO_STATE_LOADED,
    TN_DSO_STATE_INVALID,
} tn_dso_state_t;

typedef struct tn_dso_s {
    void *lib;
    tn_dso_state_t state;
} tn_dso_t;

tn_dso_state_t tn_dso_state(tn_dso_t *dso);
void tn_dso_state_set(tn_dso_t *dso, tn_dso_state_t state);
int tn_dso_setup(tn_dso_t *dso, const char *so_name);
int tn_dso_symbol(tn_dso_t *dso, const char *sym_name, void **out_fn_ptr);
int tn_dso_cleanup(tn_dso_t *dso);

#endif
