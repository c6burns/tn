#ifndef TN_ERROR_H
#define TN_ERROR_H

#include <assert.h>
#include <errno.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>

/* general return status */
#define TN_SUCCESS 0
#define TN_ERROR -1
#define TN_ERROR_AGAIN EAGAIN
#define TN_ERROR_NOMEM ENOMEM
#define TN_ERROR_INVAL EINVAL

/* queue return status */
#define TN_QUEUE_EMPTY -100
#define TN_QUEUE_FULL -101
#define TN_QUEUE_AGAIN -102

/* event return status */
#define TN_EVENT_ERROR -200
#define TN_EVENT_NOCHAN -201
#define TN_EVENT_NOSPAN -202
#define TN_EVENT_SPANREAD -203

/* send / IO write return status */
#define TN_SEND_ERROR -300
#define TN_SEND_NOREQ -301
#define TN_SEND_NOBUF -302
#define TN_SEND_PUSH -303

#define TN_RECV_ERROR -400
#define TN_RECV_EBUF -401
#define TN_RECV_EVTPOP -402
#define TN_RECV_EVTPUSH -403
#define TN_RECV_E2BIG -404

#define TN_CLIENT_ECONN -500

#define TN_ASSERT(expr) assert(expr)
#define TN_GUARD(expr) \
    if ((expr)) return TN_ERROR
#define TN_GUARD_LOG(expr, msg) \
    if ((expr)) {               \
        tn_log_error(msg);      \
        return TN_ERROR;        \
    }
#define TN_GUARD_NULL(expr) \
    if (!(expr)) return TN_ERROR
#define TN_GUARD_GOTO(lbl, expr) \
    {                            \
        if ((expr)) goto lbl;    \
    }
#define TN_GUARD_CLEANUP(expr) TN_GUARD_GOTO(cleanup, expr)
#define TN_GUARD_NULL_CLEANUP(expr) TN_GUARD_GOTO(cleanup, !(expr))

#ifdef _WIN32
#    define TN_PLATFORM_WINDOWS 1

#    ifdef _WIN64
#    else
#    endif
#elif __APPLE__
#    include <TargetConditionals.h>
#    if TARGET_IPHONE_SIMULATOR
#        define TN_PLATFORM_IOS 1
#    elif TARGET_OS_IPHONE
#        define TN_PLATFORM_IOS 1
#    elif TARGET_OS_MAC
#        define TN_PLATFORM_OSX 1
#    else
#        error "Unknown Apple platform"
#    endif
#elif __ANDROID__
#    define TN_PLATFORM_ANDROID 1
#elif __linux__
#    define TN_PLATFORM_POSIX 1
#elif __unix__ // all unices not caught above
#    define TN_PLATFORM_POSIX 1
#elif defined(_POSIX_VERSION)
#    define TN_PLATFORM_POSIX 1
#else
#    error "Unknown compiler"
#endif

#ifdef TN_EXPORTING
#    if defined(__CYGWIN32__) || defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(_WIN64) || defined(WINAPI_FAMILY)
#        define TN_CONVENTION __cdecl
#        define TN_EXPORT __declspec(dllexport)
#    else
#        define TN_CONVENTION
#        define TN_EXPORT
#    endif
#endif

#endif
