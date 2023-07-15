/**
 * @file timer.h
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
 */

/**
 * @brief Start a timer, notify a task on completion.
 */

#ifndef _MU_TIMER_H_
#define _MU_TIMER_H_

// *****************************************************************************
// Includes

#include "mu_task.h"
#include "mu_time.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// *****************************************************************************
// C++ compatibility

#ifdef __cplusplus
extern "C" {
#endif

// *****************************************************************************
// Public types and definitions

typedef enum {
  MU_TIMER_STATE_IDLE,
  MU_TIMER_STATE_RUNNING,
  MU_TIMER_STATE_ERROR
} mu_timer_state_t;

typedef struct {
  mu_task_t task;           // the timer task object
  mu_timer_state_t state;   // current timer state
  mu_task_t *on_completion; // task to run upon completion.
  mu_time_rel_t delay_tics;
  mu_time_abs_t delay_until;
  bool periodic;
} mu_timer_t;

// *****************************************************************************
// Public declarations

/**
 * @brief Initialize the timer.
 */
void mu_timer_init(mu_timer_t *timer);

/**
 * @brief Start the timer.
 *
 * Note: if the timer is already running, it will be stopped and restarted
 * without notifying on_completion.
 *
 * @param timer Previously initialized by timer_init().
 * @param delay_tics The number of tics before the timer expires.
 * @param periodic If true, the timer repeats periodically, else is one shot.
 * @param on_completion The task to call upon expiration.  May be NULL.
 */
void mu_timer_start(mu_timer_t *timer,
                    mu_time_rel_t delay_tics,
                    bool periodic,
                    mu_task_t *on_completion);

/**
 * @brief Stop the timer.
 *
 * Note: The on_completion task will not be notified.
 * Note: If the timer is already stopped, this has no effect.
 *
 * @param timer The timer, previously initialized by timer_init().
 */
void mu_timer_stop(mu_timer_t *timer);

/**
 * @brief Return true if the timer is currently running.
 */
bool mu_timer_is_running(mu_timer_t *timer);

/**
 * @brief Return true if the timer is stopped (expired or error)
 */
bool mu_timer_is_stopped(mu_timer_t *timer);

// *****************************************************************************
// End of file

#ifdef __cplusplus
}
#endif

#endif /* #ifndef _MU_TIMER_H_ */
