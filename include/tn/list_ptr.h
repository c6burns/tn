#ifndef TN_LIST_PTR_H
#define TN_LIST_PTR_H

#include <stdint.h>
#include <stddef.h>

typedef struct tn_list_ptr_s {
	uintptr_t *data;
	uint64_t capacity;
	uint64_t index;
} tn_list_ptr_t;


int tn_list_ptr_setup(tn_list_ptr_t *list, size_t capacity);
void tn_list_ptr_cleanup(tn_list_ptr_t *list);
int tn_list_ptr_push_back(tn_list_ptr_t *list, void *item);
int tn_list_ptr_pop_back(tn_list_ptr_t *list, void **item);
uint64_t tn_list_ptr_count(tn_list_ptr_t *list);
uint64_t tn_list_ptr_capacity(tn_list_ptr_t *list);
void tn_list_ptr_clear(tn_list_ptr_t *list);
void *tn_list_ptr_get(tn_list_ptr_t *list, uint64_t index);
int tn_list_ptr_remove(tn_list_ptr_t *list, size_t index);


#endif