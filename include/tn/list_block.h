#ifndef TN_LIST_BLOCK_H
#define TN_LIST_BLOCK_H

#include <stdint.h>
#include <stddef.h>

typedef struct tn_list_block_s {
	void *priv_impl;
	void *priv_data;
	uint64_t capacity;
	uint64_t item_size;
} tn_list_block_t;


int tn_list_block_setup(tn_list_block_t *list, uint64_t capacity, uint64_t item_size);
void tn_list_block_cleanup(tn_list_block_t *list);
int tn_list_block_push_back(tn_list_block_t *list, void **item, uint64_t *out_index);
int tn_list_block_pop_back(tn_list_block_t *list, void **item);
int tn_list_block_count(tn_list_block_t *list, uint64_t *out_count);
int tn_list_block_clear(tn_list_block_t *list);
int tn_list_block_get(tn_list_block_t *list, uint64_t index, void **out_item);
int tn_list_block_remove(tn_list_block_t *list, uint64_t index);


#endif