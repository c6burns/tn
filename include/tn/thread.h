#ifndef TN_THREAD_H
#define TN_THREAD_H

#include <stdint.h>

#ifndef _WIN32
#	include <pthread.h>
#endif

#include "tn/time.h"


#define TN_TIME_MS_PER_S 1000
#define TN_TIME_US_PER_S 1000000
#define TN_TIME_NS_PER_S 1000000000

#define TN_TIME_US_PER_MS 1000
#define TN_TIME_NS_PER_MS 1000000

#define TN_TIME_NS_PER_US 1000

#define TN_THREAD_SLEEP_S(s) aws_thread_current_sleep(ms * TN_TIME_NS_PER_S)
#define TN_THREAD_SLEEP_MS(ms) aws_thread_current_sleep(ms * TN_TIME_NS_PER_MS)

#define TN_WORKERS_MAX 128


enum tn_thread_state {
	TN_THREAD_NEW,
	TN_THREAD_READY,
	TN_THREAD_JOINABLE,
	TN_THREAD_JOINED,
};

typedef struct tn_thread_s {
	struct aws_allocator *allocator;
	enum tn_thread_state detach_state;
#ifdef _WIN32
	void *thread_handle;
	unsigned long thread_id;
#else
	pthread_t thread_id;
#endif
} tn_thread_t;



void tn_thread_sleep(uint64_t ns);
#define tn_thread_sleep_s(tstamp) tn_thread_sleep(tn_tstamp_convert(tstamp, TN_TSTAMP_S, TN_TSTAMP_NS))
#define tn_thread_sleep_ms(tstamp) tn_thread_sleep(tn_tstamp_convert(tstamp, TN_TSTAMP_MS, TN_TSTAMP_NS))

uint64_t tn_thread_id(void);
int tn_thread_setup(tn_thread_t *thread);
int tn_thread_launch(tn_thread_t *thread, void(*func)(void *arg), void *arg);
uint64_t tn_thread_get_id(tn_thread_t *thread);
enum tn_thread_state tn_thread_get_state(tn_thread_t *thread);
int tn_thread_join(tn_thread_t *thread);
void tn_thread_cleanup(tn_thread_t *thread);

uint32_t tn_thread_workers(void);
int tn_thread_set_workers(uint32_t count);



#endif
