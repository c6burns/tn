#ifndef TN_TEST_HARNESS_H
#define TN_TEST_HARNESS_H

#include "aws/common/common.h"
#include "aws/testing/aws_test_harness.h"

#define TN_TEST_HARNESS
#define TN_TEST_CASE(fn) AWS_TEST_CASE(ctest_ ## fn, fn)
#define TN_TEST_CASE_FIXTURE(fn, bf, af, ctx) AWS_TEST_CASE_FIXTURE(ctest_ ## fn, bf, fn, af, ctx)
#define TN_TEST_CASE_PARAMS struct aws_allocator *allocator, void *ctx

#define TN_TEST_CASE_BEGIN(fn)         \
    static int fn(TN_TEST_CASE_PARAMS) \
    {                                  \
        (void)ctx;                     \
        (void)allocator;

#define TN_TEST_FIXTURE_DECL(fn)        \
    static void fn(TN_TEST_CASE_PARAMS) \
    {                                   \
        (void)ctx;                      \
        (void)allocator;

#include "tn/error.h"

#endif
