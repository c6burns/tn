#include "tn/list_ptr.h"

#include "tn/allocator.h"
#include "tn/error.h"

// --------------------------------------------------------------------------------------------------------------
int tn_list_ptr_setup(tn_list_ptr_t *list, size_t capacity)
{
    TN_GUARD_NULL(list);
    memset(list, 0, sizeof(*list));

    list->capacity = capacity;
    TN_GUARD_NULL(list->data = TN_MEM_ACQUIRE(list->capacity * sizeof(*list->data)));

    return TN_SUCCESS;
}

// --------------------------------------------------------------------------------------------------------------
void tn_list_ptr_cleanup(tn_list_ptr_t *list)
{
    if (!list) return;
    TN_MEM_RELEASE(list->data);
}

// --------------------------------------------------------------------------------------------------------------
int tn_list_ptr_push_back(tn_list_ptr_t *list, void *item)
{
    TN_ASSERT(list);
    TN_ASSERT(list->data);
    TN_ASSERT(item);

    TN_GUARD(list->index >= list->capacity);
    list->data[list->index++] = (uintptr_t)item;

    return TN_SUCCESS;
}

// --------------------------------------------------------------------------------------------------------------
int tn_list_ptr_pop_back(tn_list_ptr_t *list, void **item)
{
    TN_ASSERT(list);
    TN_ASSERT(list->data);
    TN_ASSERT(item);

    TN_GUARD(list->index == 0);
    *item = (void *)list->data[--list->index];

    return TN_SUCCESS;
}

// --------------------------------------------------------------------------------------------------------------
uint64_t tn_list_ptr_count(tn_list_ptr_t *list)
{
    TN_ASSERT(list);
    return list->index;
}

// --------------------------------------------------------------------------------------------------------------
uint64_t tn_list_ptr_capacity(tn_list_ptr_t *list)
{
    TN_ASSERT(list);
    return list->capacity;
}

// --------------------------------------------------------------------------------------------------------------
void tn_list_ptr_clear(tn_list_ptr_t *list)
{
    TN_ASSERT(list);
    list->index = 0;
}

// --------------------------------------------------------------------------------------------------------------
void *tn_list_ptr_get(tn_list_ptr_t *list, uint64_t index)
{
    TN_ASSERT(list);
    TN_ASSERT(list->data[index]);
    return (void *)list->data[index];
}

// --------------------------------------------------------------------------------------------------------------
void tn_list_ptr_swap(tn_list_ptr_t *list, uint64_t index1, uint64_t index2)
{
    TN_ASSERT(list);

    uintptr_t tmp = list->data[index1];
    list->data[index1] = list->data[index2];
    list->data[index2] = tmp;
}

// --------------------------------------------------------------------------------------------------------------
int tn_list_ptr_remove(tn_list_ptr_t *list, size_t index)
{
    TN_ASSERT(list);

    TN_GUARD(list->index == 0);

    list->data[index] = list->data[--list->index];

    return TN_SUCCESS;
}
