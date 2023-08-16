/**
 * @file tstat_logic.c
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

#include "tstat_logic.h"

#include "model.h"
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

// *****************************************************************************
// Private types and definitions

// *****************************************************************************
// Private (static) storage

// *****************************************************************************
// Private (static, forward) declarations

// *****************************************************************************
// Public code

tstat_model_t *tstat_logic_update_model(tstat_model_t *model_in,
                                        tstat_model_t *model_out) {
    memcpy(model_out, model_in, sizeof(tstat_model_t));

    switch (model_get_system_mode(model)) {
    case SYSTEM_MODE_OFF: {
        model_set_relay_y(model_out, false);
        model_set_relay_w(model_out, false);
    } break;
    case SYSTEM_MODE_COOL: {
        model_set_relay_y(model_out, (model_get_cool_setpoint(model_in) <
                                      model_get_ambient(model_in)));
        model_set_relay_w(model_out, false);
    } break;
    case SYSTEM_MODE_HEAT: {
        model_set_relay_y(model_out, false);
        model_set_relay_w(model_out, (model_get_heat_setpoint(model_in) >
                                      model_get_ambient(model_in)));
    } break;

    } // switch

    return model_out;
}

// *****************************************************************************
// Private (static) code

// *****************************************************************************
// End of file

#define TSTAT_LOGIC_TESTS
#ifdef TSTAT_LOGIC_TESTS

#include <stdio.h>

#define BUF_LEN 200

int main(void) {
    char buf[BUF_LEN];
    model_t model;

    // pre-load some model state
    model_load_json(&model,
                    "{\"ambient\":2000, \"cool_setpoint\":1980, "
                    "\"heat_setpoint\":2020, \"relay_y\":true, "
                    "\"relay_w\":true, \"system_mode\":\"SYSTEM_MODE_OFF\"}");
    printf("\nInitial state:");
    model_dump_json(&model, buf, sizeof(buf));
    printf("\n%s", buf);

    printf("\nUpdate after SYSTEM_MODE_OFF:");
    tstat_logic_update(&model); // y should de-energize, w should de-energize
    model_dump_json(&model, buf, sizeof(buf));
    printf("\n%s", buf);

    model_set_system_mode(&model, SYSTEM_MODE_COOL);
    printf("\nUpdate after SYSTEM_MODE_COOL:");
    tstat_logic_update(&model); // y should energize, w should de-energize
    model_dump_json(&model, buf, sizeof(buf));
    printf("\n%s", buf);

    model_set_system_mode(&model, SYSTEM_MODE_HEAT);
    printf("\nUpdate after SYSTEM_MODE_HEAT:");
    tstat_logic_update(&model); // y should de-energize, w should energize
    model_dump_json(&model, buf, sizeof(buf));
    printf("\n%s", buf);

    printf("\n");
}
#endif

/*
clear ; gcc -Wall -o tstat_logic tstat_logic.c model.c jsmn.c jems.c ; ./tstat_logic ; rm -f ./tstat_logic

Initial state:
{"ambient":2000,"cool_setpoint":1980,"heat_setpoint":2020,"relay_y":true,"relay_w":true,"system_mode":"SYSTEM_MODE_OFF"}
Update after SYSTEM_MODE_OFF:
{"ambient":2000,"cool_setpoint":1980,"heat_setpoint":2020,"relay_y":false,"relay_w":false,"system_mode":"SYSTEM_MODE_OFF"}
Update after SYSTEM_MODE_COOL:
{"ambient":2000,"cool_setpoint":1980,"heat_setpoint":2020,"relay_y":true,"relay_w":false,"system_mode":"SYSTEM_MODE_COOL"}
Update after SYSTEM_MODE_HEAT:
{"ambient":2000,"cool_setpoint":1980,"heat_setpoint":2020,"relay_y":false,"relay_w":true,"system_mode":"SYSTEM_MDOE_HEAT"}
 */
