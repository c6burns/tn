#include "tn/term.h"

#include "uv.h"
#ifdef _WIN32
#	include <io.h>
#endif
#include <ctype.h>

#include "tn/buffer.h"


typedef struct rc_term_priv_s {
	uv_loop_t uv_loop;
	uv_signal_t uv_signal;
	int ttyin_fd, ttyout_fd;
	uv_tty_t uv_tty_in, uv_tty_out;
	uv_timer_t uv_timer_tty;
	struct aws_byte_buf bbuf;
	struct aws_byte_cursor bpos;
} tn_term_priv_t;


// private ------------------------------------------------------------------------------------------------------
void tn_term_state_set(tn_term_t *term, tn_term_state_t state)
{
	TN_ASSERT(term);
	tn_atomic_store(&term->state, state);
}

// private ------------------------------------------------------------------------------------------------------
int tn_term_buf_write_u8(tn_term_t *term, char in_char)
{
	TN_ASSERT(term && term->priv);

	int ret = TN_ERROR;
	tn_term_priv_t *priv = term->priv;

	tn_mutex_lock(&term->mtx);
	TN_GUARD_CLEANUP(!aws_byte_buf_write_u8(&priv->bbuf, in_char));

	ret = TN_SUCCESS;

cleanup:
	tn_mutex_unlock(&term->mtx);
	return ret;
}

// private ------------------------------------------------------------------------------------------------------
int tn_term_csi_parse(tn_term_t *term, const char *input, int maxlen, struct tn_term_csi *csi)
{
	TN_ASSERT(term && input && csi);

	char str[128];
	int istr = 0;
	memset(csi, 0, sizeof(*csi));

	TN_GUARD(maxlen < 3);
	TN_GUARD(input[0] != '\x1b');

	for (int i = 1; i < maxlen; i++) {
		char c = input[i];

		if (isdigit(c)) {
			str[istr++] = c;
			continue;
		} else if (c == '[') {
			csi->open_count++;
			continue;
		}
		
		if (istr) {
			str[istr] = '\0';
			csi->param[csi->param_count++] = atoi(str);
			istr = 0;
		} else {
			csi->param[csi->param_count++] = 0;
		}

		if (isalpha(c) || c == '~') {
			csi->seq_end = c;
			csi->seq_len = i + 1;
			break;
		}
	}

	TN_GUARD(!csi->seq_end);

	return TN_SUCCESS;
}

// --------------------------------------------------------------------------------------------------------------
void on_term_close_cb(uv_handle_t *handle)
{
}

// --------------------------------------------------------------------------------------------------------------
void on_term_alloc_cb(uv_handle_t* handle, size_t size, uv_buf_t* buf)
{
	buf->base = malloc(size);
	buf->len = TN_BUFLEN_CAST(size);
}

