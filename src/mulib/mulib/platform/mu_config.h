/**
 * MIT License
 *
 * Copyright (c) 2021-2022 R. D. Poor <rdpoor@gmail.com>
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
 * @file mu_config.h
 *
 * @brief Platform specific definitions and declarations required by mulib.
 *
 * If you are porting mulib to a specific platform, create the required
 * definitions and declarations in mu_config.h and mu_time.h and implementation
 * in mu_time.c.  If you are using mulib in your project, you must find (or
 & create) the appropriate mu_config and mu_time files for your config.
 */

#ifndef _MU_CONFIG_H_
#define _MU_CONFIG_H_

// *****************************************************************************
// Includes

// *****************************************************************************
// C++ Compatibility

#ifdef __cplusplus
extern "C" {
#endif

// *****************************************************************************
// Public types and definitions

// Optional: un-comment this if your config supports floating point operations
// #define MU_CONFIG_HAS_FLOAT

#ifdef MU_CONFIG_HAS_FLOAT
// If your config supports floats, choose one or the other
// #define MU_FLOAT double
#define MU_FLOAT float
#endif

// Optional: Define the number of deferred events that may be scheduled.
// Leave commented to accept the default.
// #define MU_CONFIG_SCHED_MAX_DEFERRED_TASKS 20

// Optional: Define the number of interrupt events that may be scheduled.
// Leave commented to accept the default.
// #define MU_CONFIG_SCHED_MAX_IRQ_TASKS 8 // must be a power of two!

// Optional: Define the number of immediate events that may be scheduled.
// Leave commented to accept the default.
// #define MU_CONFIG_SCHED_MAX_ASAP_TASKS 20

// *****************************************************************************
// Public declarations

// *****************************************************************************
// End of file

#ifdef __cplusplus
}
#endif

#endif /* #ifndef _MU_CONFIG_H_ */
