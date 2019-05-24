#ifndef TN_TEST_HARNESS_H
#define TN_TEST_HARNESS_H

#include "aws/testing/aws_test_harness.h"
#include "aws/common/common.h"

#define TN_TEST_HARNESS
#define TN_TEST_CASE AWS_TEST_CASE
#define TN_TEST_CASE_PARAMS struct aws_allocator *allocator, void *ctx

#define TN_TEST_CASE_BEGIN(fn) static int fn (TN_TEST_CASE_PARAMS) \
{ \
	(void)ctx; \
	(void)allocator; \


#include "tn/error.h"



#endif