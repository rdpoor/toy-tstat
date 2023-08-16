/**
 * @file model_iface.h
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
 * @brief Interface between remote and local model.
 */

#ifndef _MODEL_IFACE_H_
#define _MODEL_IFACE_H_

// *****************************************************************************
// Includes

#include "mulib/core/mu_task.h"
#include <stdbool.h>

// *****************************************************************************
// C++ compatibility

#ifdef __cplusplus
extern "C" {
#endif

// *****************************************************************************
// Public types and definitions

// *****************************************************************************
// Public declarations

/**
 * @brief Called once at startup prior to starting scheduler.
 */
void model_iface_init(void);

/**
 * @brief Pull remote model state into local model.
 */
bool model_iface_pull(model_t *model, mu_task_t *on_completion);

/**
 * @brief Push local model to remote model.
 */
bool model_iface_push(model_t *model);

/**
 * @brief Return true if the most recent call to model_iface_pull() had error.
 */
bool model_iface_had_error(void);

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

// *****************************************************************************
// End of file

#ifdef __cplusplus
}
#endif

#endif /* #ifndef _MODEL_IFACE_H_ */
