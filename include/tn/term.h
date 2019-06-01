#ifndef TN_TERM_H
#define TN_TERM_H

#include "tn/error.h"
#include "tn/allocator.h"
#include "tn/atomic.h"
#include "tn/thread.h"
#include "tn/mutex.h"
#include "tn/queue_spsc.h"


#define TN_TERM_MAX_LINE 1024
#define TN_TERM_MAX_CMD 16
#define TN_TERM_INIT { .priv = NULL, .state = TN_ATOMIC_INIT(TN_TERM_STATE_NEW), }

typedef enum tn_term_state_e {
	TN_TERM_STATE_NEW,
    TN_TERM_STATE_STARTING,
	TN_TERM_STATE_STARTED,
	TN_TERM_STATE_STOPPING,
	TN_TERM_STATE_STOPPED,
    TN_TERM_STATE_ERROR,
    TN_TERM_STATE_INVALID,
} tn_term_state_t;

struct tn_term_pos {
	int x;
	int y;
};

struct tn_term_csi {
	char seq_end;
	int seq_len;
	int param_count;
	int param[8];
	int open_count;
};

struct tn_term_buf {
	char buf[TN_TERM_MAX_LINE];
	int len;
};

typedef void(*tn_term_callback_char_func)(char in_char);
typedef void(*tn_term_callback_cmd_func)(const char *in_cmd);

typedef struct tn_term_s {
	void *priv;
	tn_queue_spsc_t queue_log;
	tn_queue_spsc_t queue_cmd;
	tn_term_callback_char_func cb_char;
	tn_term_callback_cmd_func cb_cmd;
	tn_thread_t thread_io;
	tn_mutex_t mtx;
	tn_atomic_t state;
	struct tn_term_pos size;
	struct tn_term_pos pos_last;
} tn_term_t;


int tn_term_setup(tn_term_t *term);
void tn_term_cleanup(tn_term_t *term);

int tn_term_start(tn_term_t *term, tn_term_callback_char_func cb_char, tn_term_callback_cmd_func cb_cmd);
int tn_term_stop(tn_term_t *term);

tn_term_state_t tn_term_state(tn_term_t *term);
int tn_term_callback_cmd(tn_term_t *term, tn_term_callback_cmd_func cb_cmd);
int tn_term_callback_char(tn_term_t *term, tn_term_callback_char_func cb_char);

int tn_term_clear(tn_term_t *term);
int tn_term_clear_down(tn_term_t *term);
int tn_term_clear_up(tn_term_t *term);
int tn_term_clear_line(tn_term_t *term);
int tn_term_clear_line_home(tn_term_t *term);
int tn_term_clear_line_end(tn_term_t *term);

int tn_term_pos_store(tn_term_t *term);
int tn_term_pos_restore(tn_term_t *term);
int tn_term_pos_up(tn_term_t *term, int count);
int tn_term_pos_down(tn_term_t *term, int count);
int tn_term_pos_right(tn_term_t *term, int count);
int tn_term_pos_left(tn_term_t *term, int count);

int tn_term_write(tn_term_t *term, const char *fmt, ...);

#endif