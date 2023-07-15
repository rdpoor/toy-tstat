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

// *****************************************************************************
// Includes

#include "mu_log.h"

#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>

// *****************************************************************************
// Local (private) types and definitions

// *****************************************************************************
// Local (private, static) storage

// define s_level_names[], an array that maps a logging level to a string
#define EXPAND_LEVEL_NAMES(_enum_id, _name) _name,
static const char *s_level_names[] = {MU_LOG_LEVELS(EXPAND_LEVEL_NAMES)};
#define N_LOG_LEVELS (sizeof(s_level_names)/sizeof(s_level_names[0]))

// the current reporting level.  May be changed dynamically.
static mu_log_level_t s_reporting_level;

static mu_log_logging_fn s_logging_fn;

// *****************************************************************************
// Local (private, static) forward declarations

// *****************************************************************************
// Public code

void mu_log_init(mu_log_level_t reporting_level, mu_log_logging_fn logging_fn) {
    mu_log_set_reporting_level(reporting_level);
    mu_log_set_logging_function(logging_fn);
}

void mu_log_set_reporting_level(mu_log_level_t reporting_level) {
    s_reporting_level = reporting_level;
}

mu_log_level_t mu_log_get_reporting_level(void) {
    return s_reporting_level;
}

void mu_log_set_logging_function(mu_log_logging_fn logging_fn) {
    s_logging_fn = logging_fn;
}

mu_log_logging_fn mu_log_get_logging_function(void) {
    return s_logging_fn;
}

bool mu_log_is_reporting(mu_log_level_t reporting_level) {
    return reporting_level >= s_reporting_level;
}

const char *mu_log_level_name(mu_log_level_t level) {
    if (level < N_LOG_LEVELS) {
        return s_level_names[level];
    } else {
        return "UNKNOWN";
    }
}

void mu_log(mu_log_level_t level, const char *fmt, ...) {
    if (mu_log_is_reporting(level) && (s_logging_fn != NULL)) {
        va_list ap;
        va_start(ap, fmt);
        s_logging_fn(fmt, ap);
        va_end(ap);
    }
}

// *****************************************************************************
// Local (private, static) code

// *****************************************************************************
// *****************************************************************************
// Standalone tests
// *****************************************************************************
// *****************************************************************************

// Run this command in to run the standalone tests.
// gcc -Wall -DTEST_MU_LOG -o test_mu_log mu_log.c && ./test_mu_log && rm ./test_mu_log

#ifdef TEST_MU_LOG

#include <stdio.h>
#include <string.h>

#define ASSERT(e) assert(e, #e, __FILE__, __LINE__)
static void assert(bool expr, const char *str, const char *file, int line) {
    if (!expr) {
        printf("\nassertion %s failed at %s:%d", str, file, line);
    }
}

static char tbuf[200];

static void clear_tbuf(void) {
    memset(tbuf, 0, sizeof(tbuf));
}

static bool test_tbuf(const char *expected) {
    return strcmp(tbuf, expected) == 0;
}

static int tprint(const char *format, va_list ap) {
    int n = sprintf(tbuf, "prefix: ");
    n += vsprintf(&tbuf[n], format, ap);
    return n;
}

