#include "tn/term.h"

#include <ctype.h>
#include <stdarg.h>

#include "uv.h"

#include "tn/buffer.h"

const char *const tn_term_key_string[] = {
    "",
    "[F1]",
    "[F2]",
    "[F3]",
    "[F4]",
    "[F5]",
    "[F6]",
    "[F7]",
    "[F8]",
    "[F9]",
    "[F10]",
    "[F11]",
    "[F12]",
    "[HOME]",
    "[END]",
    "[INS]",
    "[DEL]",
    "[BKSP]",
    "[UP]",
    "[DOWN]",
    "[RIGHT]",
    "[LEFT]",
    "[TAB]",
    "[ESC]",
};

typedef struct tn_term_priv_s {
    uv_loop_t uv_loop;
    uv_signal_t uv_signal;
    int ttyin_fd, ttyout_fd;
    uv_tty_t uv_tty_in, uv_tty_out;
    uv_timer_t uv_timer_tty;
    struct aws_byte_buf bbuf;
    struct aws_byte_cursor bpos;
} tn_term_priv_t;

void on_term_close_cb(uv_handle_t *handle);
void on_term_alloc_cb(uv_handle_t *handle, size_t size, uv_buf_t *buf);
void on_term_read_cb(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf);
void on_term_timer_cb(uv_timer_t *handle);
void on_term_signal_cb(uv_signal_t *handle, int signum);

// private ------------------------------------------------------------------------------------------------------
void tn_term_state_set(tn_term_t *term, tn_term_state_t state)
{
    TN_ASSERT(term);
    tn_atomic_store(&term->state, state);
}

// private ------------------------------------------------------------------------------------------------------
int tn_term_write_direct(tn_term_t *term, const char *fmt, ...)
{
    uv_buf_t buf;
    int len;
    char str[TN_TERM_MAX_LINE];
    tn_term_priv_t *priv = term->priv;

    va_list args;
    va_start(args, fmt);
    len = vsnprintf(str, TN_TERM_MAX_LINE, fmt, args);
    va_end(args);

    buf.base = str;
    buf.len = len;
    TN_GUARD(uv_try_write((uv_stream_t *)&priv->uv_tty_out, &buf, 1) <= 0);

    return TN_SUCCESS;
}

// private ------------------------------------------------------------------------------------------------------
int tn_term_buf_write_u8(tn_term_t *term, char in_char)
{
    TN_ASSERT(term && term->priv);

    int ret = TN_ERROR;
    tn_term_priv_t *priv = term->priv;

    TN_GUARD_CLEANUP(!aws_byte_buf_write_u8(&priv->bbuf, in_char));

    ret = TN_SUCCESS;

cleanup:
    return ret;
}

// private ------------------------------------------------------------------------------------------------------
int tn_term_key_print(tn_term_t *term, enum tn_term_key key)
{
    TN_GUARD(key <= TN_TERM_KEY_NONE);
    TN_GUARD(key >= TN_TERM_KEY_INVALID);
    return tn_term_write_direct(term, tn_term_key_string[key]);
}

// private ------------------------------------------------------------------------------------------------------
void tn_term_key_handle(tn_term_t *term, enum tn_term_key key)
{
    if (term->debug_print) tn_term_key_print(term, key);

    if (term->cb_key) term->cb_key(key);
    ;
}

// private ------------------------------------------------------------------------------------------------------
int tn_term_csi_print(tn_term_t *term, const char *buf, int len)
{
    TN_ASSERT(term && buf);

    char str[128];
    int istr = 0;

    str[istr++] = '[';
    for (int i = 0; i < len; i++) {
        const char c = buf[i];
        if (c == 27) {
            str[istr++] = '^';
            continue;
        }
        str[istr++] = c;
    }
    str[istr++] = ']';
    str[istr++] = '\0';

    return tn_term_write_direct(term, "%s", str);
}

