#ifndef TN_TERM_H
#define TN_TERM_H


typedef enum rc_term_state_e {
	TN_TERM_STATE_NEW,
    TN_TERM_STATE_STARTING,
	TN_TERM_STATE_STARTED,
	TN_TERM_STATE_STOPPING,
	TN_TERM_STATE_STOPPED,
    TN_TERM_STATE_ERROR,
    TN_TERM_STATE_INVALID,
} tn_term_state_t;

typedef struct tn_term_s {
    void *priv;
	tn_term_state_t state;
	int width, height;
} tn_term_t;


int tn_term_start(tn_term_t *term);

#endif
