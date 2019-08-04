#include "tn/uuid.h"

#include "aws/common/uuid.h"

#include "tn/allocator.h"
#include "tn/error.h"
#include "tn/log.h"

// --------------------------------------------------------------------------------------------------------------
tn_uuid_t *tn_uuid_new(void)
{
    return TN_MEM_ACQUIRE(sizeof(tn_uuid_t));
}

// --------------------------------------------------------------------------------------------------------------
void tn_uuid_delete(tn_uuid_t **ptr_uuid)
{
    TN_MEM_RELEASE_PTR((void **)ptr_uuid);
}

// --------------------------------------------------------------------------------------------------------------
int tn_uuid_generate(tn_uuid_t *uuid)
{
    int ret;
    if (!uuid) return TN_ERROR;
    if ((ret = aws_uuid_init((struct aws_uuid *)uuid))) tn_log_error("failed to generate uuid");
    return ret;
}

// --------------------------------------------------------------------------------------------------------------
int tn_uuid_clear(tn_uuid_t *uuid)
{
    if (!uuid) return TN_ERROR;
    memset(uuid, 0, sizeof(tn_uuid_t));
    return TN_SUCCESS;
}

// --------------------------------------------------------------------------------------------------------------
int tn_uuid_compare(tn_uuid_t *uuid1, tn_uuid_t *uuid2)
{
    if (!uuid1 && !uuid2) return TN_SUCCESS;
    if (!uuid1 || !uuid2) return TN_ERROR;

    return aws_uuid_equals((struct aws_uuid *)uuid1, (struct aws_uuid *)uuid2);
}
