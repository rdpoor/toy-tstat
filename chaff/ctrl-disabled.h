/**
 * @file ctrl.h
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
  * @brief Abstract controller API for a simple thermostat.
  */

#ifndef _CTRL_H_
#define _CTRL_H_

// *****************************************************************************
// Includes

#include "system_mode.h"
#include <stdint.h>

// *****************************************************************************
// C++ compatibility

#ifdef __cplusplus
extern "C" {
#endif

// *****************************************************************************
// Public types and definitions

typedef struct {
    void (*set_setpoint)(int setpoint);
    void (*incr_setpoint)(bool up);
    void (*set_system_mode)(system_mode_t system_mode);
    void (*on_tick)(void);
} ctrl_api_t;

typedef struct {
    const ctrl_api_t *api;
    uintptr_t *ctx;
} ctrl_t;

// *****************************************************************************
// Public declarations

void ctrl_set_setpoint(ctrl_t *ctrl, int setpoint);
void ctrl_incr_setpoint(ctrl_t *ctrl, bool up);
void ctrl_set_system_mode(ctrl_t *ctrl, system_mode_t system_mode);
void ctrl_on_tick(ctrl_t *ctrl);

// *****************************************************************************
// End of file

#ifdef __cplusplus
}
#endif

#endif /* #ifndef _CTRL_H_ */
