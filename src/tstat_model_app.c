/**
 * @file tstat_model_app.c
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

#include "tstat_model_app.h"

#include "mulib/core/mu_sched.h"
#include "mulib/core/mu_task.h"
#include <stdbool.h>
#include <stdint.h>

// *****************************************************************************
// Private types and definitions

#define APP_STATES(M)                                                          \
    M(APP_STATE_INIT)                                                          \
    M(APP_STATE_ERR)

#define EXPAND_STATE_ENUMS(_name) _name,
typedef enum { APP_STATES(EXPAND_STATE_ENUMS) } app_state_t;

typedef struct {
    mu_task_t task;            // the app task object
} app_t;

// *****************************************************************************
// Private (static) storage

#define EXPAND_STATE_NAMES(_name) #_name,
static const char *s_state_names[] = {APP_STATES(EXPAND_STATE_NAMES)};
#define N_STATES (sizeof(s_state_names) / sizeof(s_state_names[0]))

/**
 * @brief Globablly available general purpose transmit and receive buffers
 */
static char s_tx_buf[APP_TX_BUF_SIZE];
static char s_rx_buf[APP_RX_BUF_SIZE];

/**
 * @brief the (singleton) instance of the app context.
 */
static app_t s_app;

/**
 * @brief A task_info structure for use by task_info.c
 */
static task_info_t s_task_info = {
    .task_name = "app",
    .state_names = s_state_names,
    .n_states = N_STATES,
};

// *****************************************************************************
// Private (static, forward) declarations

/**
 * @brief Return a reference to the app context.
 */
static inline app_t *app(void) { return &s_app; }

/**
 * @brief Return a reference to the app task.
 */
static inline mu_task_t *app_task(void) { return &s_app.task; }

/**
 * @brief The primary state machine for the app task.
 */
static void app_fn(mu_task_t *task, void *arg);

// *****************************************************************************
// Public code

void tstat_model_app_init(void) {
    mu_sched_init();

    mu_task_init(app_task(), app_fn, APP_STATE_INIT, &s_task_info);
    // By convention, a task's xxx_init function does not invoke the scheduler.
    // But this top level entry point is the exception: calling mu_task_yield()
    // starts everything going.
    mu_task_yield(app_task(), APP_STATE_INIT);
}

void tstat_model_app_tasks(void) {
    mu_sched_step(void);
}

// *****************************************************************************
// Private (static) code

static void app_fn(mu_task_t *task, void *arg) {
    app_ctx_t *self = app_ctx();
    (void)arg; // unused

    switch (mu_task_get_state(task)) {

    case APP_STATE_INIT: {
        // Arrive here at startup.
        line_reader_init(&s_line_reader, s_rx_buf, sizeof(s_rx_buf));
    } break;

    case APP_STATE_START_READ: {
        // Arrive here to start reading from serial input / stdin
        if (!line_reader_get_line(&s_line_reader, task)) {
            MU_LOG_ERROR("Unable to open line reader for input");
            mu_task_yield(task, APP_STATE_ERR);
        } else {
            mu_task_wait(task, APP_STATE_AWAIT_READ);
        }
    } break;

    case APP_STATE_AWAIT_READ: {
        // Arrive here with a newline terminated string in s_rx_buf (or error)
        if (line_reader_had_error(&s_line_reader)) {
            MU_LOG_ERROR("Error while trying to read line");
            mu_task_yield(task, APP_STATE_ERR);
        } else {
            // process received data and loop back for more.
            process_line(line_reader_data(&s_line_reader),
                         line_reader_data_available(&s_line_reader));
            mu_task_yield(task, APP_STATE_START_READ);
        }
    } break;

    case APP_STATE_ERR: {
        // Arrive here on some error.  In this app, just restart...
        mu_task_yield(task, APP_STATE_AWAIT_READ);
    } break;

    } // switch
}

static void process_line(const char *line, size_t line_length) {
    // expect line to be JSON of the form:
    //
    // 0     1            2      3                 4       5  6
    // {"topic":"<something>", "fn":"get_tstat_model", "args":{}}
    //
    // 0     1            2      3                 4       5  6
    // {"topic":"<something>", "fn":"set_tstat_model", "args":<values>}
    //
    // ... where <values> is a JSON object that names zero or more slot / value
    // pairs:
    //
    // {"ambient":<n>, "cool_setpoint":<n>, etc}

}
// *****************************************************************************
// End of file
