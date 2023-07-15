/**
 * @file test_mu_sched.c
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

// *****************************************************************************
// Includes

#include "mu_sched.h"
#include "test_support.h"
#include <stdio.h>


// *****************************************************************************
// Local (private) types and definitions

// *****************************************************************************
// Local (private, static) storage

static counting_obj_t s_idle_obj;
static counting_obj_t s_obj1;
static counting_obj_t s_obj2;
static mu_task_t *s_idle_task;
static mu_task_t *s_task1;
static mu_task_t *s_task2;
static mu_time_abs_t s_time;
static mu_task_t s_basic_task;

// *****************************************************************************
// Local (private, static) forward declarations

static void setup(void);
static mu_time_abs_t get_test_time(void);
static void set_test_time(mu_time_abs_t time);
static void basic_task_fn(mu_task_t *task, void *arg);

// *****************************************************************************
// Public code

void test_mu_sched(void) {
    printf("\nStarting test_mu_sched...");

    setup();
    MU_ASSERT(counting_obj_get_call_count(&s_idle_obj) == 0);
    mu_sched_step();
    MU_ASSERT(counting_obj_get_call_count(&s_idle_obj) == 1);

    // void mu_sched_reset(void);
    // void mu_sched_step(void);
    // mu_clock_fn mu_sched_get_clock_source(void);
    // void mu_sched_set_clock_source(mu_clock_fn clock_fn);
    // mu_time_abs_t mu_sched_get_current_time(void);
    // mu_task_t *mu_sched_get_idle_task(void);
    // void mu_sched_set_idle_task(mu_task_t *task);

    setup();
    MU_ASSERT(mu_sched_asap(s_task1) == MU_TASK_ERR_NONE);
    MU_ASSERT(counting_obj_get_call_count(&s_obj1) == 0);
    mu_sched_step();   // task1 is available to run
    MU_ASSERT(counting_obj_get_call_count(&s_obj1) == 1);
    MU_ASSERT(counting_obj_get_call_count(&s_idle_obj) == 0);
    mu_sched_step();   // task1 already ran - run idle task instead
    MU_ASSERT(counting_obj_get_call_count(&s_obj1) == 1);
    MU_ASSERT(counting_obj_get_call_count(&s_idle_obj) == 1);

    // mu_sched_asap preserves order
    setup();
    MU_ASSERT(mu_sched_asap(s_task1) == MU_TASK_ERR_NONE);
    MU_ASSERT(mu_sched_asap(s_task2) == MU_TASK_ERR_NONE);
    mu_sched_step();   // task1 runs first
    MU_ASSERT(counting_obj_get_call_count(&s_idle_obj) == 0);
    MU_ASSERT(counting_obj_get_call_count(&s_obj1) == 1);
    MU_ASSERT(counting_obj_get_call_count(&s_obj2) == 0);
    mu_sched_step();   // followed by task2
    MU_ASSERT(counting_obj_get_call_count(&s_idle_obj) == 0);
    MU_ASSERT(counting_obj_get_call_count(&s_obj1) == 1);
    MU_ASSERT(counting_obj_get_call_count(&s_obj2) == 1);
    mu_sched_step();   // followed by idle task
    MU_ASSERT(counting_obj_get_call_count(&s_idle_obj) == 1);
    MU_ASSERT(counting_obj_get_call_count(&s_obj1) == 1);
    MU_ASSERT(counting_obj_get_call_count(&s_obj2) == 1);

    // mu_sched_from_isr preserves order
    setup();
    MU_ASSERT(mu_sched_from_isr(s_task1) == MU_TASK_ERR_NONE);
    MU_ASSERT(mu_sched_from_isr(s_task2) == MU_TASK_ERR_NONE);
    mu_sched_step();   // task1 runs first
    MU_ASSERT(counting_obj_get_call_count(&s_idle_obj) == 0);
    MU_ASSERT(counting_obj_get_call_count(&s_obj1) == 1);
    MU_ASSERT(counting_obj_get_call_count(&s_obj2) == 0);
    mu_sched_step();   // followed by task2
    MU_ASSERT(counting_obj_get_call_count(&s_idle_obj) == 0);
    MU_ASSERT(counting_obj_get_call_count(&s_obj1) == 1);
    MU_ASSERT(counting_obj_get_call_count(&s_obj2) == 1);
    mu_sched_step();   // followed by idle task
    MU_ASSERT(counting_obj_get_call_count(&s_idle_obj) == 1);
    MU_ASSERT(counting_obj_get_call_count(&s_obj1) == 1);
    MU_ASSERT(counting_obj_get_call_count(&s_obj2) == 1);

    // isr tasks run before regular tasks
    setup();
    MU_ASSERT(mu_sched_asap(s_task1) == MU_TASK_ERR_NONE);
    MU_ASSERT(mu_sched_from_isr(s_task2) == MU_TASK_ERR_NONE);
    mu_sched_step();   // task2 runs first
    MU_ASSERT(counting_obj_get_call_count(&s_idle_obj) == 0);
    MU_ASSERT(counting_obj_get_call_count(&s_obj1) == 0);
    MU_ASSERT(counting_obj_get_call_count(&s_obj2) == 1);
    mu_sched_step();   // followed by task1
    MU_ASSERT(counting_obj_get_call_count(&s_idle_obj) == 0);
    MU_ASSERT(counting_obj_get_call_count(&s_obj1) == 1);
    MU_ASSERT(counting_obj_get_call_count(&s_obj2) == 1);
    mu_sched_step();   // followed by idle task
    MU_ASSERT(counting_obj_get_call_count(&s_idle_obj) == 1);
    MU_ASSERT(counting_obj_get_call_count(&s_obj1) == 1);
    MU_ASSERT(counting_obj_get_call_count(&s_obj2) == 1);

    // mu_task_err_t mu_sched_defer_until(mu_task_t *task, mu_time_abs_t at);
    setup();
    MU_ASSERT(mu_sched_defer_until(s_task1, 10) == MU_TASK_ERR_NONE);
    MU_ASSERT(mu_sched_defer_until(s_task2, 12) == MU_TASK_ERR_NONE);
    set_test_time(8);
    mu_sched_step();
    MU_ASSERT(counting_obj_get_call_count(&s_idle_obj) == 1);
    MU_ASSERT(counting_obj_get_call_count(&s_obj1) == 0);
    MU_ASSERT(counting_obj_get_call_count(&s_obj2) == 0);
    set_test_time(10);
    mu_sched_step();
    MU_ASSERT(counting_obj_get_call_count(&s_idle_obj) == 1);
    MU_ASSERT(counting_obj_get_call_count(&s_obj1) == 1);
    MU_ASSERT(counting_obj_get_call_count(&s_obj2) == 0);
    set_test_time(12);
    mu_sched_step();
    MU_ASSERT(counting_obj_get_call_count(&s_idle_obj) == 1);
    MU_ASSERT(counting_obj_get_call_count(&s_obj1) == 1);
    MU_ASSERT(counting_obj_get_call_count(&s_obj2) == 1);

    // mu_task_err_t mu_sched_defer_for(mu_task_t *task, mu_time_rel_t in);
    setup();
    set_test_time(8);
    MU_ASSERT(mu_sched_defer_for(s_task1, 2) == MU_TASK_ERR_NONE);
    MU_ASSERT(mu_sched_defer_for(s_task2, 4) == MU_TASK_ERR_NONE);
    mu_sched_step();
    MU_ASSERT(counting_obj_get_call_count(&s_idle_obj) == 1);
    MU_ASSERT(counting_obj_get_call_count(&s_obj1) == 0);
    MU_ASSERT(counting_obj_get_call_count(&s_obj2) == 0);
    set_test_time(10);
    mu_sched_step();
    MU_ASSERT(counting_obj_get_call_count(&s_idle_obj) == 1);
    MU_ASSERT(counting_obj_get_call_count(&s_obj1) == 1);
    MU_ASSERT(counting_obj_get_call_count(&s_obj2) == 0);
    set_test_time(12);
    mu_sched_step();
    MU_ASSERT(counting_obj_get_call_count(&s_idle_obj) == 1);
    MU_ASSERT(counting_obj_get_call_count(&s_obj1) == 1);
    MU_ASSERT(counting_obj_get_call_count(&s_obj2) == 1);

    // mu_task_err_t mu_sched_remove_deferred_task(mu_task_t *task);
    setup();
    MU_ASSERT(mu_sched_defer_until(s_task1, 10) == MU_TASK_ERR_NONE);
    MU_ASSERT(mu_sched_defer_until(s_task2, 12) == MU_TASK_ERR_NONE);
    set_test_time(8);
    // first remove_deferred_task removes the task
    MU_ASSERT(mu_sched_remove_deferred_task(s_task1) == MU_TASK_ERR_NONE);
    // second call fails (since it's been removed)
    MU_ASSERT(mu_sched_remove_deferred_task(s_task1) == MU_TASK_ERR_NOT_FOUND);
    mu_sched_step();
    MU_ASSERT(counting_obj_get_call_count(&s_idle_obj) == 1);
    MU_ASSERT(counting_obj_get_call_count(&s_obj1) == 0);
    MU_ASSERT(counting_obj_get_call_count(&s_obj2) == 0);
    // task1 will not get called because it's been removed.  idle get called.
    set_test_time(10);
    mu_sched_step();
    MU_ASSERT(counting_obj_get_call_count(&s_idle_obj) == 2);
    MU_ASSERT(counting_obj_get_call_count(&s_obj1) == 0);
    MU_ASSERT(counting_obj_get_call_count(&s_obj2) == 0);
    set_test_time(12);
    mu_sched_step();
    MU_ASSERT(counting_obj_get_call_count(&s_idle_obj) == 2);
    MU_ASSERT(counting_obj_get_call_count(&s_obj1) == 0);
    MU_ASSERT(counting_obj_get_call_count(&s_obj2) == 1);

    // mu_task_t *mu_sched_get_current_task(void);
    mu_sched_asap(&s_basic_task);
    // verify that mu_sched_get_current_task() == &s_basic_task
    // see body of basic_task_fn
    mu_sched_step();
    // check that mu_sched_get_current_task() is null outside of a step() call
    MU_ASSERT(mu_sched_get_current_task() == NULL);

    printf("\n   Completed test_mu_sched.");
}

// *****************************************************************************
// Local (private, static) code

static void setup(void) {
    mu_sched_init();
    s_idle_task = counting_obj_task(counting_obj_init(&s_idle_obj));
    s_task1 = counting_obj_task(counting_obj_init(&s_obj1));
    s_task2 = counting_obj_task(counting_obj_init(&s_obj2));
    mu_sched_set_idle_task(s_idle_task);
    mu_sched_set_clock_source(get_test_time);
    set_test_time(0);
    mu_task_init(&s_basic_task, basic_task_fn, (mu_task_state_t)9);
}

static mu_time_abs_t get_test_time(void) {
    return s_time;
}

static void set_test_time(mu_time_abs_t time) {
    s_time = time;
}

static void basic_task_fn(mu_task_t *task, void *arg) {
    (void)arg;
    MU_ASSERT(mu_sched_get_current_task() == task);
}
