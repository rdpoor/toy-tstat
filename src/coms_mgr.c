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

/**
 * This implementation of coms_mgr provides an interface to the Microchip
 * MPLAB.X / Harmony3 serial ring buffer library object.
 */

// *****************************************************************************
// Includes

#include "coms_mgr.h"

#include "definitions.h"
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
    return SERCOM3_USART_Write(msg, msg_len) = msg_len;
}

bool coms_mgr_recv(char *buf, size_t capacity, mu_task_t *on_completion) {
    coms_mgr_t *self = coms_mgr();
    mu_task_t *task = coms_mgr_task();

    MU_LOG_DEBUG("coms_mgr: recv");
    self->buf = buf;
    self->capacity = capacity;
    self->on_completion = on_completion;

    self->bytes_received = 0;
    self->had_error = false;
    mu_task_yield(task, COMS_MGR_STATE_START_RQST);
}

bool coms_mgr_had_error(void) { return coms_mgr()->had_error; }

// *****************************************************************************
// Private (static) code

static void coms_mgr_fn(mu_task_t *task, void *arg) {
    coms_mgr_t *self = coms_mgr();
    (void)arg; // unused

    switch (mu_task_get_state(task)) {

    case COMS_MGR_STATE_IDLE: {
        // wait here for a call to coms_mgr_recv()
    } break;

    case COMS_MGR_STATE_START_RQST: {
        // task will be advanced by coms_rx_cb()
        mu_task_wait(task, COMS_MGR_STATE_AWAIT_RQST);
    } break;

    case COMS_MGR_STATE_AWAIT_RQST: {
        // here after after coms_rx_bc interrupt triggers
        switch (self->event) {

        case SERCOM_USART_EVENT_READ_THRESHOLD_REACHED:
        case SERCOM_USART_EVENT_READ_BUFFER_FULL: {
            /* bytes are available in the receive ring buffer */
            char ch;

            while ((self->bytes_received < self->capacity) &&
                   ((SERCOM3_USART_Read(&ch, 1) == 1))) {
                // quit on null terminator or full buffer
                self->buf[self->bytes_received++] = ch;
                if (ch == '\0') {
                    break;
                }
            }
            // If null terminator or buffer full, call continuation.  Else
            // wait for more characters.
            if ((ch == '\0') || (self->bytes_received == self->capacity)) {
                endgame(false);
            } else {
                mu_task_wait(task, COMS_MGR_STATE_AWAIT_RQST);
            }
        } break;

        case SERCOM_USART_EVENT_READ_ERROR: {
            /* USART error. Application must call the SERCOMx_USART_ErrorGet API to get the type of error and clear the error. */
            endgame(true);
        } break;

        case SERCOM_USART_EVENT_WRITE_THRESHOLD_REACHED: {
            /* Threshold number of free space is available in the transmit ring buffer */
        } break;

        case SERCOM_USART_EVENT_BREAK_SIGNAL_DETECTED: {
            /* Receive break signal is detected */
        } break;

        }  // switch
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


static coms_rx_cb(SERCOM_USART_EVENT event, uintptr_t context) {
    coms_mgr_t *self = (coms_mgr_t *)context;
    mu_task_t *task = &self->task;
    self->event = event;

    mu_task_sched_from_isr(task);
}

// *****************************************************************************
// End of file