// private ------------------------------------------------------------------------------------------------------
int tn_term_csi_parse(tn_term_t *term, const char *input, int maxlen, struct tn_term_csi *csi)
{
    TN_ASSERT(term && input && csi);

    char str[128];
    int istr = 0;
    int is_shift = 0;
    memset(csi, 0, sizeof(*csi));

    TN_GUARD(maxlen < 3);
    TN_GUARD(input[0] != '\033');

    for (int i = 1; i < maxlen; i++) {
        char c = input[i];

        if (isdigit(c)) {
            str[istr++] = c;
            continue;
        } else if (c == '[') {
            csi->open_count++;
            continue;
        } else if (c == 'O') {
            is_shift = 1;
            continue;
        }

        if (istr) {
            str[istr] = '\0';
            csi->param[csi->param_count++] = atoi(str);
            istr = 0;
        } else {
            csi->param[csi->param_count++] = 0;
        }

        if (is_shift) {
            csi->seq_end = 'O';
            csi->seq_len = i + 1;
            csi->shift_char = c;
            break;
        } else if (isalpha(c) || c == '~') {
            csi->seq_end = c;
            csi->seq_len = i + 1;
            break;
        }
    }

    TN_GUARD(!csi->seq_end);

    return TN_SUCCESS;
}

// private ------------------------------------------------------------------------------------------------------
int tn_term_csi_handle(tn_term_t *term, const struct tn_term_csi *csi)
{
    int unhandled = 0;

    switch (csi->seq_end) {
    case 'A':
        if (csi->open_count == 1) {
            tn_term_key_handle(term, TN_TERM_KEY_UP);
        } else if (csi->open_count == 2) {
            tn_term_key_handle(term, TN_TERM_KEY_F1);
        }
        break;
    case 'B':
        if (csi->open_count == 1) {
            tn_term_key_handle(term, TN_TERM_KEY_DOWN);
        } else if (csi->open_count == 2) {
            tn_term_key_handle(term, TN_TERM_KEY_F2);
        }
        break;
    case 'C':
        if (csi->open_count == 1) {
            tn_term_key_handle(term, TN_TERM_KEY_RIGHT);
        } else if (csi->open_count == 2) {
            tn_term_key_handle(term, TN_TERM_KEY_F3);
        }
        break;
    case 'D':
        if (csi->open_count == 1) {
            tn_term_key_handle(term, TN_TERM_KEY_LEFT);
        } else if (csi->open_count == 2) {
            tn_term_key_handle(term, TN_TERM_KEY_F4);
        }
        break;
    case 'E':
        if (csi->open_count == 2) {
            tn_term_key_handle(term, TN_TERM_KEY_F5);
        }
        break;
    case 'F':
        if (csi->open_count == 1) {
            tn_term_key_handle(term, TN_TERM_KEY_END);
        }
        break;
    case 'H':
        if (csi->open_count == 1) {
            tn_term_key_handle(term, TN_TERM_KEY_HOME);
        }
        break;
    case 'O': // single shift
        switch (csi->shift_char) {
        case 'P':
            tn_term_key_handle(term, TN_TERM_KEY_F1);
            break;
        case 'Q':
            tn_term_key_handle(term, TN_TERM_KEY_F2);
            break;
        case 'R':
            tn_term_key_handle(term, TN_TERM_KEY_F3);
            break;
        case 'S':
            tn_term_key_handle(term, TN_TERM_KEY_F4);
            break;
        }
        break;
    case 'R':
        term->pos_last.y = csi->param[0];
        term->pos_last.x = csi->param[1];
        if (term->debug_print) tn_term_write_direct(term, "[%d,%d]", term->pos_last.x, term->pos_last.y);
        break;
    case '~':
        switch (csi->param[0]) {
        case 1:
            tn_term_key_handle(term, TN_TERM_KEY_HOME);
            break;
        case 2:
            tn_term_key_handle(term, TN_TERM_KEY_INSERT);
            break;
        case 3:
            tn_term_key_handle(term, TN_TERM_KEY_DELETE);
            break;
        case 4:
            tn_term_key_handle(term, TN_TERM_KEY_END);
            break;
        case 15:
            tn_term_key_handle(term, TN_TERM_KEY_F5);
            break;
        case 17:
            tn_term_key_handle(term, TN_TERM_KEY_F6);
            break;
        case 18:
            tn_term_key_handle(term, TN_TERM_KEY_F7);
            break;
        case 19:
            tn_term_key_handle(term, TN_TERM_KEY_F8);
            break;
        case 20:
            tn_term_key_handle(term, TN_TERM_KEY_F9);
            break;
        case 21:
            tn_term_key_handle(term, TN_TERM_KEY_F10);
            break;
        case 23:
            tn_term_key_handle(term, TN_TERM_KEY_F11);
            break;
        case 24:
            tn_term_key_handle(term, TN_TERM_KEY_F12);
            break;
        default:
            return TN_ERROR;
        }
        break;
    default:
        return TN_ERROR;
    }

    return TN_SUCCESS;
}

