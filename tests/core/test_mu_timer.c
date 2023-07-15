/**
 * @file test_mu_timer.c
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

#include "mu_timer.h"

#include "mu_sched.h"
#include "mu_task.h"
#include "mu_time.h"
#include "test_support.h"
#include <stdio.h>

// *****************************************************************************
// Local (private) types and definitions

// *****************************************************************************
// Local (private, static) storage

static counting_obj_t s_obj1;
static mu_task_t *s_task1;
static mu_time_abs_t s_time;

// *****************************************************************************
// Local (private, static) forward declarations

static void setup(void);
static mu_time_abs_t get_test_time(void);
static void set_test_time(mu_time_abs_t time);

// *****************************************************************************
// Public code

void test_mu_timer(void) {
    printf("\nStarting test_mu_timer...");

    mu_timer_t timer;

    // void mu_timer_start(mu_timer_t *timer,
    //                     uint32_t delay_tics,
    //                     bool periodic,
    //                     mu_task_t *on_completion);
    setup();
    mu_timer_init(&timer);

    mu_timer_start(&timer, 5, false, s_task1);
    set_test_time(4);
    mu_sched_step();
    MU_ASSERT(counting_obj_get_call_count(&s_obj1) == 0);
    set_test_time(5);
    mu_sched_step();
    MU_ASSERT(counting_obj_get_call_count(&s_obj1) == 1);
    set_test_time(10);
    mu_sched_step();
    MU_ASSERT(counting_obj_get_call_count(&s_obj1) == 1);

    setup();
    mu_timer_init(&timer);

    mu_timer_start(&timer, 5, true, s_task1);
    set_test_time(4);
    mu_sched_step();
    MU_ASSERT(counting_obj_get_call_count(&s_obj1) == 0);
    set_test_time(5);
    mu_sched_step();
    MU_ASSERT(counting_obj_get_call_count(&s_obj1) == 1);
    set_test_time(10);
    mu_sched_step();
    MU_ASSERT(counting_obj_get_call_count(&s_obj1) == 2);

    // void mu_timer_stop(mu_timer_t *timer);
    // bool mu_timer_is_running(mu_timer_t *timer);
    // bool mu_timer_is_stopped(mu_timer_t *timer);
    setup();
    mu_timer_init(&timer);

    mu_timer_start(&timer, 5, false, s_task1);
    set_test_time(4);
    mu_sched_step();
    MU_ASSERT(counting_obj_get_call_count(&s_obj1) == 0);
    MU_ASSERT(mu_timer_is_running(&timer) == true);
    MU_ASSERT(mu_timer_is_stopped(&timer) == false);
    mu_timer_stop(&timer);
    MU_ASSERT(mu_timer_is_running(&timer) == false);
    MU_ASSERT(mu_timer_is_stopped(&timer) == true);
    set_test_time(5);
    mu_sched_step();
    MU_ASSERT(counting_obj_get_call_count(&s_obj1) == 0);

    printf("\n   Completed test_mu_timer.");
}

// *****************************************************************************
// Local (private, static) code

static void setup(void) {
    mu_sched_init();
    s_task1 = counting_obj_task(counting_obj_init(&s_obj1));
    mu_sched_set_clock_source(get_test_time);
    set_test_time(0);
}

static mu_time_abs_t get_test_time(void) {
    return s_time;
}

static void set_test_time(mu_time_abs_t time) {
    s_time = time;
}