// --------------------------------------------------------------------------------------------------------------
void on_term_read_cb(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf)
{
	char in_char;
	int cmd_len = 0;
	struct tn_term_csi csi;
    tn_term_t *term = stream->data;
	tn_term_priv_t *priv = term->priv;
	if (nread <= 0) {
		tn_term_state_set(term, TN_TERM_STATE_STOPPING);
		return;
	}

	for (int i = 0; i < nread; i++) {
		in_char = buf->base[i];
		if (term->cb_char) {
			term->cb_char(in_char);
		}

		switch (in_char) {
		case 3: // ctrl-C
		case 4: // ctrl-D
			tn_term_state_set(term, TN_TERM_STATE_STOPPING);
			return;
		case 12:
		case 13:
		case 15: // line ending
			if (priv->bbuf.len > 0) {
				TN_GUARD_CLEANUP(tn_term_buf_write_u8(term, '\0'));
				printf("\ncommand: %s\n\n", priv->bbuf.buffer);
				if (term->cb_cmd) {
					term->cb_cmd(priv->bbuf.buffer);
				}
				aws_byte_buf_reset(&priv->bbuf, true);
			}
			break;
		case 27:
			if (tn_term_csi_parse(term, &buf->base[i], (int)nread - i, &csi) == TN_SUCCESS) {
				i += csi.seq_len - 1;

				switch (csi.seq_end) {
				case 'A':
					if (csi.open_count == 1) {
						printf("[up]");
					} else if (csi.open_count == 2) {
						printf("[F1]");
					}
					break;
				case 'B':
					if (csi.open_count == 1) {
						printf("[down]");
					} else if (csi.open_count == 2) {
						printf("[F2]");
					}
					break;
				case 'C':
					if (csi.open_count == 1) {
						printf("[right]");
						//tn_term_pos_right(term, csi.param[0]);
					} else if (csi.open_count == 2) {
						printf("[F3]");
					}
					break;
				case 'D':
					if (csi.open_count == 1) {
						printf("[left]");
						//tn_term_pos_left(term, csi.param[0]);
					} else if (csi.open_count == 2) {
						printf("[F4]");
					}
					break;
				case 'E':
					if (csi.open_count == 2) {
						printf("[F5]");
					}
					break;
				case 'R':
					term->pos_last.x = csi.param[0];
					term->pos_last.y = csi.param[1];
					break;
				case '~':
					switch (csi.param[0]) {
					case 1:
						printf("[home]");
						break;
					case 2:
						printf("[ins]");
						break;
					case 3:
						printf("[del]");
						break;
					case 4:
						printf("[end]");
						break;
					case 17:
						printf("[F6]");
						break;
					case 18:
						printf("[F7]");
						break;
					case 19:
						printf("[F8]");
						break;
					case 20:
						printf("[F9]");
						break;
					case 21:
						printf("[F10]");
						break;
					case 23:
						printf("[F11]");
						break;
					case 24:
						printf("[F12]");
						break;
					}
					break;
				}

				break;
			}

			printf("%c", in_char);
			break;
		default:
			printf("%d -- %d -- %c\n", (int)nread, (int)in_char, in_char);
			//printf("%c", in_char);
			TN_GUARD_CLEANUP(tn_term_buf_write_u8(term, in_char));
			break;
		}
	}

	return;

cleanup:
	printf("error writing to buffer\n");
	tn_term_state_set(term, TN_TERM_STATE_STOPPING);
	return;
}

