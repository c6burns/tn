#include "tn/event.h"

#include "tn/error.h"
#include "tn/allocator.h"
#include "tn/buffer.h"


// --------------------------------------------------------------------------------------------------------------
int tn_event_list_setup(tn_event_list_t *list, uint64_t capacity)
{
	TN_ASSERT(list);
	TN_ASSERT(capacity > 0);

	TN_GUARD(tn_queue_spsc_setup(&list->tn_events_free, capacity));
	TN_GUARD(tn_queue_spsc_setup(&list->tn_events_ready, capacity));
	list->capacity = tn_queue_spsc_capacity(&list->tn_events_ready);
	TN_GUARD_NULL(list->tn_events = TN_MEM_ACQUIRE(list->capacity * sizeof(*list->tn_events)));

	tn_event_base_t *evt_base = NULL;
	for (uint32_t i = 0; i < list->capacity; i++) {
		evt_base = &list->tn_events[i];
		evt_base->id = i;
		TN_GUARD(tn_queue_spsc_push(&list->tn_events_free, evt_base));
	}

	return TN_SUCCESS;
}

// --------------------------------------------------------------------------------------------------------------
void tn_event_list_cleanup(tn_event_list_t *list)
{
	TN_ASSERT(list);
	tn_queue_spsc_cleanup(&list->tn_events_free);
	tn_queue_spsc_cleanup(&list->tn_events_ready);
	TN_MEM_RELEASE(list->tn_events);
}

// --------------------------------------------------------------------------------------------------------------
int tn_event_list_reset(tn_event_list_t *list)
{
	TN_ASSERT(list);

	tn_queue_spsc_cleanup(&list->tn_events_free);
	tn_queue_spsc_cleanup(&list->tn_events_ready);

	TN_GUARD(tn_queue_spsc_setup(&list->tn_events_free, list->capacity));
	TN_GUARD(tn_queue_spsc_setup(&list->tn_events_ready, list->capacity));

	tn_event_base_t *evt_base = NULL;
	for (uint32_t i = 0; i < list->capacity; i++) {
		evt_base = &list->tn_events[i];
		evt_base->id = i;
		TN_GUARD(tn_queue_spsc_push(&list->tn_events_free, evt_base));
	}

	return TN_SUCCESS;
}

// --------------------------------------------------------------------------------------------------------------
int tn_event_list_free_pop_back(tn_event_list_t *list, tn_event_base_t **out_evt)
{
	TN_ASSERT(list);
	TN_ASSERT(out_evt);

	return tn_queue_spsc_pop_back(&list->tn_events_free, (void **)out_evt);
}

// --------------------------------------------------------------------------------------------------------------
int tn_event_list_free_pop_error(tn_event_list_t *list, tn_event_error_t **out_evt)
{
	TN_ASSERT(list);
	TN_ASSERT(out_evt);

	*out_evt = NULL;
	TN_GUARD(tn_queue_spsc_pop_back(&list->tn_events_free, (void **)out_evt));
	(*out_evt)->type = TN_EVENT_IOERROR;
	
	return TN_SUCCESS;
}

// --------------------------------------------------------------------------------------------------------------
int tn_event_list_free_pop_open(tn_event_list_t *list, tn_event_client_open_t **out_evt)
{
	TN_ASSERT(list);
	TN_ASSERT(out_evt);

	*out_evt = NULL;
	TN_GUARD(tn_queue_spsc_pop_back(&list->tn_events_free, (void **)out_evt));
	(*out_evt)->type = TN_EVENT_CLIENT_OPEN;

	return TN_SUCCESS;
}

// --------------------------------------------------------------------------------------------------------------
int tn_event_list_free_pop_close(tn_event_list_t *list, tn_event_client_close_t **out_evt)
{
	TN_ASSERT(list);
	TN_ASSERT(out_evt);

	*out_evt = NULL;
	TN_GUARD(tn_queue_spsc_pop_back(&list->tn_events_free, (void **)out_evt));
	(*out_evt)->type = TN_EVENT_CLIENT_CLOSE;
	(*out_evt)->tn_buffer = NULL;

	return TN_SUCCESS;
}

// --------------------------------------------------------------------------------------------------------------
int tn_event_list_free_pop_read(tn_event_list_t *list, tn_event_client_read_t **out_evt)
{
	TN_ASSERT(list);
	TN_ASSERT(out_evt);

	*out_evt = NULL;
	TN_GUARD(tn_queue_spsc_pop_back(&list->tn_events_free, (void **)out_evt));
	(*out_evt)->type = TN_EVENT_CLIENT_READ;
	(*out_evt)->priv = NULL;

	return TN_SUCCESS;
}

// --------------------------------------------------------------------------------------------------------------
int tn_event_list_ready_push(tn_event_list_t *list, void *evt)
{
	TN_ASSERT(list);
	TN_ASSERT(evt);

	return tn_queue_spsc_push(&list->tn_events_ready, evt);
}

// --------------------------------------------------------------------------------------------------------------
int tn_event_list_ready_peek(tn_event_list_t *list, tn_event_base_t **out_evt)
{
	TN_ASSERT(list);
	TN_ASSERT(out_evt);

	*out_evt = NULL;
	TN_GUARD(tn_queue_spsc_peek(&list->tn_events_ready, (void **)out_evt));

	return TN_SUCCESS;
}

// --------------------------------------------------------------------------------------------------------------
int tn_event_list_ready_pop(tn_event_list_t *list)
{
	TN_ASSERT(list);

	TN_GUARD(tn_queue_spsc_pop(&list->tn_events_ready));

	return TN_SUCCESS;
}

// --------------------------------------------------------------------------------------------------------------
void tn_event_list_ready_pop_cached(tn_event_list_t *list)
{
	tn_queue_spsc_pop_cached(&list->tn_events_ready);
}

// --------------------------------------------------------------------------------------------------------------
int tn_event_list_ready_pop_back(tn_event_list_t *list, tn_event_base_t **out_evt)
{
	TN_ASSERT(list);
	TN_ASSERT(out_evt);

	*out_evt = NULL;
	TN_GUARD(tn_queue_spsc_pop_back(&list->tn_events_ready, (void **)out_evt));

	return TN_SUCCESS;
}

// --------------------------------------------------------------------------------------------------------------
int tn_event_list_ready_pop_all(tn_event_list_t *list, tn_event_base_t **out_evt, uint64_t *out_count)
{
	TN_ASSERT(list);
	TN_ASSERT(out_evt);
	TN_ASSERT(out_count);
	TN_ASSERT(*out_count > 0);

	TN_GUARD(tn_queue_spsc_pop_all(&list->tn_events_ready, (void **)out_evt, out_count));

	return TN_SUCCESS;
}

// --------------------------------------------------------------------------------------------------------------
int tn_event_list_free_push(tn_event_list_t *list, void *evt)
{
	TN_ASSERT(list);
	TN_ASSERT(evt);

	return tn_queue_spsc_push(&list->tn_events_free, evt);
}
