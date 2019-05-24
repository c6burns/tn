#include "tn/allocator.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "tn/error.h"


// private ------------------------------------------------------------------------------------------------------
void *tn_aws_default_acquire(struct aws_allocator *allocator, size_t size)
{
	return malloc(size);
}

// private ------------------------------------------------------------------------------------------------------
void tn_aws_default_release(struct aws_allocator *allocator, void *ptr)
{
	if (!ptr) return;
	free(ptr);
}

aws_allocator_t tn_aws_default_allocator = {
	.mem_acquire = tn_aws_default_acquire,
	.mem_release = tn_aws_default_release,
	.mem_realloc = NULL,
	.impl = NULL,
};


// private ------------------------------------------------------------------------------------------------------
void *tn_allocator_default_acquire(const struct tn_allocator_s *allocator, size_t sz)
{
	uint8_t *mem;
	if (!(mem = aws_mem_acquire(&tn_aws_default_allocator, sz))) return NULL;

	return (void *)mem;
}

// private ------------------------------------------------------------------------------------------------------
void tn_allocator_default_release(const struct tn_allocator_s *allocator, void *ptr)
{
	if (!ptr) return;

	aws_mem_release(&tn_aws_default_allocator, ptr);
}

// private ------------------------------------------------------------------------------------------------------
void tn_allocator_default_release_ptr(const struct tn_allocator_s *allocator, void **ptr)
{
	if (!ptr) return;
	if (!*ptr) return;

	aws_mem_release(&tn_aws_default_allocator, *ptr);

	*ptr = NULL;
}

tn_allocator_t tn_default_allocator = {
	.acquire_fn = tn_allocator_default_acquire,
	.release_fn = tn_allocator_default_release,
	.release_ptr_fn = tn_allocator_default_release_ptr,
	.config = {
		.priv = NULL,
	},
	.priv = &tn_aws_default_allocator,
};


// --------------------------------------------------------------------------------------------------------------
tn_allocator_t *tn_allocator_new(void)
{
	tn_allocator_t *allocator;
	if (!(allocator = aws_mem_acquire(&tn_aws_default_allocator, sizeof(tn_allocator_t)))) return NULL;
	return allocator;
}

// --------------------------------------------------------------------------------------------------------------
void tn_allocator_delete(tn_allocator_t **ptr_allocator)
{
	if (!ptr_allocator) return;
	if (!*ptr_allocator) return;
	aws_mem_release(&tn_aws_default_allocator, *ptr_allocator);
	*ptr_allocator = NULL;
}

// --------------------------------------------------------------------------------------------------------------
int tn_allocator_setup(tn_allocator_t *allocator, const tn_allocator_config_t *config, tn_allocator_aquire_fn acquire_fn, tn_allocator_release_fn release_fn)
{
	if (!allocator) return TN_ERROR;

	if (config) {
		memcpy(&allocator->config, config, sizeof(tn_allocator_config_t));
	}

	if (acquire_fn) allocator->acquire_fn = acquire_fn;
	else allocator->acquire_fn = tn_allocator_default_acquire;

	if (release_fn) allocator->release_fn = release_fn;
	else allocator->release_fn = tn_allocator_default_release;

	return TN_SUCCESS;
}

// --------------------------------------------------------------------------------------------------------------
int tn_allocator_cleanup(tn_allocator_t *allocator)
{
	return TN_SUCCESS;
}

// --------------------------------------------------------------------------------------------------------------
void *tn_allocator_acquire(const tn_allocator_t *allocator, size_t sz)
{
	if (!allocator) return NULL;
	if (!allocator->acquire_fn) return NULL;

	return allocator->acquire_fn(allocator, sz);
}

// --------------------------------------------------------------------------------------------------------------
void tn_allocator_release(const tn_allocator_t *allocator, void *ptr)
{
	if (!allocator) return;
	if (!allocator->release_fn) return;

	allocator->release_fn(allocator, ptr);
}

// --------------------------------------------------------------------------------------------------------------
void tn_allocator_release_ptr(const tn_allocator_t *allocator, void **ptr)
{
	if (!allocator) return;
	if (!allocator->release_fn) return;

	allocator->release_ptr_fn(allocator, ptr);
}