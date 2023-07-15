# SYNOPSIS:
#   cmake -S . -B build
#   (cd build && make && ./test_mulib_core)
#   rm -rf build
#
# TODO 1:
# mu_task can be compiled with or without MU_CONFIG_EXTENDED_TASK in effect,
# so we really need two sets of tests.  This is addressed in:
# https://stackoverflow.com/questions/50638724/how-to-compile-source-files-with-different-2-flags-in-cmake
# In particular, here's the trick for CMake:
#   # compiles source.cpp with -DSOME_DEF=1
#   add_library(obj1 OBJECT source.cpp)
#   # This is the "modern" syntax for setting preprocessor definitions:
#   target_compile_definitions(obj1 PRIVATE SOME_DEF=1)
#   # compiles source.cpp with -DSOME_DEF=2
#   add_library(obj2 OBJECT source.cpp)
#
# TODO 2:
# incorporate fff.h, notably to isolate mu_test_task and mu_test_timer from
# mu_sched.

cmake_minimum_required(VERSION 3.10)

# Set the project name
project(MulibTests)

set(SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/mulib/core")
set(PLATFORM_DIR "${CMAKE_CURRENT_SOURCE_DIR}/mulib/platform")
set(TESTS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/tests")
set(CORE_TESTS_DIR "${TESTS_DIR}/core")
set(TEST_SUPPORT_DIR "${TESTS_DIR}/test_support")

# Include the core and test_support directories
include_directories(${SOURCE_DIR} ${PLATFORM_DIR} ${TEST_SUPPORT_DIR})

# Core library
set(CORE_SRC
    ${SOURCE_DIR}/mu_mqueue.c
    ${SOURCE_DIR}/mu_sched.c
    ${SOURCE_DIR}/mu_spsc.c
    ${SOURCE_DIR}/mu_str.c
    ${SOURCE_DIR}/mu_task.c
    ${SOURCE_DIR}/mu_timer.c
)

# Platform
set(PLATFOR_SRC
    ${PLATFORM_DIR}mu_time.c
)

add_library(core STATIC ${CORE_SRC})

# Test support library
set(TEST_SUPPORT_SRC
    ${TEST_SUPPORT_DIR}/test_support.c
)

add_library(test_support STATIC ${TEST_SUPPORT_SRC})

# Create the executable for testing
add_executable(test_mulib_core
    tests/core/test_mulib_core.c
    tests/core/test_mu_macros.c
    tests/core/test_mu_mqueue.c
    tests/core/test_mu_sched.c
    tests/core/test_mu_spsc.c
    tests/core/test_mu_str.c
    tests/core/test_mu_task.c
    tests/core/test_mu_time.c
    tests/core/test_mu_timer.c
    mulib/core/mu_mqueue.c
    mulib/core/mu_sched.c
    mulib/core/mu_spsc.c
    mulib/core/mu_str.c
    mulib/core/mu_task.c
    mulib/core/mu_timer.c
    mulib/platform/mu_time.c
)

# Link any required libraries
target_link_libraries(test_mulib_core core test_support)

# Enable testing
enable_testing()

# Add the test to be executed
add_test(NAME test_mulib_core COMMAND test_mulib_core)
