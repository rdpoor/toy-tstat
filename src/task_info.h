/**
 * @file task_info.h
 *
 * MIT License
 *
 * Copyright (c) 2023 PRO1 IAQ, INC.
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
 * @brief Log task transitions and state transitions.
 */

#ifndef _TASK_INFO_H_
#define _TASK_INFO_H_

// *****************************************************************************
// Includes

#include "mulib/core/mu_task.h"
#include <stddef.h>
#include <stdint.h>

// *****************************************************************************
// C++ compatibility

#ifdef __cplusplus
extern "C" {
#endif

// *****************************************************************************
// Public types and definitions

typedef struct {
    const char *task_name;    // string name of this task
    const char **state_names; // array of state names for this task
    size_t n_states;          // number of states
} task_info_t;

// *****************************************************************************
// Public declarations

/**
 * @brief Install task_info hooks to log task and state transitions.
 */
void task_info_init(void);

/**
 * @brief Return the task name associated with a task.
 */
const char *task_info_task_name(mu_task_t *task);

/**
 * @brief Return the state name associated with a task.
 */
const char *task_info_state_name(mu_task_t *task, mu_task_state_t state);

/**
 * @brief Transfer from current task to continuation task
 *
 * If had_error is true, this logs a warning prior to transferring control.
 *
 * @param task The current task
 * @param terminal_state The terminal state for task
 * @param had_error True if ending with an error.
 * @param continuation The next task to be invoked (or NULL)
 */
void task_info_endgame(mu_task_t *from, mu_task_state_t terminal_state,
                       bool had_error, mu_task_t *continuation);

// *****************************************************************************
// End of file

#ifdef __cplusplus
}
#endif

#endif /* #ifndef _TASK_INFO_H_ */
