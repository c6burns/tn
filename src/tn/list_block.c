#include "tn/list_block.h"

#if (tn_list_block_IMPL == tn_list_block_IMPL_AWS_ARRAY_LIST)

#include "aws/common/array_list.h"

#include "tn/error.h"
#include "tn/allocator.h"


// --------------------------------------------------------------------------------------------------------------
int tn_list_block_setup(tn_list_block_t *list, uint64_t capacity, uint64_t item_size)
{
	TN_GUARD_NULL(list);
	memset(list, 0, sizeof(*list));
	
	const uint64_t priv_size = capacity * sizeof(void *);
	TN_GUARD_NULL_CLEANUP(list->priv_data = TN_MEM_ACQUIRE(priv_size));
	memset(list->priv_data, 0, priv_size);

	TN_GUARD_NULL_CLEANUP(list->priv_impl = TN_MEM_ACQUIRE(sizeof(struct aws_array_list)));
	aws_array_list_init_static(list->priv_impl, list->priv_data, capacity, item_size);
	aws_array_list_clear(list->priv_impl);

	list->capacity = capacity;
	list->item_size = item_size;

	return TN_SUCCESS;
	
cleanup:
	TN_MEM_RELEASE(list->priv_impl);
	TN_MEM_RELEASE(list->priv_data);

	return TN_ERROR;
}

// --------------------------------------------------------------------------------------------------------------
void tn_list_block_cleanup(tn_list_block_t *list)
{
	if (!list) return;

	if (list->priv_impl) {
		aws_array_list_clean_up(list->priv_impl);
	}

	TN_MEM_RELEASE(list->priv_impl);
	TN_MEM_RELEASE(list->priv_data);
}

// --------------------------------------------------------------------------------------------------------------
int tn_list_block_push_back(tn_list_block_t *list, void **item, uint64_t *out_index)
{
	TN_ASSERT(list);
	TN_ASSERT(item);
	TN_ASSERT(*item);

	if (out_index) *out_index = aws_array_list_length(list->priv_impl);
	TN_GUARD(aws_array_list_push_back(list->priv_impl, item));

	return TN_SUCCESS;
}

// --------------------------------------------------------------------------------------------------------------
int tn_list_block_pop_back(tn_list_block_t *list, void **item)
{
	TN_ASSERT(list);
	TN_ASSERT(item);

	TN_GUARD(aws_array_list_back(list->priv_impl, item));
	TN_GUARD(aws_array_list_pop_back(list->priv_impl));
	//TN_ASSERT(*item);
	return TN_SUCCESS;
}

// --------------------------------------------------------------------------------------------------------------
int tn_list_block_count(tn_list_block_t *list, uint64_t *out_count)
{
	TN_GUARD_NULL(list);
	TN_GUARD_NULL(out_count);
	*out_count = aws_array_list_length(list->priv_impl);
	return TN_SUCCESS;
}

// --------------------------------------------------------------------------------------------------------------
int tn_list_block_clear(tn_list_block_t *list)
{
	TN_GUARD_NULL(list);
	aws_array_list_clear(list->priv_impl);
	return TN_SUCCESS;
}

// --------------------------------------------------------------------------------------------------------------
int tn_list_block_get(tn_list_block_t *list, uint64_t index, void **out_item)
{
	TN_GUARD_NULL(list);
	TN_GUARD_NULL(out_item);
	return aws_array_list_get_at(list->priv_impl, out_item, index);
}

// --------------------------------------------------------------------------------------------------------------
int tn_list_block_remove(tn_list_block_t *list, uint64_t index)
{
	TN_GUARD_NULL(list);

	const size_t last_idx = aws_array_list_length(list->priv_impl) - 1;
	aws_array_list_swap(list->priv_impl, index, last_idx);
	return aws_array_list_pop_back(list->priv_impl);
}

#endif