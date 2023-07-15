/**
 * MIT License
 *
 * Copyright (c) 2020-2023 R. D. Poor <rdpoor@gmail.com>
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
 * @brief A basic logging system with control of different logging levels.
 */

#ifndef _MU_LOG_H_
#define _MU_LOG_H_

// *****************************************************************************
// Includes

#include <stdarg.h>
#include <stdbool.h>

// *****************************************************************************
// C++ compatibility

#ifdef __cplusplus
extern "C" {
#endif

// *****************************************************************************
// Public types and definitions

#define MU_LOG_LEVELS(M)                                                       \
    M(MU_LOG_LEVEL_TRACE, "TRACE")                                             \
    M(MU_LOG_LEVEL_DEBUG, "DEBUG")                                             \
    M(MU_LOG_LEVEL_INFO, "INFO")                                               \
    M(MU_LOG_LEVEL_WARN, "WARN")                                               \
    M(MU_LOG_LEVEL_ERROR, "ERROR")                                             \
    M(MU_LOG_LEVEL_FATAL, "FATAL")

#define EXPAND_LOG_LEVEL_ENUM(_enum_id, _name) _enum_id,
typedef enum { MU_LOG_LEVELS(EXPAND_LOG_LEVEL_ENUM) } mu_log_level_t;

#define MU_LOG_TRACE(...) mu_log(MU_LOG_LEVEL_TRACE, __VA_ARGS__)
#define MU_LOG_DEBUG(...) mu_log(MU_LOG_LEVEL_DEBUG, __VA_ARGS__)
#define MU_LOG_INFO(...) mu_log(MU_LOG_LEVEL_INFO, __VA_ARGS__)
#define MU_LOG_WARN(...) mu_log(MU_LOG_LEVEL_WARN, __VA_ARGS__)
#define MU_LOG_ERROR(...) mu_log(MU_LOG_LEVEL_ERROR, __VA_ARGS__)
#define MU_LOG_FATAL(...) mu_log(MU_LOG_LEVEL_FATAL, __VA_ARGS__)

/**
 * @brief Signature for the user-supplied logging function.
 *
 * @return the number of chars printed (excluding the terminating null byte).
 */
typedef int (*mu_log_logging_fn)(const char *format, va_list ap);

// *****************************************************************************
// Public declarations

/**
 * @brief Initialize the logging system with initial reporting level and logging
 * function.
 */
void mu_log_init(mu_log_level_t reporting_level, mu_log_logging_fn logging_fn);

/**
 * @brief Set or update the current reporting level.
 */
void mu_log_set_reporting_level(mu_log_level_t reporting_level);

/**
 * @brief Return the current reporting level.
 */
mu_log_level_t mu_log_get_reporting_level(void);

/**
 * @brief Set or update the user-supplied logging function
 *
 * You can disable logging by setting logging_fn to NULL.
 */
void mu_log_set_logging_function(mu_log_logging_fn logging_fn);

/**
 * @brief Get the user-supplied logging function
 */
mu_log_logging_fn mu_log_get_logging_function(void);

/**
 * @brief Return the current logging level as a string.
 */
const char *mu_log_level_name(mu_log_level_t level);

/**
 * @brief Return true if MU_LOG_xxx() would call mu_log(...).
 */
bool mu_log_is_reporting(mu_log_level_t reporting_level);

/**
 * @brief If the log level is enabled, call the user-supplied logging function.
 */
void mu_log(mu_log_level_t level, const char *fmt, ...);

#ifdef __cplusplus
}
#endif

#endif /* #ifndef _MU_LOG_H_ */
