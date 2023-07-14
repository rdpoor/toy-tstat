/**
 * @file coms_mgr.h
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
 * @brief Send and receive packets of data.
 */

#ifndef _COMS_MGR_H_
#define _COMS_MGR_H_

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
void coms_mgr_init(void);

/**
 * @brief Send a message.  Returns immediately.
 */
bool coms_mgr_send(const char *msg, size_t msg_len);

/**
 * @brief Wait for a msg null terminated msg to arrive, notify on_completion.
 *
 * Note: msg will be stored in buf, null terminated.
 */
bool coms_mgr_recv(char *buf, size_t capacity, mu_task_t *on_completion);

/**
 * @brief Return true if the most recent call to coms_mgr_recv() had error.
 */
bool coms_mgr_had_error(void);

// *****************************************************************************
// End of file

#ifdef __cplusplus
}
#endif

#endif /* #ifndef _COMS_MGR_H_ */
