/**
 * @file tstat_model.c
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

#include "tstat_model.h"

#include "jems.h"
#include "jsmn.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// *****************************************************************************
// Private types and definitions

#define MAX_JEMS_LEVELS 3

#define MAX_JSMN_TOKENS 13

typedef struct {
    char *buf;
    size_t buflen;
    size_t written;
} writer_state_t;

// *****************************************************************************
// Private (static) storage

// *****************************************************************************
// Private (static, forward) declarations

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

temperature_t tstat_model_get_ambient(tstat_model_t *tstat_model) { return tstat_model->ambient; }

temperature_t tstat_model_get_cool_setpoint(tstat_model_t *tstat_model) {
    return tstat_model->cool_setpoint;
}

void tstat_model_set_cool_setpoint(tstat_model_t *tstat_model, temperature_t cool_setpoint) {
    tstat_model->cool_setpoint = cool_setpoint;
}

temperature_t tstat_model_get_heat_setpoint(tstat_model_t *tstat_model) {
    return tstat_model->heat_setpoint;
}

void tstat_model_set_heat_setpoint(tstat_model_t *tstat_model, temperature_t heat_setpoint) {
    tstat_model->heat_setpoint = heat_setpoint;
}

bool tstat_model_get_relay_y(tstat_model_t *tstat_model) { return tstat_model->relay_y; }

void tstat_model_set_relay_y(tstat_model_t *tstat_model, bool energized) {
    tstat_model->relay_y = energized;
}

bool tstat_model_get_relay_w(tstat_model_t *tstat_model) { return tstat_model->relay_w; }

void tstat_model_set_relay_w(tstat_model_t *tstat_model, bool energized) {
    tstat_model->relay_w = energized;
}

system_mode_t tstat_model_get_system_mode(tstat_model_t *tstat_model) {
    return tstat_model->system_mode;
}

void tstat_model_set_system_mode(tstat_model_t *tstat_model, system_mode_t system_mode) {
    tstat_model->system_mode = system_mode;
}

// *****************************************************************************
// Private (static) code

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
