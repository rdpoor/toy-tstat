/**
 * @file test_mu_task.c
 *
 * MIT License
 *
 * Copyright (c) 2022 - 2023 R. Dunbar Poor
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

/**
NOTE: mu_task can be compiled with or without MU_CONFIG_EXTENDED_TASK in effect,
so we really need two sets of tests.  This is addressed in:

https://stackoverflow.com/questions/50638724/how-to-compile-source-files-with-different-2-flags-in-cmake

In particular, here's the trick for CMake:

# compiles source.cpp with -DSOME_DEF=1
add_library(obj1 OBJECT source.cpp)

# This is the "modern" syntax for setting preprocessor definitions:
target_compile_definitions(obj1 PRIVATE SOME_DEF=1)

# compiles source.cpp with -DSOME_DEF=2
add_library(obj2 OBJECT source.cpp)

*/

// *****************************************************************************
// Includes

#include "mu_task.h"
#include "mu_sched.h"   // NOTE: mu_sched should be mocked, not called directly
#include "test_support.h"
#include <stdio.h>

// *****************************************************************************
// Local (private) types and definitions

typedef struct {
    mu_task_t task;
    uint8_t call_count;
} test_ctx_t;

// *****************************************************************************
// Local (private, static) forward declarations

static void test_fn(mu_task_t *task, void *arg);

static void task_transfer_hook(mu_task_t *prev_task, mu_task_t *next_task);

static void task_state_change_hook(mu_task_t *task, mu_task_state_t prev_state,
                                   mu_task_state_t next_state);

// *****************************************************************************
// Local (private, static) storage

int s_transfer_hook_count;
int s_state_change_hook_count;

// *****************************************************************************
// Public code

void test_mu_task(void) {
    printf("\nStarting test_mu_task...");

    test_ctx_t ctx1;
    test_ctx_t ctx2;

    // #define MU_TASK_CTX(_task_pointer, _ctx_type, _task_slot)
    // MU_TASK_CTX(task) should evaluate to the task's context.
    MU_ASSERT(MU_TASK_CTX(&ctx1.task, test_ctx_t, task) == &ctx1);
    MU_ASSERT(MU_TASK_CTX(&ctx2.task, test_ctx_t, task) == &ctx2);

    // mu_task_t *mu_task_init(mu_task_t *task, mu_task_fn fn,
    //                         mu_task_state_t initial_state);
    MU_ASSERT(mu_task_init(&ctx1.task, test_fn, 1) == &ctx1.task);
    MU_ASSERT(mu_task_init(&ctx2.task, test_fn, 2) == &ctx2.task);

    // void mu_task_call(mu_task_t *task, void *arg);
    ctx1.call_count = 0;
    mu_task_call(&ctx1.task, NULL);
    MU_ASSERT(ctx1.call_count == 1);

    // mu_task_fn mu_task_get_fn(mu_task_t *task);
    MU_ASSERT(mu_task_get_fn(&ctx1.task) == test_fn);

    // mu_task_state_t mu_task_get_state(mu_task_t *task);
    // void mu_task_set_state(mu_task_t *task, mu_task_state_t state);
    mu_task_set_state(&ctx1.task, 22);
    MU_ASSERT(mu_task_get_state(&ctx1.task) == 22);

    // ======================
    // TODO: use a framework that supports mocking of mu_sched for the following
    // functions and make sure the mock is called with the proper arguments.

    // mu_task_t *mu_task_get_current_task(void);
    // well tested in test_mu_sched()

    // mu_task_err_t mu_task_wait(mu_task_t *task,
    //                            mu_task_state_t next_state);
    // mu_task_wait sets state but does not schedule the task...
    mu_task_set_state(&ctx1.task, 22);
    MU_ASSERT(mu_task_wait(&ctx1.task, 23) == MU_TASK_ERR_NONE);
    MU_ASSERT(mu_task_get_state(&ctx1.task) == 23);


    // mu_task_err_t mu_task_yield(mu_task_t *task,
    //                             mu_task_state_t next_state);
    // The scheduling of ctx1 is tested in test_mu_sched.  Just verify that
    // the state gets set.
    mu_sched_init();
    mu_task_yield(&ctx1.task, 24);
    MU_ASSERT(mu_task_get_state(&ctx1.task) == 24);

    // mu_task_err_t mu_task_sched_from_isr(mu_task_t *task);
    // Already tested in test_mu_sched()

    // mu_task_err_t mu_task_defer_for(mu_task_t *task,
    //                                 mu_task_state_t next_state,
    //                                 mu_time_rel_t in);
    mu_sched_init();
    MU_ASSERT(mu_task_defer_for(&ctx1.task, 25, 1) == MU_TASK_ERR_NONE);
    MU_ASSERT(mu_task_get_state(&ctx1.task) == 25);

    // mu_task_err_t mu_task_defer_until(mu_task_t *task,
    //                                   mu_task_state_t next_state,
    //                                   mu_time_abs_t at);
    mu_sched_init();
    MU_ASSERT(mu_task_defer_until(&ctx1.task, 26, 1) == MU_TASK_ERR_NONE);
    MU_ASSERT(mu_task_get_state(&ctx1.task) == 26);

    // mu_task_err_t mu_task_remove_deferred_task(mu_task_t *task);
    mu_sched_init();
    MU_ASSERT(mu_task_remove_deferred_task(&ctx1.task) == MU_TASK_ERR_NOT_FOUND);

    // mu_task_err_t mu_task_transfer(mu_task_t *from_task,
    //                                mu_task_state_t next_state,
    //                                mu_task_t *to_task);
    mu_sched_init();
    mu_task_set_state(&ctx2.task, 222);
    MU_ASSERT(mu_task_transfer(&ctx1.task, 27, &ctx2.task) == MU_TASK_ERR_NONE);
    MU_ASSERT(mu_task_get_state(&ctx1.task) == 27);
    MU_ASSERT(mu_task_get_state(&ctx2.task) == 222);

    // with mu_task_state_change_hook
    // TODO: test this feature, probably with fff

    // with mu_task_state_change_hook
    mu_task_set_state_change_hook(task_state_change_hook);
    mu_task_init(&ctx1.task, test_fn, 1);
    s_state_change_hook_count = 0;
    // should get called when state changes
    mu_task_set_state(&ctx1.task, 2);
    MU_ASSERT(s_state_change_hook_count == 1);
    // should not called when state stays the same
    mu_task_set_state(&ctx1.task, 2);
    MU_ASSERT(s_state_change_hook_count == 1);

    printf("\n   Completed test_mu_task.");
}

// *****************************************************************************
// Local (private, static) code

static void test_fn(mu_task_t *task, void *arg) {
    test_ctx_t *self = MU_TASK_CTX(task, test_ctx_t, task);
    (void)arg;
    self->call_count += 1;
}

static void task_transfer_hook(mu_task_t *prev_task, mu_task_t *next_task) {
    s_transfer_hook_count += 1;
}

static void task_state_change_hook(mu_task_t *task, mu_task_state_t prev_state,
                                   mu_task_state_t next_state) {
    s_state_change_hook_count += 1;
}
