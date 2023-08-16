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

#include <stdbool.h>
#include <stddef.h>

// *****************************************************************************
// Private types and definitions

// *****************************************************************************
// Private (static) storage

// *****************************************************************************
// Private (static, forward) declarations

// *****************************************************************************
// Public code

temperature_t tstat_model_get_ambient(tstat_model_t *tstat_model) {
    return tstat_model->ambient;
}

temperature_t tstat_model_get_cool_setpoint(tstat_model_t *tstat_model) {
    return tstat_model->cool_setpoint;
}

void tstat_model_set_cool_setpoint(tstat_model_t *tstat_model,
                                   temperature_t cool_setpoint) {
    tstat_model->cool_setpoint = cool_setpoint;
}

temperature_t tstat_model_get_heat_setpoint(tstat_model_t *tstat_model) {
    return tstat_model->heat_setpoint;
}

void tstat_model_set_heat_setpoint(tstat_model_t *tstat_model,
                                   temperature_t heat_setpoint) {
    tstat_model->heat_setpoint = heat_setpoint;
}

bool tstat_model_get_relay_y(tstat_model_t *tstat_model) {
    return tstat_model->relay_y;
}

void tstat_model_set_relay_y(tstat_model_t *tstat_model, bool energized) {
    tstat_model->relay_y = energized;
}

bool tstat_model_get_relay_w(tstat_model_t *tstat_model) {
    return tstat_model->relay_w;
}

void tstat_model_set_relay_w(tstat_model_t *tstat_model, bool energized) {
    tstat_model->relay_w = energized;
}

system_mode_t tstat_model_get_system_mode(tstat_model_t *tstat_model) {
    return tstat_model->system_mode;
}

void tstat_model_set_system_mode(tstat_model_t *tstat_model,
                                 system_mode_t system_mode) {
    tstat_model->system_mode = system_mode;
}

// *****************************************************************************
// Private (static) code

// *****************************************************************************
// End of file
