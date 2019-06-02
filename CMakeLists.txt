cmake_minimum_required(VERSION 3.1)
project(tn LANGUAGES C)

set(TN_DEPS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/deps)
set(TN_LIBRARIES "")
set(TN_INCLUDE_DIRS "")

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
if(NOT EXISTS ${TN_DEPS_DIR}/libuv)
	execute_process(COMMAND git clone https://github.com/c6burns/libuv WORKING_DIRECTORY ${TN_DEPS_DIR})
endif()

add_custom_target(libuv_update
	COMMAND git checkout master
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
	${TN_SRC}/src/tn/log.c
	${TN_SRC}/include/tn/log.h
	${TN_SRC}/src/tn/system.c
	${TN_SRC}/include/tn/system.h
	${TN_SRC}/src/tn/endpoint.c
	${TN_SRC}/include/tn/endpoint.h
	${TN_SRC}/src/tn/uuid.c
	${TN_SRC}/include/tn/uuid.h
	${TN_SRC}/src/tn/time.c
	${TN_SRC}/include/tn/time.h
	${TN_SRC}/src/tn/list_ptr.c
	${TN_SRC}/include/tn/list_ptr.h
	${TN_SRC}/src/tn/list_block.c
	${TN_SRC}/include/tn/list_block.h
	${TN_SRC}/src/tn/map.c
	${TN_SRC}/include/tn/map.h
	${TN_SRC}/src/tn/queue_spsc.c
	${TN_SRC}/include/tn/queue_spsc.h
	${TN_SRC}/src/tn/allocator.c
	${TN_SRC}/include/tn/allocator.h
	${TN_SRC}/src/tn/thread.c
	${TN_SRC}/include/tn/thread.h
	${TN_SRC}/src/tn/mutex.c
	${TN_SRC}/include/tn/mutex.h
	${TN_SRC}/src/tn/atomic.c
	${TN_SRC}/include/tn/atomic.h
	${TN_SRC}/src/tn/buffer.c
	${TN_SRC}/include/tn/buffer.h
	${TN_SRC}/src/tn/buffer_pool.c
	${TN_SRC}/include/tn/buffer_pool.h
	${TN_SRC}/src/tn/event.c
	${TN_SRC}/include/tn/event.h
	${TN_SRC}/src/tn/cmd.c
	${TN_SRC}/include/tn/cmd.h
	${TN_SRC}/src/tn/term.c
	${TN_SRC}/include/tn/term.h
	${TN_SRC}/include/tn/error.h
	${TN_SRC}/include/tn/test_harness.h
	${TN_SRC}/include/tn/config.h
)
add_library(tn STATIC ${TN_LIBRARY_SRCS})
target_link_libraries(tn ${TN_LIBRARIES})
target_compile_definitions(tn PRIVATE ${TN_TARGET_COMPILE_DEFS})
list(APPEND TN_LIBRARIES tn)


#
# build tn test executables
set(TN_TESTS_C
	"tests/test_buffer.c"
	"tests/test_endpoint.c"
	"tests/test_event.c"
	"tests/test_list_block.c"
	"tests/test_list_ptr.c"
	"tests/test_queue_spsc.c"
)
set(TESTS ${TN_TESTS_C})

tn_add_test_case(test_events)
tn_add_test_case(test_buffer_pool)
tn_add_test_case(test_endpoints)
tn_add_test_case(test_list_block_create)
tn_add_test_case(test_list_block_ops)
tn_add_test_case(test_list_ptr_create)
tn_add_test_case(test_list_ptr_ops)
tn_add_test_case(test_queue_spsc_stress)
tn_add_test_case(test_queue_spsc_empty)
tn_add_test_case(test_queue_spsc_full)
tn_add_test_case(test_queue_spsc_npot)

tn_generate_test_driver(tn-tests tn)
target_link_libraries(tn-tests ${TN_LIBRARIES})
target_compile_definitions(tn-tests PRIVATE ${TN_TARGET_COMPILE_DEFS})

# runner for single test for easy debug step through an impl issue
#tn_test_single(test_event test_events "${TN_LIBRARIES}" "${TN_TARGET_COMPILE_DEFS}")


set(TN_INCLUDE_DIRS ${TN_INCLUDE_DIRS} PARENT_SCOPE)
set(TN_LIBRARIES ${TN_LIBRARIES} PARENT_SCOPE)