// private ------------------------------------------------------------------------------------------------------
void tn_term_input_handle(tn_term_t *term, const char *buf, int len)
{
    char in_char;
    struct tn_term_csi csi;
    tn_term_priv_t *priv = term->priv;

    for (int i = 0; i < len; i++) {
        in_char = buf[i];

        switch (in_char) {
        case 3: // ctrl-C
        case 4: // ctrl-D
            tn_term_key_handle(term, TN_TERM_KEY_BREAK);
            tn_term_state_set(term, TN_TERM_STATE_STOPPING);
            return;
        case 8:
        case 127: // backspace
            tn_term_key_handle(term, TN_TERM_KEY_BACKSPACE);
            break;
        case 9: // tab
            tn_term_key_handle(term, TN_TERM_KEY_TAB);
            break;
        case 12:
        case 13:
        case 15: // line ending / enter key
            tn_term_key_handle(term, TN_TERM_KEY_ENTER);
            //if (priv->bbuf.len > 0) {
            //	TN_GUARD_CLEANUP(tn_term_buf_write_u8(term, '\0'));
            //	if (term->cb_cmd) term->cb_cmd((const char *)priv->bbuf.buffer);
            //	aws_byte_buf_reset(&priv->bbuf, true);
            //}
            break;
        case 27: // escape seq (or possible just esc keypress)
            if (tn_term_csi_parse(term, &buf[i], len - i, &csi) == TN_SUCCESS) {
                if (tn_term_csi_handle(term, &csi)) {
                    if (term->debug_print) tn_term_csi_print(term, &buf[i], csi.seq_len);
                }
                i += csi.seq_len - 1;
            } else {
                tn_term_key_handle(term, TN_TERM_KEY_ESC);
            }
            break;
        default:
            if (!isprint(in_char)) {
                tn_term_write_direct(term, "[%d]", (int)in_char);
            } else {
                if (term->cb_char) term->cb_char(in_char);
            }
            //printf("%c", in_char);
            TN_GUARD_CLEANUP(tn_term_buf_write_u8(term, in_char));
            break;
        }
    }

    return;

cleanup:
    tn_term_state_set(term, TN_TERM_STATE_ERROR);
}

// private ------------------------------------------------------------------------------------------------------
void tn_term_uv_cleanup(tn_term_t *term)
{
    tn_term_priv_t *priv = term->priv;
    // uv_tty_set_mode(&priv->uv_tty_in, UV_TTY_MODE_NORMAL);
    // uv_tty_reset_mode();

    if (uv_is_active((uv_handle_t *)&priv->uv_signal)) {
        uv_close((uv_handle_t *)&priv->uv_signal, on_term_close_cb);
    }

    if (uv_is_active((uv_handle_t *)&priv->uv_timer_tty)) {
        uv_close((uv_handle_t *)&priv->uv_timer_tty, on_term_close_cb);
    }

    if (uv_is_active((uv_handle_t *)&priv->uv_tty_in)) {
        uv_close((uv_handle_t *)&priv->uv_tty_in, on_term_close_cb);
    }

    if (uv_is_active((uv_handle_t *)&priv->uv_tty_out)) {
        uv_close((uv_handle_t *)&priv->uv_tty_in, on_term_close_cb);
    }
}

