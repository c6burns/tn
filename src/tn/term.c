#include "tn/term.h"

#include <assert.h>

#include "uv.h"

#include "tn/error.h"
#include "tn/allocator.h"
#include "tn/buffer.h"


typedef struct rc_term_priv_s {
	int ttyin_fd, ttyout_fd;
	uv_tty_t uv_tty_in, uv_tty_out;
	uv_timer_t uv_tty_timer;
	struct aws_byte_buf bbuf;
	struct aws_byte_cursor bpos;
	uv_loop_t *uv_loop;
} tn_term_priv_t;


int rc_term_read_char(char *out_c)
{
	// if (!aws_byte_cursor_read_u8(&priv.bpos, out_c)) return -1;
	return 0;
}


void on_term_alloc_cb(uv_handle_t* handle, size_t size, uv_buf_t* buf)
{
	buf->base = malloc(size);
	buf->len = size;
}

void on_term_read_cb(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf)
{
    tn_term_t *term = stream->data;
	tn_term_priv_t *priv = term->priv;
	if (nread <= 0) {
		term->state = TN_TERM_STATE_STOPPING;
		return;
	}

	buf->base[nread] = 0;
	for (int i = 0; i < nread; i++) {
		switch (buf->base[i]) {
		case 3:
		case 4:
			term->state = TN_TERM_STATE_STOPPING;
			break;
		}
	}

	printf("%d -- %d -- %s\n", nread, (int)buf->base[0], buf->base);
	if (!aws_byte_buf_write(&priv->bbuf, buf->base, nread)) {
		printf("error writing to buffer\n");
		term->state = TN_TERM_STATE_STOPPING;
		return;
	}
}

static void on_term_timer_cb(uv_timer_t* handle)
{
	tn_term_t *term = handle->data;
	if (term->state == TN_TERM_STATE_STOPPING) {
		if (uv_is_active((uv_handle_t *)handle)) {
			uv_close((uv_handle_t *)handle, NULL);
		}
	}
}


int tn_term_start(tn_term_t *term)
{
	int ret;

    memset(term, 0, sizeof(*term));
	TN_GUARD_NULL(term->priv = TN_MEM_ACQUIRE(sizeof(tn_term_priv_t)));

	tn_term_priv_t *priv = term->priv;
	memset(priv, 0, sizeof(*priv));

	TN_GUARD_NULL(priv->uv_loop = TN_MEM_ACQUIRE(sizeof(*priv->uv_loop)));
	TN_GUARD(uv_loop_init(priv->uv_loop));

	aws_byte_buf_init(&priv->bbuf, &tn_aws_default_allocator, 4096);
	priv->bpos = aws_byte_cursor_from_buf(&priv->bbuf);

	uv_timer_init(priv->uv_loop, &priv->uv_tty_timer);
	uv_timer_start(&priv->uv_tty_timer, on_term_timer_cb, 100, 100);
	priv->uv_tty_timer.data = term;

#ifdef _WIN32
	HANDLE handle;
	handle = CreateFileA("conin$",
						GENERIC_READ | GENERIC_WRITE,
						FILE_SHARE_READ | FILE_SHARE_WRITE,
						NULL,
						OPEN_EXISTING,
						FILE_ATTRIBUTE_NORMAL,
						NULL);
	ASSERT(handle != INVALID_HANDLE_VALUE);
	priv->uv_ttyin_fd = _open_osfhandle((intptr_t) handle, 0);

	handle = CreateFileA("conout$",
						GENERIC_READ | GENERIC_WRITE,
						FILE_SHARE_READ | FILE_SHARE_WRITE,
						NULL,
						OPEN_EXISTING,
						FILE_ATTRIBUTE_NORMAL,
						NULL);
	ASSERT(handle != INVALID_HANDLE_VALUE);
	priv->uv_ttyout_fd = _open_osfhandle((intptr_t) handle, 0);
#else /* unix */
	priv->ttyin_fd = open("/dev/tty", O_RDONLY, 0);
	if (priv->ttyin_fd < 0) {
		fprintf(stderr, "Cannot open /dev/tty as read-only: %s\n", strerror(errno));
		fflush(stderr);
		return -1;
	}

	priv->ttyout_fd = open("/dev/tty", O_WRONLY, 0);
	if (priv->ttyout_fd < 0) {
		fprintf(stderr, "Cannot open /dev/tty as write-only: %s\n", strerror(errno));
		fflush(stderr);
		return -1;
	}
#endif

	assert(priv->ttyin_fd >= 0);
	assert(priv->ttyout_fd >= 0);

	assert(UV_UNKNOWN_HANDLE == uv_guess_handle(-1));

	assert(UV_TTY == uv_guess_handle(priv->ttyin_fd));
	assert(UV_TTY == uv_guess_handle(priv->ttyout_fd));

	ret = uv_tty_init(priv->uv_loop, &priv->uv_tty_in, priv->ttyin_fd, 1);
	assert(ret == 0);
	assert(uv_is_readable((uv_stream_t*)&priv->uv_tty_in));
	assert(!uv_is_writable((uv_stream_t*)&priv->uv_tty_in));
	priv->uv_tty_in.data = term;

	ret = uv_tty_init(priv->uv_loop, &priv->uv_tty_out, priv->ttyout_fd, 0);
	assert(ret == 0);
	assert(!uv_is_readable((uv_stream_t*)&priv->uv_tty_out));
	assert(uv_is_writable((uv_stream_t*)&priv->uv_tty_out));
	priv->uv_tty_out.data = term;

	ret = uv_tty_get_winsize(&priv->uv_tty_out, &term->width, &term->height);
	assert(ret == 0);

	printf("width=%d height=%d\n", term->width, term->height);

	if (uv_read_start((uv_stream_t *)&priv->uv_tty_in, on_term_alloc_cb, on_term_read_cb)) {
		printf("Error starting read on tty!!!!\n");
	}

	int tty_w, tty_h;
	if (uv_tty_get_winsize(&priv->uv_tty_out, &tty_w, &tty_h)) {
		printf("Error getting winsize!!!!\n");
	} else {
		printf("winsize: %d x %d\n", tty_w, tty_h);
	}

	assert(0 == uv_tty_set_mode(&priv->uv_tty_in, UV_TTY_MODE_RAW));

	//if (uv_guess_handle(1) == UV_TTY) {
	//	uv_write_t req;
	//	uv_buf_t buf;
	//	buf.base = "\033[41;37m";
	//	buf.len = strlen(buf.base);
	//	uv_write(&req, (uv_stream_t*)&tty_out, &buf, 1, NULL);
	//}

	uv_write_t req;
	uv_buf_t buf;
	buf.base = "Hello TTY\n";
	buf.len = strlen(buf.base);
	uv_write(&req, (uv_stream_t*)&priv->uv_tty_out, &buf, 1, NULL);

	//printf("AWWWWWW YEAH!!!!");
	uv_run(priv->uv_loop, UV_RUN_DEFAULT);

	uv_tty_reset_mode();

	return 0;
}
