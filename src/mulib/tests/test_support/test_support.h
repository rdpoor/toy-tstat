/**
 * MIT License
 *
 * Copyright (c) 2021-2023 R. Dunbar Poor <rdpoor@gmail.com>
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
 * @file: test_support.h
 *
 * @brief Support for mulib unit tests
 */

#ifndef _TEST_SUPPORT_H_
#define _TEST_SUPPORT_H_

// *****************************************************************************
// Includes

#include "mu_task.h"
#include <stdbool.h>

// *****************************************************************************
// C++ Compatibility

#ifdef __cplusplus
extern "C" {
#endif

// *****************************************************************************
// Public types and definitions

#define MU_ASSERT(e) _mu_assert(e, #e, __FILE__, __LINE__)

/**
 * @brief Define a object that counts how many times it has been called.
 */
typedef struct {
    mu_task_t task;
    int call_count;
} counting_obj_t;

// *****************************************************************************
// Public declarations

void _mu_assert(bool expr, const char *str, const char *file, int line);

counting_obj_t *counting_obj_init(counting_obj_t *counting_obj);
mu_task_t *counting_obj_task(counting_obj_t *counting_obj);
counting_obj_t *counting_obj_reset(counting_obj_t *counting_obj);
int counting_obj_get_call_count(counting_obj_t *counting_obj);
int counting_obj_increment_call_count(counting_obj_t *counting_obj);

// *****************************************************************************
// End of file

#ifdef __cplusplus
}
#endif

#endif /* #ifndef _TEST_SUPPORT_H_ */