// private ------------------------------------------------------------------------------------------------------
void tn_term_queue_process(tn_term_t *term)
{
    const uint64_t batch_id = tn_atomic_load(&term->batch_id);
    //if (batch_id == term->batch_process_cache) return;

    uv_buf_t buf;
    struct tn_term_buf *term_buf;
    tn_term_priv_t *priv = term->priv;

    while (tn_queue_spsc_peek(&term->buffer_queue, (void **)&term_buf) == TN_SUCCESS) {
        if (batch_id == term->batch_process_cache) return;
        tn_queue_spsc_pop_cached(&term->buffer_queue);

        buf.base = term_buf->buf;
        buf.len = TN_BUFLEN_CAST(term_buf->len);
        TN_GUARD_CLEANUP(uv_try_write((uv_stream_t *)&priv->uv_tty_out, &buf, 1) <= 0);

        TN_GUARD_CLEANUP(tn_list_ptr_push_back(&term->buffer_stack, term_buf));
    }

    term->batch_process_cache++;

    return;

cleanup:
    tn_term_state_set(term, TN_TERM_STATE_ERROR);
}

// --------------------------------------------------------------------------------------------------------------
void on_term_close_cb(uv_handle_t *handle)
{
}

// --------------------------------------------------------------------------------------------------------------
void on_term_alloc_cb(uv_handle_t *handle, size_t size, uv_buf_t *buf)
{
    buf->base = malloc(size);
    buf->len = TN_BUFLEN_CAST(size);
}

// --------------------------------------------------------------------------------------------------------------
void on_term_read_cb(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf)
{
    tn_term_t *term = stream->data;
    tn_term_priv_t *priv = term->priv;
    if (nread <= 0) {
        tn_term_state_set(term, TN_TERM_STATE_STOPPING);
        return;
    }

    tn_term_input_handle(term, buf->base, (int)nread);
}

// --------------------------------------------------------------------------------------------------------------
void on_term_timer_cb(uv_timer_t *handle)
{
    tn_term_t *term = handle->data;
    tn_term_priv_t *priv = term->priv;

    if (handle == &priv->uv_timer_tty) {
        if (tn_term_state(term) == TN_TERM_STATE_STOPPING || tn_term_state(term) == TN_TERM_STATE_ERROR) {
            tn_term_uv_cleanup(term);
        } else {
            tn_term_queue_process(term);
        }
    }
}

// --------------------------------------------------------------------------------------------------------------
void on_term_signal_cb(uv_signal_t *handle, int signum)
{
    if (signum == SIGWINCH) {
        tn_term_t *term = handle->data;
        tn_term_priv_t *priv = term->priv;
        uv_tty_get_winsize(&priv->uv_tty_out, &term->size.x, &term->size.y);
        if (term->cb_resize) term->cb_resize(term->size.x, term->size.y);
    }
}

