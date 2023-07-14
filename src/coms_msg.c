/**
 * @file coms_mgr.c
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

#include "coms_mgr.h"

#include "mulib/core/mu_task.h"
#include "mulib/extras/mu_log.h"
#include "task_info.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// *****************************************************************************
// Private types and definitions

#define COMS_MGR_STATES(M)                                                  \
    M(COMS_MGR_STATE_IDLE)                                                  \
    M(COMS_MGR_STATE_START_RQST)                                            \
    M(COMS_MGR_STATE_AWAIT_RQST)

#define EXPAND_STATE_ENUMS(_name) _name,
typedef enum { COMS_MGR_STATES(EXPAND_STATE_ENUMS) } coms_mgr_state_t;

typedef struct {
    mu_task_t task;           // the coms_mgr task object
    mu_task_t *on_completion; // called on completion or error
    bool had_error;
} coms_mgr_t;

// *****************************************************************************
// Private (static) storage

#define EXPAND_STATE_NAMES(_name) #_name,
static const char *s_state_names[] = {COMS_MGR_STATES(EXPAND_STATE_NAMES)};
#define N_STATES (sizeof(s_state_names) / sizeof(s_state_names[0]))

/**
 * @brief the (singleton) instance of the coms_mgr context.
 */
static coms_mgr_t s_coms_mgr;

/**
 * @brief A task_info structure for use by task_info.c
 */
static task_info_t s_task_info = {
    .task_name = "coms_mgr",
    .state_names = s_state_names,
    .n_states = N_STATES,
};

// *****************************************************************************
// Private (static, forward) declarations

/**
 * @brief Return a reference to the singleton coms_mgr context.
 */
static inline coms_mgr_t *coms_mgr(void) { return &s_coms_mgr; }

/**
 * @brief Return a reference to the coms_mgr task.
 */
static inline mu_task_t *coms_mgr_task(void) { return &s_coms_mgr.task; }

/**
 * @brief The primary state machine for the coms_mgr task.
 */
static void coms_mgr_fn(mu_task_t *task, void *arg);

/**
 * @brief Set terminal state and invoke on_completion task.
 */
static void endgame(bool had_error);

// *****************************************************************************
// Public code

void coms_mgr_init(void) {
    mu_task_init(coms_mgr_task(), coms_mgr_fn, COMS_MGR_STATE_IDLE,
                 &s_task_info);
}

bool coms_mgr_send(const char *msg, size_t msg_len) {
    printf("%.*s\0", msg_len, msg);
    return true;
}

bool coms_mgr_recv(char *buf, size_t capacity, mu_task_t *on_completion) {
    coms_mgr_t *self = coms_mgr();
    mu_task_t *task = coms_mgr_task();

    MU_LOG_DEBUG("coms_mgr: fatch");
    self->buf = buf;
    self->capacity = capacity;
    self->on_completion = on_completion;

    self->had_error = false;
    mu_task_yield(task, COMS_MGR_STATE_START_RQST);
}

bool coms_mgr_push(model_t *model) {
    char buf[MAX_LEN];
    // serialize and send the model.
    if (model_dump_json(model, buf, sizeof(buf)) == NULL) {
        return false;
    } else {
        coms_mgr_send(buf, strlen(buf));
        return true;
    }
}

bool coms_mgr_had_error(void) { return coms_mgr()->had_error; }

// *****************************************************************************
// Private (static) code

static void coms_mgr_fn(mu_task_t *task, void *arg) {
    coms_mgr_t *self = coms_mgr();
    (void)arg; // unused

    switch (mu_task_get_state(task)) {

    case COMS_MGR_STATE_IDLE: {
        // wait here for a call to coms_mgr_fatch()
    } break;

    case COMS_MGR_STATE_START_PULL: {
        // request model state
        coms_mgr_send(MODEL_REQUEST, sizeof(MODEL_REQUEST));

        if (!coms_mgr_recv(self->rx_buf, sizeof(self->rx_buf))) {
            MU_LOG_ERROR("coms_mgr: failed to start receiving message");
            endgame(true);
        } else {
            mu_task_wait(task, COMS_MGR_STATE_AWAIT_RQST);
        }
    } break;

    case COMS_MGR_STATE_AWAIT_PULL: {
        // here after receiving serial response
        if (coms_mgr_had_error()) {
            MU_LOG_ERROR("coms_mgr: failed to receive message");
            endgame(true);
        } else if (model_load_json(self->model, self->rx_buf) == NULL) {
            MU_LOG_ERROR("coms_mgr: failed parse model JSON");
            endgame(true);
        } else {
            MU_LOG_DEBUG("coms_mgr: success");
            endame(false);
        }
    } break;

    } // switch
}

static void endgame(bool had_error) {
    coms_mgr_t *self = coms_mgr();
    mu_task_t *task = coms_mgr_task();

    self->had_error = had_error;
    task_info(task, COMS_MGR_STATE_IDLE, had_error);
    mu_task_transfer(task, COMS_MGR_STATE_IDLE, self->on_completion);
}

// *****************************************************************************
// End of file
