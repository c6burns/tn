#include "tn/mutex.h"

#include "aws/common/mutex.h"
#include "tn/error.h"

// --------------------------------------------------------------------------------------------------------------
int tn_mutex_setup(tn_mutex_t *mtx)
{
    TN_ASSERT(mtx);
    return aws_mutex_init((struct aws_mutex *)mtx);
}

// --------------------------------------------------------------------------------------------------------------
int tn_mutex_lock(tn_mutex_t *mtx)
{
    TN_ASSERT(mtx);
    return aws_mutex_lock((struct aws_mutex *)mtx);
}

// --------------------------------------------------------------------------------------------------------------
int tn_mutex_unlock(tn_mutex_t *mtx)
{
    TN_ASSERT(mtx);
    return aws_mutex_unlock((struct aws_mutex *)mtx);
}

// --------------------------------------------------------------------------------------------------------------
void tn_mutex_cleanup(tn_mutex_t *mtx)
{
    TN_ASSERT(mtx);
    aws_mutex_clean_up((struct aws_mutex *)mtx);
}