// --------------------------------------------------------------------------------------------------------------
void run_term_thread_io(void *data)
{
    TN_ASSERT(data);
    tn_term_t *term = (tn_term_t *)data;

    //int ret;
    tn_term_priv_t priv_scoped;
    tn_term_priv_t *priv = &priv_scoped;
    tn_term_state_t state = tn_term_state(term);

    TN_GUARD_CLEANUP(state != TN_TERM_STATE_STARTING);

    memset(priv, 0, sizeof(*priv));
    term->priv = priv;

    TN_GUARD_CLEANUP(aws_byte_buf_init(&priv->bbuf, aws_default_allocator(), TN_TERM_MAX_LINE));
    aws_byte_buf_reset(&priv->bbuf, true);
    priv->bpos = aws_byte_cursor_from_buf(&priv->bbuf);

    TN_GUARD_CLEANUP(uv_loop_init(&priv->uv_loop));

    TN_GUARD_CLEANUP(uv_signal_init(&priv->uv_loop, &priv->uv_signal));
    TN_GUARD_CLEANUP(uv_signal_start(&priv->uv_signal, on_term_signal_cb, SIGWINCH));
    priv->uv_signal.data = term;

    uv_timer_init(&priv->uv_loop, &priv->uv_timer_tty);
    uv_timer_start(&priv->uv_timer_tty, on_term_timer_cb, 10, 10);
    priv->uv_timer_tty.data = term;

    priv->ttyin_fd = 0;
    priv->ttyout_fd = 1;

    TN_GUARD_CLEANUP(uv_guess_handle(priv->ttyin_fd) != UV_TTY);
    TN_GUARD_CLEANUP(uv_guess_handle(priv->ttyout_fd) != UV_TTY);

    TN_GUARD_CLEANUP(uv_tty_init(&priv->uv_loop, &priv->uv_tty_in, priv->ttyin_fd, 1));
    TN_GUARD_CLEANUP(!uv_is_readable((uv_stream_t *)&priv->uv_tty_in));
    priv->uv_tty_in.data = term;

    TN_GUARD_CLEANUP(uv_tty_init(&priv->uv_loop, &priv->uv_tty_out, priv->ttyout_fd, 0));
    TN_GUARD_CLEANUP(!uv_is_writable((uv_stream_t *)&priv->uv_tty_out));
    priv->uv_tty_out.data = term;

    TN_GUARD_CLEANUP(uv_tty_get_winsize(&priv->uv_tty_out, &term->size.x, &term->size.y));
    if (term->cb_resize) term->cb_resize(term->size.x, term->size.y);

    TN_GUARD_CLEANUP(uv_tty_set_mode(&priv->uv_tty_in, UV_TTY_MODE_RAW));
    TN_GUARD_CLEANUP(uv_read_start((uv_stream_t *)&priv->uv_tty_in, on_term_alloc_cb, on_term_read_cb));

    state = tn_term_state(term);
    if (state == TN_TERM_STATE_STARTING) {
        tn_term_state_set(term, TN_TERM_STATE_STARTED);
        TN_GUARD_CLEANUP(uv_run(&priv->uv_loop, UV_RUN_DEFAULT));
    }

    tn_term_state_set(term, TN_TERM_STATE_STOPPED);
    return;

cleanup:
    tn_term_state_set(term, TN_TERM_STATE_ERROR);
}

// --------------------------------------------------------------------------------------------------------------
void tn_term_debug_print(tn_term_t *term, bool enabled)
{
    TN_ASSERT(term);
    term->debug_print = enabled ? 1 : 0;
}

// --------------------------------------------------------------------------------------------------------------
tn_term_state_t tn_term_state(tn_term_t *term)
{
    TN_ASSERT(term);
    return tn_atomic_load(&term->state);
}

// --------------------------------------------------------------------------------------------------------------
void tn_term_callback_char(tn_term_t *term, tn_term_callback_char_func cb_char)
{
    TN_ASSERT(term);
    term->cb_char = cb_char;
}

// --------------------------------------------------------------------------------------------------------------
void tn_term_callback_key(tn_term_t *term, tn_term_callback_key_func cb_key)
{
    TN_ASSERT(term);
    term->cb_key = cb_key;
}

// --------------------------------------------------------------------------------------------------------------
void tn_term_callback_resize(tn_term_t *term, tn_term_callback_resize_func cb_resize)
{
    TN_ASSERT(term);
    term->cb_resize = cb_resize;
}

