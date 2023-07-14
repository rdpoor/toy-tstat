/**
 * @file template_mgr.c
 *
 * MIT License
 *
 * Copyright (c) 2023 PRO1 IAQ, INC.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicens, and/or sell
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

#include "template_mgr.h"

#include "bb_log.h"
#include "mulib/core/mu_task.h"
#include "task_info.h"
#include "timeout_mgr.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// *****************************************************************************
// Private types and definitions

#define TEMPLATE_MGR_STATES(M)                                                 \
    M(TEMPLATE_MGR_STATE_IDLE)                                                 \
    M(TEMPLATE_MGR_STATE_START)                                                \
    M(TEMPLATE_MGR_STATE_AWAIT)                                                \
    M(TEMPLATE_MGR_STATE_SUCCESS)                                              \
    M(TEMPLATE_MGR_STATE_ERR)                                                  \
    M(TEMPLATE_MGR_STATE_ERR_START_FAILED)                                     \
    M(TEMPLATE_MGR_STATE_ERR_AWAIT_TIMED_OUT)                                  \
    M(TEMPLATE_MGR_STATE_ERR_AWAIT_FAILED)

#define EXPAND_STATE_ENUMS(_name) _name,
typedef enum { TEMPLATE_MGR_STATES(EXPAND_STATE_ENUMS) } template_mgr_state_t;

typedef struct {
    mu_task_t task;            // the template_mgr task object
    timeout_mgr_t timeout_mgr; // a retry timer for timeout processing.
    mu_task_t *on_completion;  // called on completion or error
} template_mgr_ctx_t;

// *****************************************************************************
// Private (static) storage

#define EXPAND_STATE_NAMES(_name) #_name,
static const char *s_state_names[] = {TEMPLATE_MGR_STATES(EXPAND_STATE_NAMES)};
#define N_STATES (sizeof(s_state_names) / sizeof(s_state_names[0]))

/**
 * @brief the (singleton) instance of the template_mgr context.
 */
static template_mgr_ctx_t s_template_mgr_ctx;

/**
 * @brief A task_info structure for use by task_info.c
 */
static task_info_t s_task_info = {
    .task_name = "template_mgr",
    .state_names = s_state_names,
    .n_states = N_STATES,
};

// *****************************************************************************
// Private (static, forward) declarations

/**
 * @brief Return a reference to the singleton template_mgr context.
 */
static inline template_mgr_ctx_t *template_mgr(void) {
    return &s_template_mgr_ctx;
}

/**
 * @brief Return a reference to the template_mgr task.
 */
static inline mu_task_t *template_mgr_task(void) {
    return &s_template_mgr_ctx.task;
}

/**
 * @brief The primary state machine for the template_mgr task.
 */
static void template_mgr_fn(mu_task_t *task, void *arg);

/**
 * @brief Perform any cleanup before transferring to on_completion task.
 */
static void endgame(template_mgr_state_t terminal_state);

// *****************************************************************************
// Public code

void template_mgr_init(void) {
    mu_task_init(template_mgr_task(), template_mgr_fn, TEMPLATE_MGR_STATE_IDLE,
                 &s_task_info);
    timeout_mgr_init(&backlog_mgr()->timeout_mgr, "template_mgr");
}

void template_mgr_start(mu_task_t *on_completion) {
    template_mgr_ctx_t *self = template_mgr();
    mu_task_t *task = template_mgr_task();

    self->on_completion = on_completion;
    mu_task_yield(task, TEMPLATE_MGR_STATE_START);
}

bool template_mgr_timed_out(void) {
    return false;
}

bool template_mgr_had_error(void) {
    return mu_task_get_state(template_mgr_task) >= TEMPLATE_MGR_STATE_ERR;
}

// *****************************************************************************
// Private (static) code

static void template_mgr_fn(mu_task_t *task, void *arg) {
    template_mgr_ctx_t *self = template_mgr();
    (void)arg; // unused

    switch (mu_task_get_state(task)) {

    case TEMPLATE_MGR_STATE_IDLE: {
        // wait here for a call to template_mgr_start()
    } break;

    case TEMPLATE_MGR_STATE_START: {
        if (!other_module_start(task)) {
            endgame(TEMPLATE_MGR_STATE_ERR_START_FAILED);
        } else {
            mu_task_wait(task, TEMPLATE_MGR_STATE_AWAIT);
            timeout_mgr_arm(&self->timeout_mgr, TEMPLATE_TIMEOUT_MS, 0, NULL,
                            task);
        }
    } break;

    case TEMPLATE_MGR_STATE_AWAIT: {
        // Here when other module completes or times out
        if (timeout_mgr_check(&self->timeout_mgr)) {
            // timed out waiting for file system mount
            endgame(TEMPLATE_MGR_STATE_ERR_AWAIT_TIMED_OUT);
        } else if (other_module_had_error()) {
            endgame(TEMPLATE_MGR_STATE_ERR_AWAIT_FAILED);
        } else {
            endgame(task, TEMPLATE_MGR_STATE_SUCCESS);
        }
    } break;

    case TEMPLATE_MGR_STATE_START_ARCHIVE: {
        template_err_t err = template_archive_report((const uint8_t *)self->report, self->report_len);
        if (err != TEMPLATE_ERR_NONE) {
            BB_LOG_ERROR("archiving of template file failed: %s", template_err_name(err));
        }
        // TODO: honor error return
        endgame(TEMPLATE_MGR_STATE_IDLE);
    } break;

    case TEMPLATE_MGR_STATE_ERR:
    case TEMPLATE_MGR_STATE_ERR_START_MOUNT_FILESYS_FAILED:
    case TEMPLATE_MGR_STATE_ERR_AWAIT_MOUNT_FILESYS_TIMED_OUT:
    case TEMPLATE_MGR_STATE_ERR_AWAIT_MOUNT_FILESYS_FAILED: {
        // terminal state for various errors
    } break;

    } // switch
}

static void endgame(template_mgr_state_t terminal_state) {
    mu_task_t *task = template_mgr_task();
    template_mgr_ctx_t *self = template_mgr();
    if (terminal_state >= TEMPLATE_MGR_STATE_ERR) {
        // if ending on an error state, log a warning
        BB_LOG_WARN("%s => %s",
                    task_info_state_name(task, mu_task_get_state(task)),
                    task_info_state_name(task, terminal_state));
    }
    mu_task_transfer(task, terminal_state, self->on_completion);
}

// *****************************************************************************
// End of file
