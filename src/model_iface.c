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

#define MAX_JEMS_LEVELS 3

#define MAX_JSMN_TOKENS 13

typedef struct {
    char *buf;
    size_t buflen;
    size_t written;
} writer_state_t;


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

/**
 * @brief Write a byte to the JSON output buffer.
 */
static void jems_writer(char ch, uintptr_t arg);

static const char *system_mode_to_string(system_mode_t system_mode);

static bool is_odd(int n);

/**
 * @brief Return true if token is of type string and equals str
 */
static bool token_streq(const char *json, jsmntok_t *token, const char *str);

/**
 * @brief Set dst to the integer value indicated by token.
 *
 * @return true on success.
 */
static bool update_int(const char *json, jsmntok_t *token, int *dst);

/**
 * @brief Set dst to the boolean value indicated by token.
 *
 * @return true on success.
 */
static bool update_bool(const char *json, jsmntok_t *token, bool *dst);

/**
 * @brief Set dst to the value[] corresponding to the name[] indicated by token.
 *
 * @return true on success.
 */
static bool update_enum(const char *json, jsmntok_t *token, const char *names[],
                        const int values[], size_t n_elements, int *dst);


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

tstat_model_t *tstat_model_load_json(tstat_model_t *tstat_model, const char *json) {
    jsmntok_t tokens[MAX_JSMN_TOKENS];
    jsmn_parser jsmn_parser;

    jsmn_init(&jsmn_parser);
    int result =
        jsmn_parse(&jsmn_parser, json, strlen(json), tokens, MAX_JSMN_TOKENS);
    if (result < 1) {
        // zero length or un-parsable JSON string
        return NULL;
    } else if (tokens[0].type != JSMN_OBJECT) {
        // JSON string must have the form of a JSON object { ... }
        return NULL;
    } else if (!is_odd(result)) {
        // must have an odd number of tokens (object + N * key/value pairs)
        return NULL;
    }
    for (int i = 1; i < result; i += 2) {
        jsmntok_t *key = &tokens[i];
        jsmntok_t *value = &tokens[i + 1];

        if (key->type != JSMN_STRING) {
            // expected object key to be a string.
            return NULL;
        }
        if (token_streq(json, key, "ambient")) {
            if (!update_int(json, value, &tstat_model->ambient)) {
                return NULL;
            }
        } else if (token_streq(json, key, "cool_setpoint")) {
            if (!update_int(json, value, &tstat_model->cool_setpoint)) {
                return NULL;
            }
        } else if (token_streq(json, key, "heat_setpoint")) {
            if (!update_int(json, value, &tstat_model->heat_setpoint)) {
                return NULL;
            }
        } else if (token_streq(json, key, "relay_y")) {
            if (!update_bool(json, value, &tstat_model->relay_y)) {
                return NULL;
            }
        } else if (token_streq(json, key, "relay_w")) {
            if (!update_bool(json, value, &tstat_model->relay_w)) {
                return NULL;
            }
        } else if (token_streq(json, key, "system_mode")) {
            const char *sys_names[] = {"SYSTEM_MODE_OFF", "SYSTEM_MODE_COOL",
                                       "SYSTEM_MODE_HEAT"};
            int sys_values[] = {SYSTEM_MODE_OFF, SYSTEM_MODE_COOL,
                                SYSTEM_MODE_HEAT};
            if (!update_enum(json, value, sys_names, sys_values, 3,
                             &tstat_model->system_mode)) {
                return NULL;
            }
        } else {
            // ignore unrecognized fields.
        }
    }
    return tstat_model;
}

