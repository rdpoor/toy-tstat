/**
 * @file ctrl_local.c
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

#include "ctrl_local.h"

#include "ctrl.h"
#include <stdbool.h>
#include <stdint.h>

// *****************************************************************************
// Private types and definitions

typedef struct {
    int setpoint;
    system_mode_t system_mode;
} ctrl_ctx_t;

// *****************************************************************************
// Private (static, forward) declarations

// *****************************************************************************
// Private (static) storage

static const ctrl_api_t s_ctrl_api = {
    .set_setpoint = set_setpoint,
    .incr_setpoint = incr_setpoint,
    .set_system_mode = set_system_mode,
    .on_tick = on_tick
};

static ctrl_ctx_t s_ctrl_ctx;

static const ctrl_t s_ctrl = {
    .api = s_ctrl_api,
    .ctx = s_ctrl_ctx
};

// *****************************************************************************
// Public code

void ctrl_set_setpoint(ctrl_t *ctrl, int setpoint) {
    ctrl->ctx.setpoint = setpoint;
}

void ctrl_incr_setpoint(ctrl_t *ctrl, bool up) {
    ctrl->ctx.setpoint += up ? 1 : -1;
}

void ctrl_set_system_mode(ctrl_t *ctrl, system_mode_t system_mode) {
    ctrl->ctx.system_mode = system_mode;
}

void ctrl_on_tick(ctrl_t *ctrl) {
    mu_task_yield(&ctrl->ctx.task, CTRL_STATE_START_AMBIENT_REQ);
}

// *****************************************************************************
// Private (static) code

void ctrl_fn(mu_task_t *task, void *arg) {
    ctrl_ctx_t *self = MU_TASK_CTX(task, ctrl_ctx_t, task);
    switch(mu_task_get_state(task)) {

    case CTRL_STATE_IDLE: {
        // remain here until a call to on_tick
    } break;

    case CTRL_STATE_START_AMBIENT_REQ: {
        // Here on a call to on_tick.  Start async request tstat state
        tstat_get_state(self->tstat, task);
        mu_task_wait(task, CTRL_STATE_AWAIT_AMBIENT_REQ);
    } break;

    case CTRL_STATE_AWAIT_AMBIENT_REQ: {
        // Here when tstat state is available or on error.
        tstat_state_t *tstat_state = tstat_get_state(self->tstat);
        if (tstat_state != NULL) {
            ctrl_process(self, tstat_state);
            mu_task_yield(task, CTRL_STATE_IDLE);
        } else {
            // TODO: handle async error
            mu_task_yield(task, CTRL_STATE_IDLE);
        }
    } break;

    }  // switch
}

static void ctrl_process(ctrl_ctx_t *self, tstat_state_t *tstat_state) {
    // The main logic of the tstat happens here.
    tstat_
    if (self->system_mode == SYSTEM_MODE_OFF) {

    }
    ui_state_t ui_state = {
        .setpoint =
        .ambient = tstat_state->ambient,
    }
}

// *****************************************************************************
// End of file
