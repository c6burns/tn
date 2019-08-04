#include "tn/time.h"

#include "aws/common/clock.h"

// --------------------------------------------------------------------------------------------------------------
uint64_t tn_tstamp()
{
    uint64_t rv;
    aws_sys_clock_get_ticks(&rv);
    return rv;
}

// --------------------------------------------------------------------------------------------------------------
uint64_t tn_tstamp_convert(uint64_t tstamp, uint32_t from, uint32_t to)
{
    return aws_timestamp_convert(tstamp, from, to, NULL);
}
