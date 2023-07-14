/**
 * @file model_iface.c
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

#include "model_iface.h"

#include "mulib/core/mu_task.h"
#include "mulib/extras/mu_log.h"
#include "task_info.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// *****************************************************************************
// Private types and definitions

#define MODEL_IFACE_STATES(M)                                                  \
    M(MODEL_IFACE_STATE_IDLE)                                                  \
    M(MODEL_IFACE_STATE_START_RQST)                                            \
    M(MODEL_IFACE_STATE_AWAIT_RQST)

#define EXPAND_STATE_ENUMS(_name) _name,
typedef enum { MODEL_IFACE_STATES(EXPAND_STATE_ENUMS) } model_iface_state_t;

typedef struct {
    mu_task_t task;           // the model_iface task object
    mu_task_t *on_completion; // called on completion or error
    bool had_error;
} model_iface_t;

// *****************************************************************************
// Private (static) storage

#define EXPAND_STATE_NAMES(_name) #_name,
static const char *s_state_names[] = {MODEL_IFACE_STATES(EXPAND_STATE_NAMES)};
#define N_STATES (sizeof(s_state_names) / sizeof(s_state_names[0]))

/**
 * @brief the (singleton) instance of the model_iface context.
 */
static model_iface_t s_model_iface;

/**
 * @brief A task_info structure for use by task_info.c
 */
static task_info_t s_task_info = {
    .task_name = "model_iface",
    .state_names = s_state_names,
    .n_states = N_STATES,
};

// *****************************************************************************
// Private (static, forward) declarations

/**
 * @brief Return a reference to the singleton model_iface context.
 */
static inline model_iface_t *model_iface(void) { return &s_model_iface; }

/**
 * @brief Return a reference to the model_iface task.
 */
static inline mu_task_t *model_iface_task(void) { return &s_model_iface.task; }

/**
 * @brief The primary state machine for the model_iface task.
 */
static void model_iface_fn(mu_task_t *task, void *arg);

/**
 * @brief Set terminal state and invoke on_completion task.
 */
static void endgame(bool had_error);

// *****************************************************************************
// Public code

void model_iface_init(void) {
    mu_task_init(model_iface_task(), model_iface_fn, MODEL_IFACE_STATE_IDLE,
                 &s_task_info);
}

/**
 * @brief Pull remote model state into local model.
 */
bool model_iface_pull(model_t *model, mu_task_t *on_completion) {
    model_iface_t *self = model_iface();
    mu_task_t *task = model_iface_task();

    MU_LOG_DEBUG("model_iface: fatch");
    self->model = model;
    self->on_completion = on_completion;
    self->had_error = false;
    mu_task_yield(task, MODEL_IFACE_STATE_START_RQST);
}

bool model_iface_push(model_t *model) {
    char buf[MAX_LEN];
    // serialize and send the model.
    if (model_dump_json(model, buf, sizeof(buf)) == NULL) {
        return false;
    } else {
        coms_mgr_send(buf, strlen(buf));
        return true;
    }
}

bool model_iface_had_error(void) { return model_iface()->had_error; }

// *****************************************************************************
// Private (static) code

static void model_iface_fn(mu_task_t *task, void *arg) {
    model_iface_t *self = model_iface();
    (void)arg; // unused

    switch (mu_task_get_state(task)) {

    case MODEL_IFACE_STATE_IDLE: {
        // wait here for a call to model_iface_fatch()
    } break;

    case MODEL_IFACE_STATE_START_PULL: {
        // request model state
        coms_mgr_send(MODEL_REQUEST, sizeof(MODEL_REQUEST));

        if (!coms_mgr_recv(self->rx_buf, sizeof(self->rx_buf))) {
            MU_LOG_ERROR("model_iface: failed to start receiving message");
            endgame(true);
        } else {
            mu_task_wait(task, MODEL_IFACE_STATE_AWAIT_RQST);
        }
    } break;

    case MODEL_IFACE_STATE_AWAIT_PULL: {
        // here after receiving serial response
        if (coms_mgr_had_error()) {
            MU_LOG_ERROR("model_iface: failed to receive message");
            endgame(true);
        } else if (model_load_json(self->model, self->rx_buf) == NULL) {
            MU_LOG_ERROR("model_iface: failed parse model JSON");
            endgame(true);
        } else {
            MU_LOG_DEBUG("model_iface: success");
            endame(false);
        }
    } break;

    } // switch
}

static void endgame(bool had_error) {
    model_iface_t *self = model_iface();
    mu_task_t *task = model_iface_task();

    self->had_error = had_error;
    task_info(task, MODEL_IFACE_STATE_IDLE, had_error);
    mu_task_transfer(task, MODEL_IFACE_STATE_IDLE, self->on_completion);
}

// *****************************************************************************
// End of file
