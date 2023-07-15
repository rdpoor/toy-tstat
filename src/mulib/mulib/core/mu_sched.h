/**
 * MIT License
 *
 * Copyright (c) 2020 R. Dunbar Poor <rdpoor@gmail.com>
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
Implmeentation notes:

mu_sched implements a discrete time, run-to-completion scheduler.  A
mu_task can be scheduled to run at some point in the future through the
following calls:

   mu_task_err_t mu_sched_asap(mu_task_t *task);
   mu_task_err_t mu_sched_defer_until(mu_task_t *task, mu_time_abs_t at);
   mu_task_err_t mu_sched_defer_for(mu_task_t *task, mu_time_rel_t in);

Each of these functions add a task to the scheduler's queue.

mu_sched also supports safely scheduling a task from interrupt level:

   mu_task_err_t mu_sched_from_isr(mu_task_t *task);

Any task scheduled from interrupt level is saved in an interrupt safe
"single producer, single consumer" queue.  Upon returning from interrupt level,
the next time mu_sched_step() is called, any tasks on the queue are added
to the regular schedule as if mu_sched_asap() was called.

The function

   mu_task_err_t mu_sched_step(void);

is where all the magic happens.  The scheduler examines the first task in
the queue, and if its start time has arrived, the task is removed from the
queue and is called.

*/

#ifndef _MU_SCHED_H_
#define _MU_SCHED_H_

#ifdef __cplusplus
extern "C" {
#endif

// *****************************************************************************
// Includes

#include "mu_config.h"
#include "mu_task.h"
#include "mu_time.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// *****************************************************************************
// C++ Compatibility

#ifdef __cplusplus
extern "C" {
#endif

// *****************************************************************************
// Public types and definitions

#ifndef MU_CONFIG_SCHED_MAX_DEFERRED_TASKS
#define MU_CONFIG_SCHED_MAX_DEFERRED_TASKS 20
#endif

#ifndef MU_CONFIG_SCHED_MAX_IRQ_TASKS
#define MU_CONFIG_SCHED_MAX_IRQ_TASKS 8 // must be a power of two!
#endif

#ifndef MU_CONFIG_SCHED_MAX_ASAP_TASKS
#define MU_CONFIG_SCHED_MAX_ASAP_TASKS 20
#endif

// Signature for clock source function.  Returns the current time.
typedef mu_time_abs_t (*mu_clock_fn)(void);

// *****************************************************************************
// Public declarations

/**
 * @brief initialize the schedule module.  Not interrupt safe.
 */
void mu_sched_init(void);

/**
 * @brief  Remove all scheduled items from the schedule.  Not interrupt safe.
 */
void mu_sched_reset(void);

/**
 * @brief Process the next runnable task or -- if none are runnable -- the idle
 * task.
 */
void mu_sched_step(void);

/**
 * @brief Return the current clock source.
 */
mu_clock_fn mu_sched_get_clock_source(void);

/**
 * @brief Set the clock source for the scheduler.
 *
 * This function is provided primarily for unit testing.
 *
 * @param clock_fn A function that returns the current time.
 */
void mu_sched_set_clock_source(mu_clock_fn clock_fn);

/**
 * @brief Return the scheduler's idea of time according to the clock source.
 */
mu_time_abs_t mu_sched_get_current_time(void);

/**
 * @brief Return the idle task.
 */
mu_task_t *mu_sched_get_idle_task(void);

/**
 * @brief Set the task to be invoked when there are no tasks to run.
 *
 * Setting this to NULL runs the default idle task.
 */
void mu_sched_set_idle_task(mu_task_t *task);

/**
 * @brief Return the task currently being run or NULL if none are active.
 */
mu_task_t *mu_sched_current_task(void);

/**
 * @brief Return the next task to be processed, or NULL if none.
 *
 * Note: this consults the asap and deferreds queues but not the irq queue.
 */
mu_task_t *mu_sched_peek_next_task(void);

/**
 * @brief Schedule a task to run as soon as possible.
 */
mu_task_err_t mu_sched_asap(mu_task_t *task);

/**
 * @brief Schedule a task to run as soon as possible from interrupt level.
 */
mu_task_err_t mu_sched_from_isr(mu_task_t *task);

/**
 * @brief Schedule a task to run at the specified time in the future.
 */
mu_task_err_t mu_sched_defer_until(mu_task_t *task, mu_time_abs_t at);

/**
 * @brief Schedule a task to run after the specified delay.
 */
mu_task_err_t mu_sched_defer_for(mu_task_t *task, mu_time_rel_t in);

/**
 * @brief Remove a deferred task from the schedule.
 */
mu_task_err_t mu_sched_remove_deferred_task(mu_task_t *task);

// *****************************************************************************
// End of file

#ifdef __cplusplus
}
#endif

#endif /* #ifndef _MU_SCHED_H_ */