// --------------------------------------------------------------------------------------------------------------
int tn_term_setup(tn_term_t *term)
{
    TN_ASSERT(term);

    TN_GUARD_CLEANUP(term->priv != NULL);

    tn_mutex_setup(&term->mtx);

    memset(&term->buffer_stack, 0, sizeof(term->buffer_stack));
    TN_GUARD_NULL_CLEANUP(term->buffer_pool = TN_MEM_ACQUIRE(sizeof(*term->buffer_pool) * TN_TERM_BUFFER_STACK_SIZE));

    TN_GUARD_CLEANUP(tn_list_ptr_setup(&term->buffer_stack, TN_TERM_BUFFER_STACK_SIZE));

    struct tn_term_buf *tb;
    for (int i = 1; i <= TN_TERM_BUFFER_STACK_SIZE; i++) {
        tb = term->buffer_pool + TN_TERM_BUFFER_STACK_SIZE - i;
        TN_GUARD_CLEANUP(tn_list_ptr_push_back(&term->buffer_stack, tb));
    }

    TN_GUARD_CLEANUP(tn_queue_spsc_setup(&term->buffer_queue, TN_TERM_MAX_LINE));

    tn_atomic_store(&term->batch_id, 0);
    term->batch_request_cache = 0;
    term->batch_process_cache = 0;

    term->cb_char = NULL;
    term->cb_key = NULL;
    term->cb_resize = NULL;

    return TN_SUCCESS;

cleanup:
    tn_term_state_set(term, TN_TERM_STATE_ERROR);
    return TN_ERROR;
}

// --------------------------------------------------------------------------------------------------------------
void tn_term_cleanup(tn_term_t *term)
{
    TN_ASSERT(term);

    tn_queue_spsc_cleanup(&term->buffer_queue);
    tn_list_ptr_cleanup(&term->buffer_stack);
    TN_MEM_RELEASE(term->buffer_pool);
    tn_mutex_cleanup(&term->mtx);

    uv_tty_reset_mode();
}

// --------------------------------------------------------------------------------------------------------------
int tn_term_start(tn_term_t *term)
{
    TN_ASSERT(term);

    tn_term_state_t state = tn_term_state(term);
    TN_GUARD(state != TN_TERM_STATE_STOPPED && state != TN_TERM_STATE_NEW);

    tn_term_state_set(term, TN_TERM_STATE_STARTING);
    TN_GUARD_CLEANUP(tn_thread_launch(&term->thread_io, run_term_thread_io, term));

    return TN_SUCCESS;

cleanup:
    tn_term_state_set(term, TN_TERM_STATE_ERROR);
    return TN_ERROR;
}

// --------------------------------------------------------------------------------------------------------------
int tn_term_stop(tn_term_t *term)
{
    TN_ASSERT(term);

    tn_term_state_t state = tn_term_state(term);
    TN_GUARD(state == TN_TERM_STATE_STOPPING || state == TN_TERM_STATE_STOPPED);

    tn_term_state_set(term, TN_TERM_STATE_STOPPING);

    return TN_SUCCESS;
}

// --------------------------------------------------------------------------------------------------------------
int tn_term_clear(tn_term_t *term)
{
    return tn_term_write(term, "\033[2J");
}

// --------------------------------------------------------------------------------------------------------------
int tn_term_clear_down(tn_term_t *term)
{
    return tn_term_write(term, "\033[J");
}

// --------------------------------------------------------------------------------------------------------------
int tn_term_clear_up(tn_term_t *term)
{
    return tn_term_write(term, "\033[1J");
}

// --------------------------------------------------------------------------------------------------------------
int tn_term_clear_line(tn_term_t *term)
{
    return tn_term_write(term, "\033[2K");
}

// --------------------------------------------------------------------------------------------------------------
int tn_term_clear_line_home(tn_term_t *term)
{
    return tn_term_write(term, "\033[1K");
}

