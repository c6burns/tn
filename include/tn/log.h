#ifndef TN_LOG_H
#define TN_LOG_H

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "tn/config.h"
#include "tn/thread.h"

#if _MSC_VER
#    define __FILENAME__ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
#else
#    define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#endif

#define TN_LOG_FILENAME "hubbub.log"
//#define TN_LOG_DISABLE
#define TN_LOG_USE_COLOR
#define TN_THREAD_ID tn_thread_id()

typedef void (*tn_log_lock_fn)(void *udata, int lock);

typedef enum tn_log_level_e {
    TN_LOG_LEVEL_TRACE,
    TN_LOG_LEVEL_DEBUG,
    TN_LOG_LEVEL_INFO,
    TN_LOG_LEVEL_WARN,
    TN_LOG_LEVEL_ERROR,
    TN_LOG_LEVEL_FATAL,
} tn_log_level_t;

#ifdef TN_LOG_DISABLE
#    define tn_log_uv_error(...)

#    define tn_log(...)
#    define tn_log_trace(...)
#    define tn_log_debug(...)
#    define tn_log_info(...)
#    define tn_log_warn(...)
#    define tn_log_error(...)
#    define tn_log_fatal(...)

#    define tn_log_enable(b)
#    define tn_log_disable(b)
#    define tn_log_level(l)
#    define tn_log_file(f)
#else
#    define tn_log_uv_error(code) tn_log_error("uv error code: %d -- %s -- %s", code, uv_err_name(code), uv_strerror(code))
#    define tn_log(...) tn_log_log(TN_LOG_LEVEL_DEBUG, __FUNCTION__, __FILENAME__, __LINE__, TN_THREAD_ID, __VA_ARGS__)
#    define tn_log_trace(...) tn_log_log(TN_LOG_LEVEL_TRACE, __FUNCTION__, __FILENAME__, __LINE__, TN_THREAD_ID, __VA_ARGS__)
#    define tn_log_debug(...) tn_log_log(TN_LOG_LEVEL_DEBUG, __FUNCTION__, __FILENAME__, __LINE__, TN_THREAD_ID, __VA_ARGS__)
#    define tn_log_info(...) tn_log_log(TN_LOG_LEVEL_INFO, __FUNCTION__, __FILENAME__, __LINE__, TN_THREAD_ID, __VA_ARGS__)
#    define tn_log_warn(...) tn_log_log(TN_LOG_LEVEL_WARN, __FUNCTION__, __FILENAME__, __LINE__, TN_THREAD_ID, __VA_ARGS__)
#    define tn_log_warning(...) tn_log_log(TN_LOG_LEVEL_WARN, __FUNCTION__, __FILENAME__, __LINE__, TN_THREAD_ID, __VA_ARGS__)
#    define tn_log_error(...) tn_log_log(TN_LOG_LEVEL_ERROR, __FUNCTION__, __FILENAME__, __LINE__, TN_THREAD_ID, __VA_ARGS__)
#    define tn_log_fatal(...) tn_log_log(TN_LOG_LEVEL_FATAL, __FUNCTION__, __FILENAME__, __LINE__, TN_THREAD_ID, __VA_ARGS__)

#    define tn_log_enable(b) tn_log_set_quiet(0);
#    define tn_log_disable(b) tn_log_set_quiet(1);
#    define tn_log_level(l) tn_log_set_level(l);
#    define tn_log_file(f) tn_log_set_fp(f);
#endif

//#ifndef TN_BUILD_DEBUG
//#	undef tn_log_trace
//#	define tn_log_trace(...) ((void)0)
//#endif

void tn_log_setup(void);
void tn_log_cleanup(void);

// TODO: udata and locking should probably be private
void tn_log_color(int enable);
void tn_log_set_udata(void *udata);
void tn_log_set_lock(tn_log_lock_fn fn);
void tn_log_set_fp(FILE *fp);
void tn_log_set_level(int level);
void tn_log_set_quiet(int enable);

void tn_log_log(int level, const char *func, const char *file, int line, uint64_t thread_id, const char *fmt, ...);

#endif
