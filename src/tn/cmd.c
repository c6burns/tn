#include "tn/cmd.h"

#include "tn/error.h"
#include "tn/allocator.h"
#include "tn/buffer.h"


// --------------------------------------------------------------------------------------------------------------
int tn_cmd_list_setup(tn_cmd_list_t *list, uint64_t capacity)
{
	TN_ASSERT(list);
	TN_ASSERT(capacity > 0);

	TN_GUARD(tn_queue_spsc_setup(&list->tn_cmds_free, capacity));
	TN_GUARD(tn_queue_spsc_setup(&list->tn_cmds_ready, capacity));
	list->capacity = tn_queue_spsc_capacity(&list->tn_cmds_ready);
	TN_GUARD_NULL(list->tn_cmds = TN_MEM_ACQUIRE(list->capacity * sizeof(*list->tn_cmds)));

	tn_cmd_base_t *evt_base = NULL;
	for (uint32_t i = 0; i < list->capacity; i++) {
		evt_base = &list->tn_cmds[i];
		evt_base->id = i;
		TN_GUARD(tn_queue_spsc_push(&list->tn_cmds_free, evt_base));
	}

	return TN_SUCCESS;
}

// --------------------------------------------------------------------------------------------------------------
void tn_cmd_list_cleanup(tn_cmd_list_t *list)
{
	TN_ASSERT(list);
	tn_queue_spsc_cleanup(&list->tn_cmds_free);
	tn_queue_spsc_cleanup(&list->tn_cmds_ready);
	TN_MEM_RELEASE(list->tn_cmds);
}

// --------------------------------------------------------------------------------------------------------------
int tn_cmd_list_reset(tn_cmd_list_t *list)
{
	TN_ASSERT(list);

	tn_queue_spsc_cleanup(&list->tn_cmds_free);
	tn_queue_spsc_cleanup(&list->tn_cmds_ready);

	TN_GUARD(tn_queue_spsc_setup(&list->tn_cmds_free, list->capacity));
	TN_GUARD(tn_queue_spsc_setup(&list->tn_cmds_ready, list->capacity));

	tn_cmd_base_t *evt_base = NULL;
	for (uint32_t i = 0; i < list->capacity; i++) {
		evt_base = &list->tn_cmds[i];
		evt_base->id = i;
		TN_GUARD(tn_queue_spsc_push(&list->tn_cmds_free, evt_base));
	}

	return TN_SUCCESS;
}

// --------------------------------------------------------------------------------------------------------------
int tn_cmd_list_free_peek(tn_cmd_list_t *list, tn_cmd_base_t **out_evt)
{
	TN_ASSERT(list);
	TN_ASSERT(out_evt);

	*out_evt = NULL;
	TN_GUARD(tn_queue_spsc_peek(&list->tn_cmds_free, (void **)out_evt));

	return TN_SUCCESS;
}

// --------------------------------------------------------------------------------------------------------------
int tn_cmd_list_free_pop(tn_cmd_list_t *list)
{
	TN_ASSERT(list);

	TN_GUARD(tn_queue_spsc_pop(&list->tn_cmds_free));

	return TN_SUCCESS;
}

// --------------------------------------------------------------------------------------------------------------
void tn_cmd_list_free_pop_cached(tn_cmd_list_t *list)
{
	tn_queue_spsc_pop_cached(&list->tn_cmds_free);
}

// --------------------------------------------------------------------------------------------------------------
int tn_cmd_list_free_pop_back(tn_cmd_list_t *list, tn_cmd_base_t **out_evt)
{
	TN_ASSERT(list);
	TN_ASSERT(out_evt);

	return tn_queue_spsc_pop_back(&list->tn_cmds_free, (void **)out_evt);
}

