/*
 * Copyright (c) 2017 rxi -- this is the original copyright holder. Thanks guy!
 * Additional Copyright (c) 2019 c6burns -- many a modification
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#if _MSC_VER
#    include <windows.h>
#endif

#include "tn/log.h"

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "aws/common/condition_variable.h"
#include "aws/common/mutex.h"
#include "aws/common/thread.h"

static const char *level_names[] = {
    "TRACE",
    "DEBUG",
    "INFO",
    "WARN",
    "ERROR",
    "FATAL",
};

#ifdef TN_LOG_USE_COLOR
static const char *level_colors[] = {
    "\033[90m",
    "\033[36m",
    "\033[32m",
    "\033[33m",
    "\033[31m",
    "\033[35m",
};
#endif

typedef struct aws_mutex aws_mutex_t;
aws_mutex_t tn_log_mtx;
FILE *tn_log_fp = NULL;
int tn_log_ready = 0;

typedef struct tn_log_s {
    void *udata;
    tn_log_lock_fn lock;
    FILE *fp;
    int level;
    int quiet;
    int color;
} tn_log_t;

static tn_log_t tn_log_ctx;

//typedef struct aws_thread tn_thread_t;
//typedef struct aws_condition_variable tn_condition_t;
//
//typedef struct tn_log_thread_private_s {
//	tn_condition_t cond;
//} tn_log_thread_private_t;
//tn_thread_t tn_log_thread;

// private ------------------------------------------------------------------------------------------------------
void tn_log_lock_impl(void *udata, int lock)
{
    aws_mutex_t *mtx = (aws_mutex_t *)udata;
    if (lock)
        aws_mutex_lock(mtx);
    else
        aws_mutex_unlock(mtx);
}

// --------------------------------------------------------------------------------------------------------------
void tn_log_setup(void)
{
#ifndef TN_LOG_DISABLE
    if (tn_log_ready) return;
    tn_log_ready = 1;

    memset(&tn_log_ctx, 0, sizeof(tn_log_ctx));

    tn_log_color(1);

    aws_mutex_init(&tn_log_mtx);
    tn_log_set_udata(&tn_log_mtx);
    tn_log_set_lock(tn_log_lock_impl);

#    ifdef TN_LOG_FILENAME
    tn_log_fp = fopen(TN_LOG_FILENAME, "a+");
    if (tn_log_fp) tn_log_set_fp(tn_log_fp);
#    endif

    tn_log_trace("Application logging started ... ");

    atexit(tn_log_cleanup);
#endif
}

// --------------------------------------------------------------------------------------------------------------
void tn_log_cleanup(void)
{
#ifndef TN_LOG_DISABLE
    tn_log_trace("Application logging completed");

    if (!tn_log_ready) return;
    tn_log_ready = 0;
    aws_mutex_clean_up(&tn_log_mtx);
    if (tn_log_fp) fclose(tn_log_fp);
#endif
}

void tn_log_color(int enable)
{
#ifdef TN_LOG_USE_COLOR
    if (enable) {
#    if _MSC_VER
        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        if (hOut == INVALID_HANDLE_VALUE) {
            enable = 0;
        }

        DWORD dwMode = 0;
        if (!GetConsoleMode(hOut, &dwMode)) {
            enable = 0;
        }

        dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
        if (!SetConsoleMode(hOut, dwMode)) {
            enable = 0;
        }
#    endif
    }

    tn_log_ctx.color = enable;
#endif
}

// --------------------------------------------------------------------------------------------------------------
static void tn_log_lock(void)
{
    if (tn_log_ctx.lock) {
        tn_log_ctx.lock(tn_log_ctx.udata, 1);
    }
}

// --------------------------------------------------------------------------------------------------------------
static void tn_log_unlock(void)
{
    if (tn_log_ctx.lock) {
        tn_log_ctx.lock(tn_log_ctx.udata, 0);
    }
}

// --------------------------------------------------------------------------------------------------------------
void tn_log_set_udata(void *udata)
{
    tn_log_ctx.udata = udata;
}

// --------------------------------------------------------------------------------------------------------------
void tn_log_set_lock(tn_log_lock_fn fn)
{
    tn_log_ctx.lock = fn;
}

// --------------------------------------------------------------------------------------------------------------
void tn_log_set_fp(FILE *fp)
{
    tn_log_ctx.fp = fp;
}

// --------------------------------------------------------------------------------------------------------------
void tn_log_set_level(int level)
{
    tn_log_ctx.level = level;
}

// --------------------------------------------------------------------------------------------------------------
void tn_log_set_quiet(int enable)
{
    tn_log_ctx.quiet = enable ? 1 : 0;
}

// --------------------------------------------------------------------------------------------------------------
void tn_log_log(int level, const char *func, const char *file, int line, uint64_t thread_id, const char *fmt, ...)
{
    if (!tn_log_ready) tn_log_setup();

    if (level < tn_log_ctx.level) return;

    tn_log_lock();

    time_t tstamp = time(NULL);
    struct tm *local_time = localtime(&tstamp);

    if (!tn_log_ctx.quiet) {
        va_list args;
        char time_buf[32];
        time_buf[strftime(time_buf, sizeof(time_buf), "%H:%M:%S", local_time)] = '\0';

        if (tn_log_ctx.color) {
            fprintf(stderr, "%s %s%-5s\033[0m \033[90m%llu:%s:%d - %s: \033[0m ", time_buf, level_colors[level], level_names[level], thread_id, file, line, func);
        } else {
            fprintf(stderr, "%s %-5s %llu:%s:%d - %s: ", time_buf, level_names[level], thread_id, file, line, func);
        }

        va_start(args, fmt);
        vfprintf(stderr, fmt, args);
        va_end(args);
        fprintf(stderr, "\n");
        fflush(stderr);
    }

    if (tn_log_ctx.fp) {
        va_list args;
        char time_buf[64];
        time_buf[strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", local_time)] = '\0';
        fprintf(tn_log_ctx.fp, "%s %-5s %llu:%s:%d - %s: ", time_buf, level_names[level], thread_id, file, line, func);
        va_start(args, fmt);
        vfprintf(tn_log_ctx.fp, fmt, args);
        va_end(args);
        fprintf(tn_log_ctx.fp, "\n");
        fflush(tn_log_ctx.fp);
    }

    tn_log_unlock();
}
