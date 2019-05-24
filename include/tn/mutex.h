#ifndef TN_MUTEX_H
#define TN_MUTEX_H

#ifndef _WIN32
#	include <pthread.h>
#endif


typedef struct tn_mutex_s {
#ifdef _WIN32
	void *mutex_handle;
#else
	pthread_mutex_t mutex_handle;
#endif
} tn_mutex_t;


int tn_mutex_setup(tn_mutex_t *mtx);
int tn_mutex_lock(tn_mutex_t *mtx);
int tn_mutex_unlock(tn_mutex_t *mtx);
void tn_mutex_cleanup(tn_mutex_t *mtx);


#endif