// --------------------------------------------------------------------------------------------------------------
int tn_cmd_list_free_pop_open(tn_cmd_list_t *list, tn_cmd_client_open_t **out_evt)
{
	TN_ASSERT(list);
	TN_ASSERT(out_evt);

	*out_evt = NULL;
	TN_GUARD(tn_queue_spsc_pop_back(&list->tn_cmds_free, (void **)out_evt));
	(*out_evt)->type = TN_CMD_CLIENT_OPEN;

	return TN_SUCCESS;
}

// --------------------------------------------------------------------------------------------------------------
int tn_cmd_list_free_pop_close(tn_cmd_list_t *list, tn_cmd_client_close_t **out_evt)
{
	TN_ASSERT(list);
	TN_ASSERT(out_evt);

	*out_evt = NULL;
	TN_GUARD(tn_queue_spsc_pop_back(&list->tn_cmds_free, (void **)out_evt));
	(*out_evt)->type = TN_CMD_CLIENT_CLOSE;

	return TN_SUCCESS;
}

// --------------------------------------------------------------------------------------------------------------
int tn_cmd_list_free_pop_send(tn_cmd_list_t *list, tn_cmd_client_send_t **out_evt)
{
	TN_ASSERT(list);
	TN_ASSERT(out_evt);

	*out_evt = NULL;
	TN_GUARD(tn_queue_spsc_pop_back(&list->tn_cmds_free, (void **)out_evt));
	(*out_evt)->type = TN_CMD_CLIENT_SEND;
	(*out_evt)->priv = NULL;

	return TN_SUCCESS;
}

// --------------------------------------------------------------------------------------------------------------
int tn_cmd_list_free_push(tn_cmd_list_t *list, void *evt)
{
	TN_ASSERT(list);
	TN_ASSERT(evt);

	return tn_queue_spsc_push(&list->tn_cmds_free, evt);
}

// --------------------------------------------------------------------------------------------------------------
int tn_cmd_list_ready_peek(tn_cmd_list_t *list, tn_cmd_base_t **out_evt)
{
	TN_ASSERT(list);
	TN_ASSERT(out_evt);

	*out_evt = NULL;
	TN_GUARD(tn_queue_spsc_peek(&list->tn_cmds_ready, (void **)out_evt));

	return TN_SUCCESS;
}

// --------------------------------------------------------------------------------------------------------------
int tn_cmd_list_ready_pop(tn_cmd_list_t *list)
{
	TN_ASSERT(list);

	TN_GUARD(tn_queue_spsc_pop(&list->tn_cmds_ready));

	return TN_SUCCESS;
}

// --------------------------------------------------------------------------------------------------------------
void tn_cmd_list_ready_pop_cached(tn_cmd_list_t *list)
{
	tn_queue_spsc_pop_cached(&list->tn_cmds_ready);
}

// --------------------------------------------------------------------------------------------------------------
int tn_cmd_list_ready_pop_back(tn_cmd_list_t *list, tn_cmd_base_t **out_evt)
{
	TN_ASSERT(list);
	TN_ASSERT(out_evt);

	*out_evt = NULL;
	TN_GUARD(tn_queue_spsc_pop_back(&list->tn_cmds_ready, (void **)out_evt));

	return TN_SUCCESS;
}

// --------------------------------------------------------------------------------------------------------------
int tn_cmd_list_ready_pop_all(tn_cmd_list_t *list, tn_cmd_base_t **out_evt, uint64_t *out_count)
{
	TN_ASSERT(list);
	TN_ASSERT(out_evt);
	TN_ASSERT(out_count);
	TN_ASSERT(*out_count > 0);

	TN_GUARD(tn_queue_spsc_pop_all(&list->tn_cmds_ready, (void **)out_evt, out_count));

	return TN_SUCCESS;
}

// --------------------------------------------------------------------------------------------------------------
int tn_cmd_list_ready_push(tn_cmd_list_t *list, void *evt)
{
	TN_ASSERT(list);
	TN_ASSERT(evt);

	return tn_queue_spsc_push(&list->tn_cmds_ready, evt);
}