static void test_mu_log(void) {
    printf("\nStarting test_mu_log...");

    mu_log_init(MU_LOG_LEVEL_FATAL, tprint);
    clear_tbuf();
    ASSERT(test_tbuf(""));

    // only report with MU_LOG_FATAL or more severe...
    mu_log_set_reporting_level(MU_LOG_LEVEL_FATAL);
    clear_tbuf();
    MU_LOG_TRACE("t01");
    ASSERT(test_tbuf(""));
    ASSERT(mu_log_is_reporting(MU_LOG_LEVEL_TRACE) == false);
    MU_LOG_DEBUG("t02");
    ASSERT(test_tbuf(""));
    ASSERT(mu_log_is_reporting(MU_LOG_LEVEL_DEBUG) == false);
    MU_LOG_INFO("t03");
    ASSERT(test_tbuf(""));
    ASSERT(mu_log_is_reporting(MU_LOG_LEVEL_INFO) == false);
    MU_LOG_WARN("t04");
    ASSERT(test_tbuf(""));
    ASSERT(mu_log_is_reporting(MU_LOG_LEVEL_WARN) == false);
    MU_LOG_ERROR("t05");
    ASSERT(test_tbuf(""));
    ASSERT(mu_log_is_reporting(MU_LOG_LEVEL_ERROR) == false);
    MU_LOG_FATAL("t06");
    ASSERT(test_tbuf("prefix: t06"));
    ASSERT(mu_log_is_reporting(MU_LOG_LEVEL_FATAL) == true);

    // only report with MU_LOG_ERROR or more severe...
    mu_log_set_reporting_level(MU_LOG_LEVEL_ERROR);
    clear_tbuf();
    MU_LOG_TRACE("t07");
    ASSERT(test_tbuf(""));
    ASSERT(mu_log_is_reporting(MU_LOG_LEVEL_TRACE) == false);
    MU_LOG_DEBUG("t08");
    ASSERT(test_tbuf(""));
    ASSERT(mu_log_is_reporting(MU_LOG_LEVEL_DEBUG) == false);
    MU_LOG_INFO("t09");
    ASSERT(test_tbuf(""));
    ASSERT(mu_log_is_reporting(MU_LOG_LEVEL_INFO) == false);
    MU_LOG_WARN("t10");
    ASSERT(test_tbuf(""));
    ASSERT(mu_log_is_reporting(MU_LOG_LEVEL_WARN) == false);
    MU_LOG_ERROR("t11");
    ASSERT(test_tbuf("prefix: t11"));
    ASSERT(mu_log_is_reporting(MU_LOG_LEVEL_ERROR) == true);
    MU_LOG_FATAL("t12");
    ASSERT(test_tbuf("prefix: t12"));
    ASSERT(mu_log_is_reporting(MU_LOG_LEVEL_FATAL) == true);

    // only report with MU_LOG_WARN or more severe...
    mu_log_set_reporting_level(MU_LOG_LEVEL_WARN);
    clear_tbuf();
    MU_LOG_TRACE("t13");
    ASSERT(test_tbuf(""));
    ASSERT(mu_log_is_reporting(MU_LOG_LEVEL_TRACE) == false);
    MU_LOG_DEBUG("t14");
    ASSERT(test_tbuf(""));
    ASSERT(mu_log_is_reporting(MU_LOG_LEVEL_DEBUG) == false);
    MU_LOG_INFO("t15");
    ASSERT(test_tbuf(""));
    ASSERT(mu_log_is_reporting(MU_LOG_LEVEL_INFO) == false);
    MU_LOG_WARN("t16");
    ASSERT(test_tbuf("prefix: t16"));
    ASSERT(mu_log_is_reporting(MU_LOG_LEVEL_WARN) == true);
    MU_LOG_ERROR("t17");
    ASSERT(test_tbuf("prefix: t17"));
    ASSERT(mu_log_is_reporting(MU_LOG_LEVEL_ERROR) == true);
    MU_LOG_FATAL("t18");
    ASSERT(test_tbuf("prefix: t18"));
    ASSERT(mu_log_is_reporting(MU_LOG_LEVEL_FATAL) == true);

    // only report with MU_LOG_INFO or more severe...
    mu_log_set_reporting_level(MU_LOG_LEVEL_INFO);
    clear_tbuf();
    MU_LOG_TRACE("t19");
    ASSERT(test_tbuf(""));
    ASSERT(mu_log_is_reporting(MU_LOG_LEVEL_TRACE) == false);
    MU_LOG_DEBUG("t20");
    ASSERT(test_tbuf(""));
    ASSERT(mu_log_is_reporting(MU_LOG_LEVEL_DEBUG) == false);
    MU_LOG_INFO("t21");
    ASSERT(test_tbuf("prefix: t21"));
    ASSERT(mu_log_is_reporting(MU_LOG_LEVEL_INFO) == true);
    MU_LOG_WARN("t22");
    ASSERT(test_tbuf("prefix: t22"));
    ASSERT(mu_log_is_reporting(MU_LOG_LEVEL_WARN) == true);
    MU_LOG_ERROR("t23");
    ASSERT(test_tbuf("prefix: t23"));
    ASSERT(mu_log_is_reporting(MU_LOG_LEVEL_ERROR) == true);
    MU_LOG_FATAL("t24");
    ASSERT(test_tbuf("prefix: t24"));
    ASSERT(mu_log_is_reporting(MU_LOG_LEVEL_FATAL) == true);

    // only report with MU_LOG_DEBUG or more severe...
    mu_log_set_reporting_level(MU_LOG_LEVEL_DEBUG);
    clear_tbuf();
    MU_LOG_TRACE("t25");
    ASSERT(test_tbuf(""));
    ASSERT(mu_log_is_reporting(MU_LOG_LEVEL_TRACE) == false);
    MU_LOG_DEBUG("t26");
    ASSERT(test_tbuf("prefix: t26"));
    ASSERT(mu_log_is_reporting(MU_LOG_LEVEL_DEBUG) == true);
    MU_LOG_INFO("t27");
    ASSERT(test_tbuf("prefix: t27"));
    ASSERT(mu_log_is_reporting(MU_LOG_LEVEL_INFO) == true);
    MU_LOG_WARN("t28");
    ASSERT(test_tbuf("prefix: t28"));
    ASSERT(mu_log_is_reporting(MU_LOG_LEVEL_WARN) == true);
    MU_LOG_ERROR("t29");
    ASSERT(test_tbuf("prefix: t29"));
    ASSERT(mu_log_is_reporting(MU_LOG_LEVEL_ERROR) == true);
    MU_LOG_FATAL("t30");
    ASSERT(test_tbuf("prefix: t30"));
    ASSERT(mu_log_is_reporting(MU_LOG_LEVEL_FATAL) == true);

    // only report with MU_LOG_TRACE or more severe...
    mu_log_set_reporting_level(MU_LOG_LEVEL_TRACE);
    clear_tbuf();
    MU_LOG_TRACE("t31");
    ASSERT(test_tbuf("prefix: t31"));
    ASSERT(mu_log_is_reporting(MU_LOG_LEVEL_TRACE) == true);
    MU_LOG_DEBUG("t32");
    ASSERT(test_tbuf("prefix: t32"));
    ASSERT(mu_log_is_reporting(MU_LOG_LEVEL_DEBUG) == true);
    MU_LOG_INFO("t33");
    ASSERT(test_tbuf("prefix: t33"));
    ASSERT(mu_log_is_reporting(MU_LOG_LEVEL_INFO) == true);
    MU_LOG_WARN("t34");
    ASSERT(test_tbuf("prefix: t34"));
    ASSERT(mu_log_is_reporting(MU_LOG_LEVEL_WARN) == true);
    MU_LOG_ERROR("t35");
    ASSERT(test_tbuf("prefix: t35"));
    ASSERT(mu_log_is_reporting(MU_LOG_LEVEL_ERROR) == true);
    MU_LOG_FATAL("t36");
    ASSERT(test_tbuf("prefix: t36"));
    ASSERT(mu_log_is_reporting(MU_LOG_LEVEL_FATAL) == true);

    // setting logging_function to NULL inhibits printing
    mu_log_init(MU_LOG_LEVEL_INFO, tprint);
    clear_tbuf();
    MU_LOG_INFO("woof");
    ASSERT(test_tbuf("prefix: woof"));  // prints
    mu_log_set_logging_function(NULL);
    clear_tbuf();
    MU_LOG_INFO("woof");
    ASSERT(test_tbuf(""));              // doesn't print

    // mu_log_init(level, *)
    mu_log_init(MU_LOG_LEVEL_TRACE, NULL);
    ASSERT(mu_log_get_reporting_level() == MU_LOG_LEVEL_TRACE);
    mu_log_init(MU_LOG_LEVEL_DEBUG, NULL);
    ASSERT(mu_log_get_reporting_level() == MU_LOG_LEVEL_DEBUG);
    mu_log_init(MU_LOG_LEVEL_INFO, NULL);
    ASSERT(mu_log_get_reporting_level() == MU_LOG_LEVEL_INFO);
    mu_log_init(MU_LOG_LEVEL_WARN, NULL);
    ASSERT(mu_log_get_reporting_level() == MU_LOG_LEVEL_WARN);
    mu_log_init(MU_LOG_LEVEL_ERROR, NULL);
    ASSERT(mu_log_get_reporting_level() == MU_LOG_LEVEL_ERROR);
    mu_log_init(MU_LOG_LEVEL_FATAL, NULL);
    ASSERT(mu_log_get_reporting_level() == MU_LOG_LEVEL_FATAL);

    // mu_log_init(*, fn)
    mu_log_init(MU_LOG_LEVEL_INFO, NULL);
    ASSERT(mu_log_get_logging_function() == NULL);
    mu_log_init(MU_LOG_LEVEL_INFO, tprint);
    ASSERT(mu_log_get_logging_function() == tprint);

    // mu_log_set_reporting_level
    mu_log_set_reporting_level(MU_LOG_LEVEL_TRACE);
    ASSERT(mu_log_get_reporting_level() == MU_LOG_LEVEL_TRACE);
    mu_log_set_reporting_level(MU_LOG_LEVEL_DEBUG);
    ASSERT(mu_log_get_reporting_level() == MU_LOG_LEVEL_DEBUG);
    mu_log_set_reporting_level(MU_LOG_LEVEL_INFO);
    ASSERT(mu_log_get_reporting_level() == MU_LOG_LEVEL_INFO);
    mu_log_set_reporting_level(MU_LOG_LEVEL_WARN);
    ASSERT(mu_log_get_reporting_level() == MU_LOG_LEVEL_WARN);
    mu_log_set_reporting_level(MU_LOG_LEVEL_ERROR);
    ASSERT(mu_log_get_reporting_level() == MU_LOG_LEVEL_ERROR);
    mu_log_set_reporting_level(MU_LOG_LEVEL_FATAL);
    ASSERT(mu_log_get_reporting_level() == MU_LOG_LEVEL_FATAL);

    // mu_log_set_logging_function
    mu_log_set_logging_function(NULL);
    ASSERT(mu_log_get_logging_function() == NULL);
    mu_log_set_logging_function(tprint);
    ASSERT(mu_log_get_logging_function() == tprint);

    // mu_log_level_name()
    ASSERT(strcmp(mu_log_level_name(MU_LOG_LEVEL_TRACE), "TRACE") == 0);
    ASSERT(strcmp(mu_log_level_name(MU_LOG_LEVEL_DEBUG), "DEBUG") == 0);
    ASSERT(strcmp(mu_log_level_name(MU_LOG_LEVEL_INFO), "INFO") == 0);
    ASSERT(strcmp(mu_log_level_name(MU_LOG_LEVEL_WARN), "WARN") == 0);
    ASSERT(strcmp(mu_log_level_name(MU_LOG_LEVEL_ERROR), "ERROR") == 0);
    ASSERT(strcmp(mu_log_level_name(MU_LOG_LEVEL_FATAL), "FATAL") == 0);

    printf("\n...test_mu_log complete\n");
}

int main(void) { test_mu_log(); }

#endif // #ifdef TEST_MU_MQUEUE
