/**
 * @file tstat_model.h
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
 */

 /**
  * @brief tstat_model embodies the internal state of a simple thermostat.
  */

#ifndef _TSTAT_MODEL_H_
#define _TSTAT_MODEL_H_

// *****************************************************************************
// Includes

#include <stdbool.h>
#include <stddef.h>

// *****************************************************************************
// C++ compatibility

#ifdef __cplusplus
extern "C" {
#endif

// *****************************************************************************
// Public types and definitions

typedef int temperature_t;

typedef enum {
    SYSTEM_MODE_OFF,
    SYSTEM_MODE_COOL,
    SYSTEM_MODE_HEAT,
} system_mode_t;

typedef struct {
    temperature_t ambient;
    temperature_t cool_setpoint;
    temperature_t heat_setpoint;
    bool relay_y;
    bool relay_w;
    int system_mode;
} tstat_model_t;

// *****************************************************************************
// Public declarations

/**
 * @brief Parse a JSON string and update fields of the tstat_model.
 *
 * JSON is a C-style null terminated string  It contains a JSON object
 * with zero or more fields of the form:
 *
 *   {'ambient':<number>, 'relay_y':<bool>, ...}
 *
 * The fields of tstat_model are updated from the values read in the JSON string.
 * NOTE: unrecognized fields are silently ignored.
 *
 * @return tstat_model if the JSON was valid, or NULL otherwise.
 */
tstat_model_t *tstat_model_load_json(tstat_model_t *tstat_model, const char *json);

const char *tstat_model_dump_json(tstat_model_t *tstat_model, char *buf, size_t buflen);

temperature_t tstat_model_get_ambient(tstat_model_t *tstat_model);

temperature_t tstat_model_get_cool_setpoint(tstat_model_t *tstat_model);

void tstat_model_set_cool_setpoint(tstat_model_t *tstat_model, temperature_t cool_setpoint);

temperature_t tstat_model_get_heat_setpoint(tstat_model_t *tstat_model);

void tstat_model_set_heat_setpoint(tstat_model_t *tstat_model, temperature_t heat_setpoint);

bool tstat_model_get_relay_y(tstat_model_t *tstat_model);

void tstat_model_set_relay_y(tstat_model_t *tstat_model, bool energized);

bool tstat_model_get_relay_w(tstat_model_t *tstat_model);

void tstat_model_set_relay_w(tstat_model_t *tstat_model, bool energized);

system_mode_t tstat_model_get_system_mode(tstat_model_t *tstat_model);

void tstat_model_set_system_mode(tstat_model_t *tstat_model, system_mode_t system_mode);

// *****************************************************************************
// End of file

#ifdef __cplusplus
}
#endif

#endif /* #ifndef _TSTAT_MODEL_H_ */
