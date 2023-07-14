/**
 * @file tstat_app.c
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

#include "tstat_app.h"

#include "mulib/core/mu_task.h"
#include "mulib/extras/mu_log.h"
#include "task_info.h"
#include "timeout_mgr.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// *****************************************************************************
// Private types and definitions

#define TSTAT_APP_STATES(M)                                                    \
    M(TSTAT_APP_STATE_IDLE)                                                    \
    M(TSTAT_APP_STATE_START_PULL_MODEL)                                        \
    M(TSTAT_APP_STATE_AWAIT_PULL_MODEL)                                        \
    M(TSTAT_APP_STATE_UPDATE_MODEL)                                            \
    M(TSTAT_APP_STATE_START_PUSH_MODEL)                                        \
    M(TSTAT_APP_STATE_AWAIT_PUSH_MODEL)

#define EXPAND_STATE_ENUMS(_name) _name,
typedef enum { TSTAT_APP_STATES(EXPAND_STATE_ENUMS) } tstat_app_state_t;

typedef struct {
    mu_task_t task;           // the tstat_app task object
    mu_task_t *on_completion; // called on completion or error
    model_t model;            // local copy of model state
    bool had_error;
} tstat_app_t;

// *****************************************************************************
// Private (static) storage

#define EXPAND_STATE_NAMES(_name) #_name,
static const char *s_state_names[] = {TSTAT_APP_STATES(EXPAND_STATE_NAMES)};
#define N_STATES (sizeof(s_state_names) / sizeof(s_state_names[0]))

/**
 * @brief the (singleton) instance of the tstat_app context.
 */
static tstat_app_t s_tstat_app;

/**
 * @brief A task_info structure for use by task_info.c
 */
static task_info_t s_task_info = {
    .task_name = "tstat_app",
    .state_names = s_state_names,
    .n_states = N_STATES,
};

// *****************************************************************************
// Private (static, forward) declarations

/**
 * @brief Return a reference to the singleton tstat_app context.
 */
static inline tstat_app_t *tstat_app(void) { return &s_tstat_app; }

/**
 * @brief Return a reference to the tstat_app task.
 */
static inline mu_task_t *tstat_app_task(void) { return &s_tstat_app.task; }

/**
 * @brief The primary state machine for the tstat_app task.
 */
static void tstat_app_fn(mu_task_t *task, void *arg);

/**
 * @brief Set terminal state and invoke on_completion task.
 */
static void endgame(bool had_error);

// *****************************************************************************
// Public code

void tstat_app_init(void) {
    mu_task_init(tstat_app_task(), tstat_app_fn, TSTAT_APP_STATE_IDLE,
                 &s_task_info);
}

void tstat_app_start(mu_task_t *on_completion) {
    tstat_app_t *self = tstat_app();
    mu_task_t *task = tstat_app_task();

    MU_LOG_DEBUG("tstat_app: start");
    self->on_completion = on_completion;
    self->had_error = false;
    mu_task_yield(task, TSTAT_APP_STATE_START);
}

bool tstat_app_had_error(void) { return tstat_app()->had_error; }

// *****************************************************************************
// Private (static) code

static void tstat_app_fn(mu_task_t *task, void *arg) {
    tstat_app_t *self = tstat_app();
    (void)arg; // unused

    switch (mu_task_get_state(task)) {

    case TSTAT_APP_STATE_IDLE: {
        // wait here for a call to tstat_app_start()
    } break;

    case TSTAT_APP_STATE_START_PULL_MODEL: {
        // request model state
        if (!model_iface_pull(&self->model, task)) {
            MU_LOG_ERROR("tstat_app: failed to start pulling model");
            endgame(true);
        } else {
            mu_task_wait(task, TSTAT_APP_STATE_AWAIT_PULL_MODEL);
        }
    } break;

    case TSTAT_APP_STATE_AWAIT_PULL_MODEL: {
        // here with updated model state
        if (model_iface_had_error()) {
            MU_LOG_ERROR("tstat_app: failed to pull model");
            endgame(true);
        } else {
            // update_model and update model
            mu_task_yield(task, TSTAT_APP_STATE_START_UPDATE_MODEL);
        }
    } break;

    case TSTAT_APP_STATE_UPDATE_MODEL: {
        // update local copy of the model
        logic_update_model(&self->model);
        mu_task_yield(task, TSTAT_APP_STATE_START_PUSH_MODEL);
    } break;

    case TSTAT_APP_STATE_START_PUSH_MODEL: {
        // Here to push updated model state
        if (!model_iface_push(&self->model, task)) {
            MU_LOG_ERROR("tstat_app: failed to start pushting model");
            endgame(true);
        } else {
            mu_task_wait(task, TSTAT_APP_STATE_AWAIT_PUSH_MODEL);
        }
    } break;

    case TSTAT_APP_STATE_AWAIT_PUSH_MODEL: {
        // Here when model state has been broadcast
        if (model_iface_had_error()) {
            MU_LOG_ERROR("tstat_app: failed push model");
            endgame(true);
        } else {
            // Success: return to caller
            MU_LOG_DEBUG("tstat_app: finished");
            endgame(false);
        }
    } break;

    case TSTAT_APP_STATE_DONE: {
        // terminal state for various errors
    } break;

    } // switch
}

static void endgame(bool had_error) {
    tstat_app_t *self = tstat_app();
    self->had_error = had_error;
    task_info_endgame(tstat_app_task(), TSTAT_APP_STATE_IDLE, had_error,
                      self->on_completion);
}

// *****************************************************************************
// End of file
