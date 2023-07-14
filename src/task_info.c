/**
 * @file task_info.c
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
 *
 */

// *****************************************************************************
// Includes

#include "task_info.h"

#include "bb_log.h"
#include "idle_task.h"
#include "mulib/core/mu_task.h"
#include <stdbool.h>
#include <stdint.h>

// *****************************************************************************
// Private types and definitions

// *****************************************************************************
// Private (static) storage

/**
 * @brief Track the previously invoked task.
 *
 * In the task_call_hook, if the called task != s_prev_task, log the transition
 * from s_prev_task to task and update s_prev_task.  Note, though, that the idle
 * task is ignored.
 */
static mu_task_t *s_prev_task;

// *****************************************************************************
// Private (static, forward) declarations

/**
 * @brief Called when a task is called.
 */
static void task_call_hook(mu_task_t *task);

/**
 * @brief Called when a task's state changes.
 */
static void state_change_hook(mu_task_t *task, mu_task_state_t from_state,
                              mu_task_state_t to_state);

// *****************************************************************************
// Public code

void task_info_init(void) {
    mu_task_install_call_hook(task_call_hook);
    mu_task_install_set_state_hook(state_change_hook);
    s_prev_task == NULL;
}

const char *task_info_task_name(mu_task_t *task) {
    task_info_t *info = NULL;
    const char *task_name = NULL;

    // Fetch the user_info associated with this task.
    if (task && (info = (task_info_t *)mu_task_get_user_info(task)) != NULL) {
        task_name = info->task_name;
    }
    if (task_name == NULL) {
        return "unknown_task";
    } else {
        return task_name;
    }
}

const char *task_info_state_name(mu_task_t *task, mu_task_state_t state) {
    task_info_t *info = NULL;
    const char *state_name = NULL;

    if (task && (info = (task_info_t *)mu_task_get_user_info(task)) != NULL) {
        if (state < info->n_states) {
            state_name = info->state_names[state];
        } else {
            state_name = "unknown_state";
        }
    }
    return state_name;
}

void task_info(mu_task_t *task, mu_task_state_t terminal_state,
               bool had_error) {
    if (had_error) {
        MU_LOG_WARN("%s => %s",
                    task_info_state_name(task, mu_task_get_state(task)),
                    task_info_state_name(task, terminal_state));
    }
    mu_task_transfer(task, terminal_state, continuation);
}

// *****************************************************************************
// Private (static) code

static void task_call_hook(mu_task_t *task) {
    // If transitioning to another task, log task names at MU_LOG_DEBUG level
    if (!bb_log_is_reporting(MU_LOG_LEVEL_DEBUG)) {
        return;
    }

    if ((task != idle_task()) && (task != s_prev_task)) {
        // Log the transition from prev_task to task
        const char *from_task_name = task_info_task_name(s_prev_task);
        const char *to_task_name = task_info_task_name(task);
        MU_LOG_DEBUG("%s => %s", from_task_name, to_task_name);
        s_prev_task = task;
    }
}

static void state_change_hook(mu_task_t *task, mu_task_state_t from_state,
                              mu_task_state_t to_state) {
    // If transitioning to another state, log state names at MU_LOG_DEBUG level
    if (!bb_log_is_reporting(MU_LOG_LEVEL_DEBUG)) {
        return;
    }

    if (mu_task_get_user_info(task) != NULL) {
        const char *task_name = task_info_task_name(task);
        const char *from_state_name = task_info_state_name(task, from_state);
        const char *to_state_name = task_info_state_name(task, to_state);
        MU_LOG_DEBUG("%s: %s => %s", task_name, from_state_name, to_state_name);
    }
}

// *****************************************************************************
// End of file