// --------------------------------------------------------------------------------------------------------------
void on_term_timer_cb(uv_timer_t* handle)
{
	tn_term_t *term = handle->data;
	tn_term_priv_t *priv = term->priv;
	if (handle == &priv->uv_timer_tty) {
		if (tn_term_state(term) == TN_TERM_STATE_STOPPING) {
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
		} else {
			uv_buf_t buf;
			struct tn_term_buf *qreq;
			while (!tn_queue_spsc_pop_back(&term->queue_log, (void **)&qreq)) {
				buf.base = qreq->buf;
				buf.len = TN_BUFLEN_CAST(qreq->len);
				uv_try_write((uv_stream_t*)&priv->uv_tty_out, &buf, 1);

				if (!memcmp(qreq->buf, "I love potatoes :D :D :D", 10)) {
					tn_term_pos_store(term);
				}

				TN_MEM_RELEASE(qreq);
			}
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
	}
}

// --------------------------------------------------------------------------------------------------------------
void run_term_thread_io(tn_term_t *term)
{
	TN_ASSERT(term);

	//int ret;
	tn_term_priv_t priv_scoped;
	tn_term_priv_t *priv = &priv_scoped;
	tn_term_state_t state = tn_term_state(term);

	TN_GUARD_CLEANUP(state != TN_TERM_STATE_STARTING);
	
	memset(priv, 0, sizeof(*priv));
	tn_mutex_setup(&term->mtx);
	term->priv = priv;

	TN_GUARD_CLEANUP(aws_byte_buf_init(&priv->bbuf, aws_default_allocator(), TN_TERM_MAX_LINE));
	aws_byte_buf_reset(&priv->bbuf, true);
	priv->bpos = aws_byte_cursor_from_buf(&priv->bbuf);

	TN_GUARD_CLEANUP(uv_loop_init(&priv->uv_loop));

	TN_GUARD_CLEANUP(uv_signal_init(&priv->uv_loop, &priv->uv_signal));
	TN_GUARD_CLEANUP(uv_signal_start(&priv->uv_signal, on_term_signal_cb, SIGWINCH));
	priv->uv_signal.data = term;

	uv_timer_init(&priv->uv_loop, &priv->uv_timer_tty);
	uv_timer_start(&priv->uv_timer_tty, on_term_timer_cb, 50, 50);
	priv->uv_timer_tty.data = term;

#ifdef _WIN32
	HANDLE handle;
	handle = CreateFileA("conin$",
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
	TN_ASSERT(handle != INVALID_HANDLE_VALUE);
	priv->ttyin_fd = _open_osfhandle((intptr_t)handle, 0);

	handle = CreateFileA("conout$",
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
	TN_ASSERT(handle != INVALID_HANDLE_VALUE);
	priv->ttyout_fd = _open_osfhandle((intptr_t)handle, 0);
#else /* unix */
	priv->ttyin_fd = open("/dev/tty", O_RDONLY, 0);
	if (priv->ttyin_fd < 0) {
		fprintf(stderr, "Cannot open /dev/tty as read-only: %s\n", strerror(errno));
		fflush(stderr);
		goto cleanup;
	}

	priv->ttyout_fd = open("/dev/tty", O_WRONLY, 0);
	if (priv->ttyout_fd < 0) {
		fprintf(stderr, "Cannot open /dev/tty as write-only: %s\n", strerror(errno));
		fflush(stderr);
		goto cleanup;
	}
#endif

	TN_ASSERT(priv->ttyin_fd >= 0);
	TN_ASSERT(priv->ttyout_fd >= 0);

	TN_ASSERT(UV_UNKNOWN_HANDLE == uv_guess_handle(-1));

	TN_ASSERT(UV_TTY == uv_guess_handle(priv->ttyin_fd));
	TN_ASSERT(UV_TTY == uv_guess_handle(priv->ttyout_fd));

	TN_GUARD_CLEANUP(uv_tty_init(&priv->uv_loop, &priv->uv_tty_in, priv->ttyin_fd, 1));
	TN_ASSERT(uv_is_readable((uv_stream_t*)&priv->uv_tty_in));
	TN_ASSERT(!uv_is_writable((uv_stream_t*)&priv->uv_tty_in));
	priv->uv_tty_in.data = term;

	TN_GUARD_CLEANUP(uv_tty_init(&priv->uv_loop, &priv->uv_tty_out, priv->ttyout_fd, 0));
	TN_ASSERT(!uv_is_readable((uv_stream_t*)&priv->uv_tty_out));
	TN_ASSERT(uv_is_writable((uv_stream_t*)&priv->uv_tty_out));
	priv->uv_tty_out.data = term;

	TN_GUARD_CLEANUP(uv_tty_get_winsize(&priv->uv_tty_out, &term->size.x, &term->size.y));

	TN_GUARD_CLEANUP(uv_tty_set_mode(&priv->uv_tty_in, UV_TTY_MODE_RAW));
	TN_GUARD_CLEANUP(uv_read_start((uv_stream_t *)&priv->uv_tty_in, on_term_alloc_cb, on_term_read_cb));

	//if (uv_guess_handle(1) == UV_TTY) {
	//	uv_write_t req;
	//	uv_buf_t buf;
	//	buf.base = "\033[41;37m";
	//	buf.len = strlen(buf.base);
	//	uv_write(&req, (uv_stream_t*)&tty_out, &buf, 1, NULL);
	//}

	state = tn_term_state(term);
	if (state == TN_TERM_STATE_STARTING) {
		tn_term_state_set(term, TN_TERM_STATE_STARTED);

		tn_term_write(term, "I love potatoes :D :D :D\n\n");

		TN_GUARD_CLEANUP(uv_run(&priv->uv_loop, UV_RUN_DEFAULT));
	}

	uv_tty_reset_mode();
	tn_term_state_set(term, TN_TERM_STATE_STOPPED);
	tn_mutex_cleanup(&term->mtx);
	return;

cleanup:
	printf("term thread error\n");
	tn_term_state_set(term, TN_TERM_STATE_ERROR);
}

// --------------------------------------------------------------------------------------------------------------
tn_term_state_t tn_term_state(tn_term_t *term)
{
	TN_ASSERT(term);
	return tn_atomic_load(&term->state);
}

// --------------------------------------------------------------------------------------------------------------
int tn_term_read_char(char *out_c)
{
	// if (!aws_byte_cursor_read_u8(&priv.bpos, out_c)) return -1;
	return 0;
}

// --------------------------------------------------------------------------------------------------------------
int tn_term_setup(tn_term_t *term)
{
	TN_ASSERT(term);
	return TN_ERROR;
}

// --------------------------------------------------------------------------------------------------------------
void tn_term_cleanup(tn_term_t *term)
{
	TN_ASSERT(term);
}

// --------------------------------------------------------------------------------------------------------------
int tn_term_start(tn_term_t *term, tn_term_callback_char_func cb_char, tn_term_callback_cmd_func cb_cmd)
{
	TN_ASSERT(term);

	tn_term_state_t state = tn_term_state(term);
	TN_GUARD(state != TN_TERM_STATE_STOPPED && state != TN_TERM_STATE_NEW);

	term->cb_char = cb_char;
	term->cb_cmd = cb_cmd;
	tn_term_state_set(term, TN_TERM_STATE_STARTING);
	TN_GUARD_CLEANUP(tn_thread_launch(&term->thread_io, run_term_thread_io, term));

	TN_GUARD_CLEANUP(tn_queue_spsc_setup(&term->queue_log, TN_TERM_MAX_LINE));

	return TN_SUCCESS;

cleanup:

	tn_term_state_set(term, TN_TERM_STATE_STOPPING);
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
	return tn_term_write(term, "\x1b[2J");
}

// --------------------------------------------------------------------------------------------------------------
int tn_term_clear_down(tn_term_t *term)
{
	return tn_term_write(term, "\x1b[J");
}

// --------------------------------------------------------------------------------------------------------------
int tn_term_clear_up(tn_term_t *term)
{
	return tn_term_write(term, "\x1b[1J");
}

// --------------------------------------------------------------------------------------------------------------
int tn_term_clear_line(tn_term_t *term)
{
	return tn_term_write(term, "\x1b[2K");
}

// --------------------------------------------------------------------------------------------------------------
int tn_term_clear_line_home(tn_term_t *term)
{
	return tn_term_write(term, "\x1b[1K");
}

// --------------------------------------------------------------------------------------------------------------
int tn_term_clear_line_end(tn_term_t *term)
{
	return tn_term_write(term, "\x1b[K");
}

// --------------------------------------------------------------------------------------------------------------
int tn_term_pos_store(tn_term_t *term)
{
	return tn_term_write(term, "\x1b[6n");
}

// --------------------------------------------------------------------------------------------------------------
int tn_term_pos_restore(tn_term_t *term)
{
	return tn_term_write(term, "\x1b[6n");
}

// --------------------------------------------------------------------------------------------------------------
int tn_term_pos_up(tn_term_t *term, int count)
{
	return tn_term_write(term, "\x1b[%dA", count ? count : 1);
}

// --------------------------------------------------------------------------------------------------------------
int tn_term_pos_down(tn_term_t *term, int count)
{
	return tn_term_write(term, "\x1b[%dB", count ? count : 1);
}

// --------------------------------------------------------------------------------------------------------------
int tn_term_pos_right(tn_term_t *term, int count)
{
	return tn_term_write(term, "\x1b[%dC", count ? count : 1);
}

// --------------------------------------------------------------------------------------------------------------
int tn_term_pos_left(tn_term_t *term, int count)
{
	return tn_term_write(term, "\x1b[%dD", count ? count : 1);
}

// --------------------------------------------------------------------------------------------------------------
int tn_term_write(tn_term_t *term, const char *fmt, ...)
{
	struct tn_term_buf *req;
	TN_GUARD_NULL(req = TN_MEM_ACQUIRE(sizeof(struct tn_term_buf)));

	va_list args;
	va_start(args, fmt);
	req->len = vsnprintf(req->buf, TN_TERM_MAX_LINE, fmt, args);
	va_end(args);

	TN_GUARD(req->len < 0);
	return tn_queue_spsc_push(&term->queue_log, req);
}