// --------------------------------------------------------------------------------------------------------------
int tn_term_clear_line_end(tn_term_t *term)
{
    return tn_term_write(term, "\033[K");
}

// --------------------------------------------------------------------------------------------------------------
int tn_term_pos_get(tn_term_t *term)
{
    return tn_term_write(term, "\033[6n");
}

// --------------------------------------------------------------------------------------------------------------
int tn_term_pos_set(tn_term_t *term, uint16_t x, uint16_t y)
{
    x = x ? x : 1;
    y = y ? y : 1;
    return tn_term_write(term, "\033[%u;%uH", y, x);
}

// --------------------------------------------------------------------------------------------------------------
int tn_term_pos_store(tn_term_t *term)
{
    return tn_term_write(term, "\033[s");
}

// --------------------------------------------------------------------------------------------------------------
int tn_term_pos_restore(tn_term_t *term)
{
    return tn_term_write(term, "\033[u");
}

// --------------------------------------------------------------------------------------------------------------
int tn_term_pos_up(tn_term_t *term, uint16_t count)
{
    return tn_term_write(term, "\033[%dA", count ? count : 1);
}

// --------------------------------------------------------------------------------------------------------------
int tn_term_pos_down(tn_term_t *term, uint16_t count)
{
    return tn_term_write(term, "\033[%dB", count ? count : 1);
}

// --------------------------------------------------------------------------------------------------------------
int tn_term_pos_right(tn_term_t *term, uint16_t count)
{
    return tn_term_write(term, "\033[%dC", count ? count : 1);
}

// --------------------------------------------------------------------------------------------------------------
int tn_term_pos_left(tn_term_t *term, uint16_t count)
{
    return tn_term_write(term, "\033[%dD", count ? count : 1);
}

// --------------------------------------------------------------------------------------------------------------
int tn_term_write(tn_term_t *term, const char *fmt, ...)
{
    TN_ASSERT(term);

    struct tn_term_buf *term_buf;
    TN_GUARD(tn_list_ptr_pop_back(&term->buffer_stack, (void **)&term_buf));

    va_list args;
    va_start(args, fmt);
    term_buf->len = vsnprintf(term_buf->buf, TN_TERM_MAX_LINE, fmt, args);
    va_end(args);

    if (!term_buf->len) return TN_SUCCESS;

    term_buf->batch_id = term->batch_request_cache;

    TN_GUARD(term_buf->len < 0);
    return tn_queue_spsc_push(&term->buffer_queue, term_buf);
}

// --------------------------------------------------------------------------------------------------------------
void tn_term_flush(tn_term_t *term)
{
    TN_ASSERT(term);
    term->batch_request_cache++;
    tn_atomic_store(&term->batch_id, term->batch_request_cache);
}

// --------------------------------------------------------------------------------------------------------------
int tn_term_color_set(tn_term_t *term, uint8_t color)
{
    return tn_term_write(term, "\033[38;5;%dm", color);
}

// --------------------------------------------------------------------------------------------------------------
int tn_term_bgcolor_set(tn_term_t *term, uint8_t color)
{
    return tn_term_write(term, "\033[48;5;%dm", color);
}

// --------------------------------------------------------------------------------------------------------------
uint8_t tn_term_color16(tn_term_t *term, enum tn_term_color color)
{
    return (color > 0xff) ? 0xff : color;
}

// --------------------------------------------------------------------------------------------------------------
uint8_t tn_term_color256(tn_term_t *term, uint8_t r, uint8_t g, uint8_t b)
{
    r = (r > 5) ? 5 : r;
    g = (g > 5) ? 5 : g;
    b = (b > 5) ? 5 : b;
    return (r * 36) + (g * 6) + b + 16;
}

// --------------------------------------------------------------------------------------------------------------
uint8_t tn_term_grey24(tn_term_t *term, uint8_t grey)
{
    return (grey > 23) ? 23 : grey;
}
