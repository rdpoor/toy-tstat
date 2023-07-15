/**
 * @file mu_timer.c
 *
 * MIT License
 *
 * Copyright (c) 2021-2022 R. D. Poor <rdpoor@gmail.com>
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

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// *****************************************************************************
// Private types and definitions

// *****************************************************************************
// Private (static) storage

// *****************************************************************************
// Private (static, forward) declarations

/**
 * @brief The primary state machine for the mu_timer task.
 */
static void mu_timer_fn(mu_task_t *task, void *arg);

// *****************************************************************************
// Public code

void mu_timer_init(mu_timer_t *timer) {
  mu_task_init(&timer->task, mu_timer_fn, MU_TIMER_STATE_IDLE, NULL);
}

void mu_timer_start(mu_timer_t *timer,
                    mu_time_rel_t delay_tics,
                    bool periodic,
                    mu_task_t *on_completion) {
  mu_timer_stop(timer);
  mu_time_abs_t now = mu_sched_get_clock_source()(); // function pointers, whoo!
  timer->delay_tics = delay_tics;
  timer->delay_until = mu_time_offset(now, timer->delay_tics);
  timer->periodic = periodic;
  timer->state = MU_TIMER_STATE_RUNNING;
  timer->on_completion = on_completion;
  mu_sched_defer_until(&timer->task, timer->delay_until);
}

void mu_timer_stop(mu_timer_t *timer) {
  if (timer->state == MU_TIMER_STATE_RUNNING) {
    mu_sched_remove_deferred_task(&timer->task); // cancel any existing timer.
  }
  timer->state = MU_TIMER_STATE_IDLE;
}

bool mu_timer_is_running(mu_timer_t *timer) {
  return timer->state == MU_TIMER_STATE_RUNNING;
}

bool mu_timer_is_stopped(mu_timer_t *timer) {
  return timer->state != MU_TIMER_STATE_RUNNING;
}

// *****************************************************************************
// Private (static) code

static void mu_timer_fn(mu_task_t *task, void *arg) {
  mu_timer_t *self = MU_TASK_CTX(task, mu_timer_t, task);
  (void)arg; // unused

  switch (self->state) {

  case MU_TIMER_STATE_IDLE: {
    // wait here for a call to mu_timer_start();
  } break;

  case MU_TIMER_STATE_RUNNING: {
    // arrive here when timer expires
    if (self->periodic) {
      // Schedule next wakeup (and prevent time slippage...)
      self->delay_until = mu_time_offset(self->delay_until, self->delay_tics);
      mu_sched_defer_until(&self->task, self->delay_until);
    } else {
      // Stop timer.
      self->state = MU_TIMER_STATE_IDLE;
    }
    // Invoke on-completion task.
    mu_task_call(self->on_completion, NULL);
  } break;

  case MU_TIMER_STATE_ERROR: {
    asm("nop");
  } break;

  } // switch
}

// *****************************************************************************
// Standalone tests

// Run this command to run the standalone tests.
/**

gcc -g -Wall -DMU_TIMER_STANDALONE_TESTS -I../posix_platform -o mu_timer_test \
  mu_timer.c mu_sched.c mu_spsc.c mu_task.c
../posix_platform/mu_time.c && \
  ./mu_timer_test && rm -rf ./mu_timer_test ./mu_timer_test.dSYM

 */

#ifdef MU_TIMER_STANDALONE_TESTS

#include <stdio.h>

#define ASSERT(e) assert(e, #e, __FILE__, __LINE__)

static void assert(bool expr, const char *str, const char *file, int line) {
  if (!expr) {
    printf("\nassertion %s failed at %s:%d", str, file, line);
  }
}

int s_called_count;

// The timer under test.
static mu_timer_t s_timer;

// A task that simply prints out the current time
static mu_task_t s_dummy_task;

static mu_time_abs_t get_now(void) { return mu_sched_get_clock_source()(); }

static void dummy_task_fn(void *ctx, void *arg) {
  printf("\ntimer call count = %d, called at %ld", s_called_count, get_now());
  if (s_called_count > 5) {
    printf("\nstopping.");
    mu_timer_stop(&s_timer);
  } else {
    s_called_count += 1;
  }
}

static void setup(void) {
  mu_sched_init();
  mu_timer_init(&s_timer);
  s_called_count = 0;
  mu_task_init(&s_dummy_task, dummy_task_fn, NULL, "Dummy Task");
}

int main(void) {
  printf("\nStarting mu_timer tests...");
  ASSERT(1 == 1);

  setup();
  mu_timer_start(&s_timer, 500, true, &s_dummy_task);
  while (s_called_count < 5) {
    mu_sched_step();
  }
  // pretty weak assertion...
  ASSERT(s_called_count == 5);

  setup();
  mu_timer_start(&s_timer, 500, false, &s_dummy_task);
  while (s_called_count < 1) {
    mu_sched_step();
  }
  // pretty weak assertion...
  ASSERT(s_called_count == 1);

  printf("\n...tests complete\n");
  return 0;
}

#endif // #ifdef MU_TIMER_STANDALONE_TESTS

// *****************************************************************************
// End of file
