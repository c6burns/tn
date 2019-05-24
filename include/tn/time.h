#ifndef TN_TIME_H
#define TN_TIME_H


#include <stdint.h>


#define TN_TSTAMP_S 1
#define TN_TSTAMP_MS 1000
#define TN_TSTAMP_US 1000000
#define TN_TSTAMP_NS 1000000000


uint64_t tn_tstamp(void);
uint64_t tn_tstamp_convert(uint64_t tstamp, uint32_t from, uint32_t to);

#endif