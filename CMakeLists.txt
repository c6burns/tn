cmake_minimum_required(VERSION 3.1)
project(tn LANGUAGES C)

set(TN_DEPS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/deps)
set(TN_LIBRARIES "")
set(TN_INCLUDE_DIRS "")

if(NOT EXISTS ${TN_DEPS_DIR})
	file(MAKE_DIRECTORY ${TN_DEPS_DIR})
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	list(APPEND TN_TARGET_COMPILE_DEFS -DTN_CMAKE_BUILD_DEBUG)
endif()

if(NOT WIN32)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC -std=c99")
	if(NOT APPLE)
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_GNU_SOURCE")
		set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pie")
	endif()
else()
	set(CompilerFlags
		CMAKE_CXX_FLAGS
		CMAKE_CXX_FLAGS_DEBUG
		CMAKE_CXX_FLAGS_RELEASE
		CMAKE_C_FLAGS
		CMAKE_C_FLAGS_DEBUG
		CMAKE_C_FLAGS_RELEASE
	)
	foreach(CompilerFlag ${CompilerFlags})
		string(REPLACE "/MD" "/MT" ${CompilerFlag} "${${CompilerFlag}}")
	endforeach()

	if(MSVC)
		add_custom_target(CMake SOURCES CMakeLists.txt)
		list(APPEND TN_TARGET_COMPILE_DEFS -D_CRT_SECURE_NO_WARNINGS)
	endif()
endif()

#
# argparse dependency
#list(APPEND TN_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/deps/argparse)
#include_directories(${CMAKE_CURRENT_SOURCE_DIR}/deps/argparse)
#add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/deps/argparse)
#list(APPEND TN_LIBRARIES argparse)

#
# aws-c-common dependency
set(BUILD_TESTING ON CACHE BOOL "" FORCE)
set(STATIC_CRT ON CACHE BOOL "" FORCE)
mark_as_advanced(FORCE BUILD_TESTING STATIC_CRT)

if(NOT EXISTS ${TN_DEPS_DIR}/aws-c-common)
	execute_process(COMMAND git clone https://github.com/c6burns/aws-c-common WORKING_DIRECTORY ${TN_DEPS_DIR})
endif()

add_custom_target(aws-c-common_update
	COMMAND git checkout master
	COMMAND git pull
	WORKING_DIRECTORY ${TN_DEPS_DIR}/aws-c-common
)

set(ORIG_CMAKE_PROJECT_NAME ${CMAKE_PROJECT_NAME})
set(CMAKE_PROJECT_NAME aws-c-common)
list(APPEND TN_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/deps/aws-c-common/include)
list(APPEND TN_INCLUDE_DIRS ${CMAKE_CURRENT_BINARY_DIR}/deps/aws-c-common/generated/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/deps/aws-c-common/include)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/deps/aws-c-common/generated/include)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/deps/aws-c-common)
add_dependencies(aws-c-common aws-c-common_update)
list(APPEND TN_LIBRARIES aws-c-common)
set(CMAKE_PROJECT_NAME ${ORIG_CMAKE_PROJECT_NAME})

#
# libuv dependency
set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
set(libuv_buildtests OFF CACHE BOOL "" FORCE)
mark_as_advanced(FORCE BUILD_TESTING libuv_buildtests)

if(NOT EXISTS ${TN_DEPS_DIR}/libuv)
	execute_process(COMMAND git clone https://github.com/c6burns/libuv WORKING_DIRECTORY ${TN_DEPS_DIR})
endif()

add_custom_target(libuv_update
	COMMAND git checkout v1.x
	COMMAND git pull
	WORKING_DIRECTORY ${TN_DEPS_DIR}/libuv
)

list(APPEND TN_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/deps/libuv/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/deps/libuv/include)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/deps/libuv)
add_dependencies(uv_a libuv_update)
list(APPEND TN_LIBRARIES uv_a)

#
# build tn library
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/CMake)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/deps/aws-c-common/cmake)
include(tnTesting)
enable_testing()

set(TN_SRC ${CMAKE_CURRENT_SOURCE_DIR})
list(APPEND TN_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include)
list(APPEND TN_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/src/tn)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/src/tn)
set(TN_LIBRARY_SRCS
	src/tn/log.c
	include/tn/log.h
	src/tn/system.c
	include/tn/system.h
	src/tn/endpoint.c
	include/tn/endpoint.h
	src/tn/uuid.c
	include/tn/uuid.h
	src/tn/time.c
	include/tn/time.h
	src/tn/list_ptr.c
	include/tn/list_ptr.h
	src/tn/list_block.c
	include/tn/list_block.h
	src/tn/map.c
	include/tn/map.h
	src/tn/queue_spsc.c
	include/tn/queue_spsc.h
	src/tn/allocator.c
	include/tn/allocator.h
	src/tn/thread.c
	include/tn/thread.h
	src/tn/mutex.c
	include/tn/mutex.h
	src/tn/atomic.c
	include/tn/atomic.h
	src/tn/buffer.c
	include/tn/buffer.h
	src/tn/buffer_pool.c
	include/tn/buffer_pool.h
	src/tn/event.c
	include/tn/event.h
	src/tn/cmd.c
	include/tn/cmd.h
	src/tn/term.c
	include/tn/term.h
	src/tn/dso.c
	include/tn/dso.h
	src/tn/heap.c
	include/tn/heap.h
	include/tn/error.h
	include/tn/test_harness.h
	include/tn/config.h
)
add_library(tn STATIC ${TN_LIBRARY_SRCS})
target_link_libraries(tn ${TN_LIBRARIES})
target_compile_definitions(tn PRIVATE ${TN_TARGET_COMPILE_DEFS})
target_compile_definitions(tn PRIVATE -DTN_EXPORTING)
list(APPEND TN_LIBRARIES tn)

#
# build tn test executables
set(TN_TESTS_C
	"tests/test_buffer.c"
	"tests/test_endpoint.c"
	"tests/test_event.c"
	"tests/test_heap.c"
	"tests/test_list_block.c"
	"tests/test_list_ptr.c"
	"tests/test_queue_spsc.c"
)
set(TESTS ${TN_TESTS_C})

tn_add_test_case(tn_buffer_ops)
tn_add_test_case(tn_endpoint_ops)
tn_add_test_case(tn_event_list_ops)
tn_add_test_case(tn_heap_ops)
tn_add_test_case(tn_list_block_create)
tn_add_test_case(tn_list_block_ops)
tn_add_test_case(tn_list_ptr_create)
tn_add_test_case(tn_list_ptr_ops)
tn_add_test_case(tn_queue_spsc_stress)
tn_add_test_case(tn_queue_spsc_testempty)
tn_add_test_case(tn_queue_spsc_testfull)
tn_add_test_case(tn_queue_spsc_npot)

tn_generate_test_driver(tn-tests tn)
target_link_libraries(tn-tests ${TN_LIBRARIES})
target_compile_definitions(tn-tests PRIVATE ${TN_TARGET_COMPILE_DEFS})

# runner for single test for easy debug step through an impl issue
#tn_test_single(test_heap tn_heap_ops "${TN_LIBRARIES}" "${TN_TARGET_COMPILE_DEFS}")
#tn_test_single(test_endpoint tn_endpoint_ops "${TN_LIBRARIES}" "${TN_TARGET_COMPILE_DEFS}")

set(TN_INCLUDE_DIRS ${TN_INCLUDE_DIRS} PARENT_SCOPE)
set(TN_LIBRARIES ${TN_LIBRARIES} PARENT_SCOPE)