const char *tstat_model_dump_json(tstat_model_t *tstat_model, char *buf, size_t buflen) {
    // It's safe to allotate writer_state and jems_levels on the stack because
    // we stay within dynamic scope of this function until JSON has been
    // written. Use buflen-1 to always leave space for null termination.
    writer_state_t writer_state = {
        .buf = buf, .buflen = buflen - 1, .written = 0};
    jems_level_t jems_levels[MAX_JEMS_LEVELS];
    jems_t jems;

    jems_init(&jems, jems_levels, MAX_JEMS_LEVELS, jems_writer,
              (uintptr_t)(&writer_state));
    jems_object_open(&jems);
    jems_key_number(&jems, "ambient", tstat_model_get_ambient(tstat_model));
    jems_key_number(&jems, "cool_setpoint", tstat_model_get_cool_setpoint(tstat_model));
    jems_key_number(&jems, "heat_setpoint", tstat_model_get_heat_setpoint(tstat_model));
    jems_key_bool(&jems, "relay_y", tstat_model_get_relay_y(tstat_model));
    jems_key_bool(&jems, "relay_w", tstat_model_get_relay_w(tstat_model));
    jems_key_string(&jems, "system_mode",
                    system_mode_to_string(tstat_model_get_system_mode(tstat_model)));
    jems_object_close(&jems);
    // null terminate the string in buf
    buf[writer_state.written] = '\0';
    return buf;
}


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

static void jems_writer(char ch, uintptr_t arg) {
    writer_state_t *writer_state = (writer_state_t *)arg;
    if (writer_state->written < writer_state->buflen) {
        writer_state->buf[writer_state->written++] = ch;
    }
}

static const char *system_mode_to_string(system_mode_t system_mode) {
    const char *s = "UNKNOWN";
    switch (system_mode) {
    case SYSTEM_MODE_OFF: {
        s = "SYSTEM_MODE_OFF";
    } break;
    case SYSTEM_MODE_COOL: {
        s = "SYSTEM_MODE_COOL";
    } break;
    case SYSTEM_MODE_HEAT: {
        s = "SYSTEM_MDOE_HEAT";
    } break;
    }
    return s;
}

static bool is_odd(int n) { return (n & 1) == 1; }

static bool token_streq(const char *json, jsmntok_t *token, const char *str) {
    if (token->type != JSMN_STRING) {
        return false;
    }
    size_t len = token->end - token->start;
    return strncmp(&json[token->start], str, len) == 0;
}

static bool update_int(const char *json, jsmntok_t *token, int *dst) {
    if (token->type != JSMN_PRIMITIVE) {
        return false;
    }
    *dst = atoi(&json[token->start]);
    return true;
}

static bool update_bool(const char *json, jsmntok_t *token, bool *dst) {
    if (token->type != JSMN_PRIMITIVE) {
        return false;
    }
    size_t len = token->end - token->start;
    if (strncmp(&json[token->start], "true", len) == 0) {
        *dst = true;
        return true;
    } else if (strncmp(&json[token->start], "false", len) == 0) {
        *dst = false;
        return true;
    } else {
        return false;
    }
}

static bool update_enum(const char *json, jsmntok_t *token, const char *names[],
                        const int values[], size_t n_elements, int *dst) {
    size_t len = token->end - token->start;
    for (int i = 0; i < n_elements; i++) {
        if (strncmp(&json[token->start], names[i], len) == 0) {
            *dst = values[i];
            return true;
        }
    }
    return false;
}


// *****************************************************************************
// End of file

// #define TSTAT_MODEL_TESTS
#ifdef TSTAT_MODEL_TESTS

#include <stdio.h>

#define BUF_LEN 200

int main(void) {
    char buf[BUF_LEN];
    tstat_model_t tstat_model;

    tstat_model_load_json(&tstat_model,
                    "{\"ambient\":2000, \"cool_setpoint\":1980, "
                    "\"heat_setpoint\":2020, \"relay_y\":true, "
                    "\"relay_w\":false, \"system_mode\":\"SYSTEM_MODE_COOL\"}");
    tstat_model_dump_json(&tstat_model, buf, sizeof(buf));
    printf("\n%s", buf);
    tstat_model_load_json(&tstat_model, "{\"relay_y\":false, \"system_mode\":\"SYSTEM_MODE_OFF\"}");
    tstat_model_dump_json(&tstat_model, buf, sizeof(buf));
    printf("\n%s", buf);
    printf("\n");
}
#endif

/*
clear ; gcc -Wall -o tstat_model tstat_model.c jsmn.c jems.c ; ./tstat_model ; rm -f ./tstat_model
 */
