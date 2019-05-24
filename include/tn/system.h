#ifndef TN_SYSTEM_H
#define TN_SYSTEM_H

#include <stdint.h>

typedef struct tn_system_s {
	void *priv;
} tn_system_t;


int tn_system_setup(tn_system_t *system);
void tn_system_cleanup(tn_system_t *system);
uint32_t tn_system_cpu_count(tn_system_t *system);


#endif
