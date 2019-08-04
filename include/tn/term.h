#ifndef TN_TERM_H
#define TN_TERM_H

#ifdef _WIN32
#    include <io.h>
#endif

#include "tn/allocator.h"
#include "tn/atomic.h"
#include "tn/error.h"
#include "tn/list_ptr.h"
#include "tn/mutex.h"
#include "tn/queue_spsc.h"
#include "tn/thread.h"

#define TN_TERM_MAX_LINE 1024
#define TN_TERM_BUFFER_STACK_SIZE 1024
#define TN_TERM_INIT                                              \
    {                                                             \
        .priv = NULL, .state = TN_ATOMIC_INIT(TN_TERM_STATE_NEW), \
    }

enum tn_term_key {
    TN_TERM_KEY_NONE,
    TN_TERM_KEY_F1,
    TN_TERM_KEY_F2,
    TN_TERM_KEY_F3,
    TN_TERM_KEY_F4,
    TN_TERM_KEY_F5,
    TN_TERM_KEY_F6,
    TN_TERM_KEY_F7,
    TN_TERM_KEY_F8,
    TN_TERM_KEY_F9,
    TN_TERM_KEY_F10,
    TN_TERM_KEY_F11,
    TN_TERM_KEY_F12,
    TN_TERM_KEY_HOME,
    TN_TERM_KEY_END,
    TN_TERM_KEY_INSERT,
    TN_TERM_KEY_DELETE,
    TN_TERM_KEY_BACKSPACE,
    TN_TERM_KEY_UP,
    TN_TERM_KEY_DOWN,
    TN_TERM_KEY_RIGHT,
    TN_TERM_KEY_LEFT,
    TN_TERM_KEY_TAB,
    TN_TERM_KEY_ESC,
    TN_TERM_KEY_BREAK,
    TN_TERM_KEY_ENTER,
    TN_TERM_KEY_INVALID,
};

enum tn_term_color {
    TN_TERM_COLOR_BLACK,
    TN_TERM_COLOR_RED_DARK,
    TN_TERM_COLOR_GREEN_DARK,
    TN_TERM_COLOR_YELLOW_DARK,
    TN_TERM_COLOR_BLUE_DARK,
    TN_TERM_COLOR_PURPLE_DARK,
    TN_TERM_COLOR_AQUA_DARK,
    TN_TERM_COLOR_GREY_BRIGHT,
    TN_TERM_COLOR_GREY_DARK,
    TN_TERM_COLOR_RED_BRIGHT,
    TN_TERM_COLOR_GREEN_BRIGHT,
    TN_TERM_COLOR_YELLOW_BRIGHT,
    TN_TERM_COLOR_BLUE_BRIGHT,
    TN_TERM_COLOR_PURPLE_BRIGHT,
    TN_TERM_COLOR_AQUA_BRIGHT,
    TN_TERM_COLOR_WHITE,
};

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
    char shift_char;
};

struct tn_term_buf {
    char buf[TN_TERM_MAX_LINE];
    uint32_t len;
    uint64_t batch_id;
};

typedef void (*tn_term_callback_char_func)(char in_char);
typedef void (*tn_term_callback_key_func)(enum tn_term_key key);
typedef void (*tn_term_callback_resize_func)(uint16_t x, uint16_t y);

typedef struct tn_term_s {
    void *priv;
    tn_queue_spsc_t buffer_queue;
    tn_list_ptr_t buffer_stack;
    struct tn_term_buf *buffer_pool;
    tn_term_callback_char_func cb_char;
    tn_term_callback_key_func cb_key;
    tn_term_callback_resize_func cb_resize;
    tn_thread_t thread_io;
    tn_mutex_t mtx;
    tn_atomic_t state;
    tn_atomic_t batch_id;
    uint64_t batch_request_cache;
    uint64_t batch_process_cache;
    struct tn_term_pos size;
    struct tn_term_pos pos_last;
    int debug_print;
} tn_term_t;

int tn_term_setup(tn_term_t *term);
void tn_term_cleanup(tn_term_t *term);

int tn_term_start(tn_term_t *term);
int tn_term_stop(tn_term_t *term);

void tn_term_debug_print(tn_term_t *term, bool enabled);
tn_term_state_t tn_term_state(tn_term_t *term);
void tn_term_callback_char(tn_term_t *term, tn_term_callback_char_func cb_char);
void tn_term_callback_key(tn_term_t *term, tn_term_callback_key_func cb_char);
void tn_term_callback_resize(tn_term_t *term, tn_term_callback_resize_func cb_cmd);

int tn_term_clear(tn_term_t *term);
int tn_term_clear_down(tn_term_t *term);
int tn_term_clear_up(tn_term_t *term);
int tn_term_clear_line(tn_term_t *term);
int tn_term_clear_line_home(tn_term_t *term);
int tn_term_clear_line_end(tn_term_t *term);

int tn_term_pos_get(tn_term_t *term);
int tn_term_pos_set(tn_term_t *term, uint16_t x, uint16_t y);
int tn_term_pos_store(tn_term_t *term);
int tn_term_pos_restore(tn_term_t *term);
int tn_term_pos_up(tn_term_t *term, uint16_t count);
int tn_term_pos_down(tn_term_t *term, uint16_t count);
int tn_term_pos_right(tn_term_t *term, uint16_t count);
int tn_term_pos_left(tn_term_t *term, uint16_t count);

int tn_term_write(tn_term_t *term, const char *fmt, ...);
void tn_term_flush(tn_term_t *term);

void tn_term_flush(tn_term_t *term);
int tn_term_color_set(tn_term_t *term, uint8_t color);
int tn_term_bgcolor_set(tn_term_t *term, enum tn_term_color color);
uint8_t tn_term_color16(tn_term_t *term, enum tn_term_color color);
uint8_t tn_term_color256(tn_term_t *term, uint8_t r, uint8_t g, uint8_t b);
uint8_t tn_term_grey24(tn_term_t *term, uint8_t grey);

#endif
