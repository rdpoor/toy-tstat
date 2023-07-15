/**
 * MIT License
 *
 * Copyright (c) 2021-2023 R. D. Poor <rdpoor@gmail.com>
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

#include "mu_str.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
// #include <string.h>

// *****************************************************************************
// Private types and definitions

// *****************************************************************************
// Private (static) storage

// *****************************************************************************
// Private (forward) declarations

// Avoid requiring string.h
static unsigned long strlen(const char *str) {
    unsigned long len = 0;
    while (*str++ != '\0') {
        len += 1;
    }
    return len;
}

static int str_compare_aux(const uint8_t *b1, size_t len1, const uint8_t *b2,
                           size_t len2);

static bool has_prefix(const uint8_t *s1, size_t s1_len, const uint8_t *s2,
                       size_t s2_len);

static bool has_suffix(const uint8_t *s1, size_t s1_len, const uint8_t *s2,
                       size_t s2_len);

static size_t mu_str_find_aux(const uint8_t *haystack, size_t haystack_len,
                              const uint8_t *needle, size_t needle_len,
                              bool skip_substr);

static size_t mu_str_rfind_aux(const uint8_t *haystack, size_t haystack_len,
                               const uint8_t *needle, size_t needle_len,
                               bool skip_substr);

// *****************************************************************************
// Public code

mu_str_t *mu_str_init(mu_str_t *str, const uint8_t *bytes, size_t len) {
    str->bytes = bytes;
    str->len = len;
    return str;
}

mu_str_t *mu_str_init_cstr(mu_str_t *str, const char *cstr) {
    return mu_str_init(str, (const uint8_t *)cstr, strlen(cstr));
}

const uint8_t *mu_str_bytes(mu_str_t *str) { return str->bytes; }

size_t mu_str_length(mu_str_t *str) { return str->len; }

bool mu_str_is_empty(mu_str_t *str) { return mu_str_length(str) == 0; }

mu_str_t *mu_str_copy(mu_str_t *dst, mu_str_t *src) {
    return mu_str_init(dst, src->bytes, src->len);
}

int mu_str_compare(mu_str_t *s1, mu_str_t *s2) {
    const uint8_t *b1 = mu_str_bytes(s1);
    size_t len1 = mu_str_length(s1);
    const uint8_t *b2 = mu_str_bytes(s2);
    size_t len2 = mu_str_length(s2);

    return str_compare_aux(b1, len1, b2, len2);
}

int mu_str_compare_cstr(mu_str_t *s1, const char *cstr) {
    const uint8_t *b1 = mu_str_bytes(s1);
    size_t len1 = mu_str_length(s1);
    const uint8_t *b2 = (const uint8_t *)cstr;
    size_t len2 = strlen(cstr);

    return str_compare_aux(b1, len1, b2, len2);
}

mu_str_t *mu_str_slice(mu_str_t *dst, mu_str_t *src, ptrdiff_t start,
                       ptrdiff_t end) {
    if (start < 0) {
        // Negative index indexes from end of string
        start = src->len + start;
    } else if ((start > src->len) || (start == MU_STR_END)) {
        // Limit start to end of src string
        start = src->len;
    }

    if (end < 0) {
        // Negative index indexes from end of string
        end = src->len + end;
    } else if ((end >= src->len) || (end == MU_STR_END)) {
        // Limit end to end of src string
        end = src->len;
    }

    if (end < start) {
        // Enforce start <= end
        end = start;
    }

    return mu_str_init(dst, &src->bytes[start], end - start);
}

bool mu_str_has_prefix(mu_str_t *s1, mu_str_t *s2) {
    return has_prefix(mu_str_bytes(s1), mu_str_length(s1), mu_str_bytes(s2),
                      mu_str_length(s2));
}

bool mu_str_has_prefix_cstr(mu_str_t *s1, const char *cstr) {
    return has_prefix(mu_str_bytes(s1), mu_str_length(s1),
                      (const uint8_t *)cstr, strlen(cstr));
}

bool mu_str_has_suffix(mu_str_t *s1, mu_str_t *s2) {
    return has_suffix(mu_str_bytes(s1), mu_str_length(s1), mu_str_bytes(s2),
                      mu_str_length(s2));
}

bool mu_str_has_suffix_cstr(mu_str_t *s1, const char *cstr) {
    return has_suffix(mu_str_bytes(s1), mu_str_length(s1),
                      (const uint8_t *)cstr, strlen(cstr));
}

size_t mu_str_find(mu_str_t *haystack, mu_str_t *needle, bool skip_substr) {
    return mu_str_find_aux(mu_str_bytes(haystack), mu_str_length(haystack),
                           mu_str_bytes(needle), mu_str_length(needle),
                           skip_substr);
}

size_t mu_str_find_cstr(mu_str_t *haystack, const char *needle,
                        bool skip_substr) {
    return mu_str_find_aux(mu_str_bytes(haystack), mu_str_length(haystack),
                           (const uint8_t *)needle, strlen(needle),
                           skip_substr);
}

size_t mu_str_rfind(mu_str_t *haystack, mu_str_t *needle, bool skip_substr) {
    return mu_str_rfind_aux(mu_str_bytes(haystack), mu_str_length(haystack),
                            mu_str_bytes(needle), mu_str_length(needle),
                            skip_substr);
}

size_t mu_str_rfind_cstr(mu_str_t *haystack, const char *needle,
                         bool skip_substr) {
    return mu_str_rfind_aux(mu_str_bytes(haystack), mu_str_length(haystack),
                            (const uint8_t *)needle, strlen(needle),
                            skip_substr);
}

size_t mu_str_match(mu_str_t *str, mu_str_predicate_t predicate, void *arg,
                    bool break_if) {
    size_t str_len = mu_str_length(str);
    const uint8_t *bytes = mu_str_bytes(str);

    for (size_t idx = 0; idx < str_len; idx++) {
        if (predicate(*bytes++, arg) == break_if) {
            return idx;
        }
    }
    return MU_STR_NOT_FOUND;
}

size_t mu_str_rmatch(mu_str_t *str, mu_str_predicate_t predicate, void *arg,
                     bool break_if) {
    size_t str_len = mu_str_length(str);
    const uint8_t *bytes = &mu_str_bytes(str)[str_len]; // for predecrement

    for (size_t idx = str_len; idx > 0; idx--) {
        if (predicate(*--bytes, arg) == break_if) {
            return idx - 1;
        }
    }
    return MU_STR_NOT_FOUND;
}

mu_str_t *mu_str_ltrim(mu_str_t *str, mu_str_predicate_t predicate, void *arg) {
    size_t idx = mu_str_match(str, predicate, arg, false);
    if (idx == MU_STR_NOT_FOUND) {
        return str;
    } else {
        return mu_str_slice(str, str, idx, MU_STR_END);
    }
}

mu_str_t *mu_str_rtrim(mu_str_t *str, mu_str_predicate_t predicate, void *arg) {
    size_t idx = mu_str_rmatch(str, predicate, arg, false);
    if (idx == MU_STR_NOT_FOUND) {
        return str;
    } else {
        return mu_str_slice(str, str, 0, idx);
    }
}

mu_str_t *mu_str_trim(mu_str_t *str, mu_str_predicate_t predicate, void *arg) {
    return mu_str_rtrim(mu_str_ltrim(str, predicate, arg), predicate, arg);
}

bool mu_str_to_cstr(mu_str_t *str, char *buf, size_t capacity) {
    size_t str_length = mu_str_length(str);
    const uint8_t *bytes = mu_str_bytes(str);
    if (str_length >= capacity) {
        return false;
    }
    for (int i = 0; i < str_length; i++) {
        buf[i] = (char)bytes[i];
    }
    buf[str_length] = '\0';
    return true;
}

// TODO: handle zero length str (!!!)

#define DEFINE_INT_PARSER(_name, _type)                                        \
    _type _name(mu_str_t *str) {                                               \
        const uint8_t *buf = mu_str_bytes(str);                                \
        size_t len = mu_str_length(str);                                       \
        _type v = 0;                                                           \
        bool is_negative = false;                                              \
        if ((len >= 1) && *buf == '-') {                                       \
            len -= 1;                                                          \
            buf++;                                                             \
            is_negative = true;                                                \
        }                                                                      \
                                                                               \
        while ((len-- > 0) && is_decimal(*buf)) {                              \
            v = (v * 10) + (*buf++ - '0');                                     \
        }                                                                      \
        return is_negative ? -v : v;                                           \
    }

#define DEFINE_UINT_PARSER(_name, _type)                                       \
    _type _name(mu_str_t *str) {                                               \
        const uint8_t *buf = mu_str_bytes(str);                                \
        size_t len = mu_str_length(str);                                       \
        _type v = 0;                                                           \
                                                                               \
        while ((len-- > 0) && is_decimal(*buf)) {                              \
            v = (v * 10) + (*buf++ - '0');                                     \
        }                                                                      \
        return v;                                                              \
    }

DEFINE_INT_PARSER(mu_str_parse_int, int)
DEFINE_UINT_PARSER(mu_str_parse_int, unsigned int)
DEFINE_INT_PARSER(mu_str_parse_int8, int8_t)
DEFINE_UINT_PARSER(mu_str_parse_uint8, uint8_t)
DEFINE_INT_PARSER(mu_str_parse_int16, int16_t)
DEFINE_UINT_PARSER(mu_str_parse_uint16, uint16_t)
DEFINE_INT_PARSER(mu_str_parse_int32, int32_t)
DEFINE_UINT_PARSER(mu_str_parse_uint32, uint32_t)
DEFINE_INT_PARSER(mu_str_parse_int64, int64_t)
DEFINE_UINT_PARSER(mu_str_parse_uint64, uint64_t)

// *****************************************************************************
// Private (static) code

static int str_compare_aux(const uint8_t *b1, size_t len1, const uint8_t *b2,
                           size_t len2) {
    size_t len = (len1 < len2) ? len1 : len2;

    for (int i = 0; i < len; i++) {
        int d = b1[i] - b2[i];
        if (d != 0) {
            return d;
        }
    }
    // The first N bytes of s1 and s2 are equal: return value based on lengths.
    return len1 - len2;
}

static bool has_prefix(const uint8_t *s1, size_t s1_len, const uint8_t *s2,
                       size_t s2_len) {
    size_t idx;

    if (s2_len == 0) {
        return true;
    } else if (s2_len > s1_len) {
        return false;
    }

    for (idx = 0; idx < s1_len && idx < s2_len; idx++) {
        if (*s1++ != *s2++) {
            break;
        }
    }
    return idx == s2_len;
}

static bool has_suffix(const uint8_t *s1, size_t s1_len, const uint8_t *s2,
                       size_t s2_len) {
    size_t idx;

    if (s2_len == 0) {
        return true;
    } else if (s2_len > s1_len) {
        return false;
    }

    // position past last byte for pre-decrement
    s1 = &s1[s1_len];
    s2 = &s2[s2_len];

    for (idx = 0; idx < s1_len && idx < s2_len; idx++) {
        if (*--s1 != *--s2) {
            break;
        }
    }
    return idx == s2_len;
}

static size_t mu_str_find_aux(const uint8_t *haystack, size_t haystack_len,
                              const uint8_t *needle, size_t needle_len,
                              bool skip_substr) {
    if (needle_len == 0) {
        // null needle matches immediately
        return 0;
    }

    int j;

    // First scan through haystack looking for a byte that matches the first
    // byte of needle.  Micro-optimization: We stop searching when we get within
    // needle_len bytes of the end of haystack, since beyond that, the
    // full-length search will always fail.
    for (int i = 0; i <= haystack_len - needle_len; i++) {
        const uint8_t *h2 = &haystack[i];
        if (*h2 == *needle) {
            // first byte matches.  Do the rest of the bytes match?
            for (j = 1; j < needle_len; j++) {
                if (h2[j] != needle[j]) {
                    // mismatch: advance to next char in haystack
                    break;
                }
            }
            if (j == needle_len) {
                // found: &haystack[i] matched all of *needle
                return skip_substr ? i + needle_len : i;
            }
        }
        // advance to next byte in haystack
    }
    // got to end of haystack without a match.
    return MU_STR_NOT_FOUND;
}

static size_t mu_str_rfind_aux(const uint8_t *haystack, size_t haystack_len,
                               const uint8_t *needle, size_t needle_len,
                               bool skip_substr) {
    if (needle_len == 0) {
        // null needle matches immediately
        return haystack_len;
    }

    int j;

    // First scan through haystack looking for a byte that matches the first
    // byte of needle.  Micro-optimization: We start searching at haystack_end -
    // needle_len bytes of the end of haystack, since beyond that, the
    // full-length search will always fail.
    for (int i = haystack_len - needle_len; i >= 0; --i) {
        const uint8_t *h2 = &haystack[i];
        if (*h2 == *needle) {
            // first byte matches.  Do the rest of the bytes match?
            for (j = 1; j < needle_len; j++) {
                if (h2[j] != needle[j]) {
                    // mismatch: advance to next char in haystack
                    break;
                }
            }
            if (j == needle_len) {
                // found: &haystack[i] matched all of *needle
                return skip_substr ? i + needle_len : i;
            }
        }
        // advance to next byte in haystack
    }
    // got to end of haystack without a match.
    return MU_STR_NOT_FOUND;
}

static bool is_decimal(uint8_t byte) {
    if ((byte >= '0') && (byte <= '9')) {
        return true;
    }
    return false;
}

// *****************************************************************************
// *****************************************************************************
// Standalone tests
// *****************************************************************************
// *****************************************************************************

// Run this command in to run the standalone tests.
// gcc -g -Wall -DTEST_MU_STR -o test_mu_str mu_str.c && ./test_mu_str && rm
// ./test_mu_str

#ifdef TEST_MU_STR

#include <stdio.h>

// Avoid dragging in all of string.h
static int strncmp(const char *s1, const char *s2, register size_t n) {
    unsigned char u1, u2;

    while (n-- > 0) {
        u1 = (unsigned char)*s1++;
        u2 = (unsigned char)*s2++;
        if (u1 != u2) {
            return u1 - u2;
        }
        if (u1 == '\0') {
            return 0;
        }
    }
    return 0;
}

#define ASSERT(e) assert(e, #e, __FILE__, __LINE__)
static void assert(bool expr, const char *str, const char *file, int line) {
    if (!expr) {
        printf("\nassertion %s failed at %s:%d", str, file, line);
    }
}

// Return true if str->bytes equals cstr
static bool cstr_eq(mu_str_t *str, const char *cstr) {
    return strncmp((const char *)mu_str_bytes(str), cstr, mu_str_length(str)) ==
           0;
}

static bool is_member(uint8_t byte, const char *bytes) {
    while (*bytes != '\0') {
        if (byte == *bytes++) {
            return true;
        }
    }
    return false;
}

static bool is_numeric(uint8_t byte, void *arg) {
    (void)arg; // unused
    return is_member(byte, "0123456789");
}

static bool is_hexadecimal(uint8_t byte, void *arg) {
    (void)arg; // unused
    return is_member(byte, "0123456789abcdefABCDEF");
}

static bool is_whitespace(uint8_t byte, void *arg) {
    (void)arg; // unused
    return is_member(byte, " \t\r\n\f\v");
}

static bool is_x(uint8_t byte, void *arg) {
    (void)arg; // unused
    return byte == 'x';
}

static bool is_never(uint8_t byte, void *arg) {
    (void)arg; // unused
    return false;
}

static bool is_always(uint8_t byte, void *arg) {
    (void)arg; // unused
    return true;
}

__attribute__((unused)) static void print_str(mu_str_t *str) {
    size_t len = mu_str_length(str);
    printf("\n[%ld]: '%.*s'", len, (int)len, mu_str_bytes(str));
}

void test_mu_str(void) {

    printf("\nStarting mu_str tests...");
    fflush(stdout);

    // mu_str_init
    do {
        mu_str_t s1;
        const uint8_t buf[] = {65, 66, 67, 68, 69, 70, 71, 72, 73, 74};

        ASSERT(&s1 == mu_str_init(&s1, buf, sizeof(buf)));
        ASSERT(mu_str_bytes(&s1) == buf);
        ASSERT(mu_str_length(&s1) == sizeof(buf));
    } while (false);

    // mu_str_init_cstr
    do {
        mu_str_t s1;
        char *cstr = "ABCDEFGHIJ";

        ASSERT(&s1 == mu_str_init_cstr(&s1, cstr));
        ASSERT(mu_str_bytes(&s1) == (const uint8_t *)cstr);
        ASSERT(mu_str_length(&s1) == strlen(cstr));
    } while (false);

    // mu_str_copy
    do {
        mu_str_t s1, s2;
        uint8_t buf[10];
        mu_str_init(&s1, buf, sizeof(buf));

        ASSERT(&s2 == mu_str_copy(&s2, &s1));
        ASSERT(mu_str_bytes(&s2) == mu_str_bytes(&s1));
        ASSERT(mu_str_length(&s2) == mu_str_length(&s1));
    } while (false);

    // mu_str_compare
    do {
        mu_str_t s1, s2;

        mu_str_init_cstr(&s1, "abcd");
        // strings are equal in content and length
        ASSERT(mu_str_compare(&s1, mu_str_init_cstr(&s2, "abcd")) == 0);
        // s1 is lexographically higher
        ASSERT(mu_str_compare(&s1, mu_str_init_cstr(&s2, "abcc")) > 0);
        // s1 is lexographically lower
        ASSERT(mu_str_compare(&s1, mu_str_init_cstr(&s2, "abce")) < 0);
        // s1 is longer
        ASSERT(mu_str_compare(&s1, mu_str_init_cstr(&s2, "abc")) > 0);
        // s1 is shorter
        ASSERT(mu_str_compare(&s1, mu_str_init_cstr(&s2, "abcde")) < 0);
        // both empty
        ASSERT(mu_str_compare(mu_str_init_cstr(&s1, ""),
                              mu_str_init_cstr(&s2, "")) == 0);
        // s2 empty
        ASSERT(mu_str_compare(mu_str_init_cstr(&s1, "abcd"),
                              mu_str_init_cstr(&s2, "")) > 0);
        // s1 empty
        ASSERT(mu_str_compare(mu_str_init_cstr(&s1, ""),
                              mu_str_init_cstr(&s2, "abcd")) < 0);
    } while (false);

    // mu_str_slice
    do {
        mu_str_t s1, s2;

        mu_str_init_cstr(&s1, "ABCDEFGHIJ");
        // whole slice (indefinite end index)
        ASSERT(&s2 == mu_str_slice(&s2, &s1, 0, MU_STR_END));
        ASSERT(cstr_eq(&s2, "ABCDEFGHIJ"));
        // whole slice (definite end index)
        ASSERT(&s2 == mu_str_slice(&s2, &s1, 0, mu_str_length(&s1)));
        ASSERT(cstr_eq(&s2, "ABCDEFGHIJ"));

        // remove first char (positive start index)
        ASSERT(&s2 == mu_str_slice(&s2, &s1, 1, MU_STR_END));
        ASSERT(cstr_eq(&s2, "BCDEFGHIJ"));
        // remove first char (negative start index)
        ASSERT(&s2 == mu_str_slice(&s2, &s1, -9, MU_STR_END));
        ASSERT(cstr_eq(&s2, "BCDEFGHIJ"));

        // remove last char (positive end index)
        ASSERT(&s2 == mu_str_slice(&s2, &s1, 0, 9));
        ASSERT(cstr_eq(&s2, "ABCDEFGHI"));
        // remove last char (negative end index)
        ASSERT(&s2 == mu_str_slice(&s2, &s1, 0, -1));
        ASSERT(cstr_eq(&s2, "ABCDEFGHI"));

        // extract middle chars (positive indeces)
        ASSERT(&s2 == mu_str_slice(&s2, &s1, 3, 7));
        ASSERT(cstr_eq(&s2, "DEFG"));
        // extract middle chars (negative indeces)
        ASSERT(&s2 == mu_str_slice(&s2, &s1, -7, -3));
        ASSERT(cstr_eq(&s2, "DEFG"));

        // start == end
        ASSERT(&s2 == mu_str_slice(&s2, &s1, 5, 5));
        ASSERT(cstr_eq(&s2, ""));
        // start > end
        ASSERT(&s2 == mu_str_slice(&s2, &s1, 6, 5));
        ASSERT(cstr_eq(&s2, ""));

        // start > end of string
        ASSERT(&s2 == mu_str_slice(&s2, &s1, 20, mu_str_length(&s1)));
        ASSERT(cstr_eq(&s2, ""));
        // end < beginnig of string
        ASSERT(&s2 == mu_str_slice(&s2, &s1, 0, -20));
        ASSERT(cstr_eq(&s2, ""));

    } while (false);

    // bool mu_str_has_prefix(mu_str_t *s1, mu_str_t *s2);
    // bool mu_str_has_prefix_cstr(mu_str_t *s1, const char *cstr);
    // bool mu_str_has_suffix(mu_str_t *s1, mu_str_t *s2);
    // bool mu_str_has_suffix_cstr(mu_str_t *s1, const char *cstr);
    do {
        mu_str_t s1, s2;
        mu_str_init_cstr(&s1, "abcd");

        ASSERT(mu_str_has_prefix(&s1, mu_str_init_cstr(&s2, "")) == true);
        ASSERT(mu_str_has_prefix(&s1, mu_str_init_cstr(&s2, "ab")) == true);
        ASSERT(mu_str_has_prefix(&s1, mu_str_init_cstr(&s2, "cd")) == false);
        ASSERT(mu_str_has_prefix(&s1, mu_str_init_cstr(&s2, "abcd")) == true);
        ASSERT(mu_str_has_prefix(&s1, mu_str_init_cstr(&s2, "abcde")) == false);

        ASSERT(mu_str_has_prefix_cstr(&s1, "") == true);
        ASSERT(mu_str_has_prefix_cstr(&s1, "ab") == true);
        ASSERT(mu_str_has_prefix_cstr(&s1, "cd") == false);
        ASSERT(mu_str_has_prefix_cstr(&s1, "abcd") == true);
        ASSERT(mu_str_has_prefix_cstr(&s1, "abcde") == false);

        ASSERT(mu_str_has_suffix(&s1, mu_str_init_cstr(&s2, "")) == true);
        ASSERT(mu_str_has_suffix(&s1, mu_str_init_cstr(&s2, "ab")) == false);
        ASSERT(mu_str_has_suffix(&s1, mu_str_init_cstr(&s2, "cd")) == true);
        ASSERT(mu_str_has_suffix(&s1, mu_str_init_cstr(&s2, "abcd")) == true);
        ASSERT(mu_str_has_suffix(&s1, mu_str_init_cstr(&s2, "abcde")) == false);

        ASSERT(mu_str_has_suffix_cstr(&s1, "") == true);
        ASSERT(mu_str_has_suffix_cstr(&s1, "ab") == false);
        ASSERT(mu_str_has_suffix_cstr(&s1, "cd") == true);
        ASSERT(mu_str_has_suffix_cstr(&s1, "abcd") == true);
        ASSERT(mu_str_has_suffix_cstr(&s1, "abcde") == false);

    } while (false);

    // size_t mu_str_find(mu_str_t *haystack, mu_str_t *needle, bool
    // skip_substr); size_t mu_str_find_cstr(mu_str_t *haystack, const char
    // *needle, bool skip_substr); size_t mu_str_rfind(mu_str_t *haystack,
    // mu_str_t *needle, bool skip_substr); size_t mu_str_rfind_cstr(mu_str_t
    // *haystack, const char *needle, bool skip_substr);
    do {
        mu_str_t s1, s2;

        //                     0123456789
        mu_str_init_cstr(&s1, "abXcdabYcd");
        ASSERT(mu_str_find(&s1, mu_str_init_cstr(&s2, ""), false) == 0);
        ASSERT(mu_str_find(&s1, mu_str_init_cstr(&s2, ""), true) == 0);
        ASSERT(mu_str_find(&s1, mu_str_init_cstr(&s2, "ab"), false) == 0);
        ASSERT(mu_str_find(&s1, mu_str_init_cstr(&s2, "ab"), true) == 2);
        ASSERT(mu_str_find(&s1, mu_str_init_cstr(&s2, "cd"), false) == 3);
        ASSERT(mu_str_find(&s1, mu_str_init_cstr(&s2, "cd"), true) == 5);
        ASSERT(mu_str_find(&s1, mu_str_init_cstr(&s2, "cdX"), false) ==
               MU_STR_NOT_FOUND);
        ASSERT(mu_str_find(&s1, mu_str_init_cstr(&s2, "cdX"), true) ==
               MU_STR_NOT_FOUND);

        ASSERT(mu_str_find_cstr(&s1, "", false) == 0);
        ASSERT(mu_str_find_cstr(&s1, "", true) == 0);
        ASSERT(mu_str_find_cstr(&s1, "ab", false) == 0);
        ASSERT(mu_str_find_cstr(&s1, "ab", true) == 2);
        ASSERT(mu_str_find_cstr(&s1, "cd", false) == 3);
        ASSERT(mu_str_find_cstr(&s1, "cd", true) == 5);
        ASSERT(mu_str_find_cstr(&s1, "cdX", false) == MU_STR_NOT_FOUND);
        ASSERT(mu_str_find_cstr(&s1, "cdX", true) == MU_STR_NOT_FOUND);

        ASSERT(mu_str_rfind(&s1, mu_str_init_cstr(&s2, ""), false) == 10);
        ASSERT(mu_str_rfind(&s1, mu_str_init_cstr(&s2, ""), true) == 10);
        ASSERT(mu_str_rfind(&s1, mu_str_init_cstr(&s2, "ab"), false) == 5);
        ASSERT(mu_str_rfind(&s1, mu_str_init_cstr(&s2, "ab"), true) == 7);
        ASSERT(mu_str_rfind(&s1, mu_str_init_cstr(&s2, "cd"), false) == 8);
        ASSERT(mu_str_rfind(&s1, mu_str_init_cstr(&s2, "cd"), true) == 10);
        ASSERT(mu_str_rfind(&s1, mu_str_init_cstr(&s2, "cdX"), false) ==
               MU_STR_NOT_FOUND);
        ASSERT(mu_str_rfind(&s1, mu_str_init_cstr(&s2, "cdX"), true) ==
               MU_STR_NOT_FOUND);

        ASSERT(mu_str_rfind_cstr(&s1, "", false) == 10);
        ASSERT(mu_str_rfind_cstr(&s1, "", true) == 10);
        ASSERT(mu_str_rfind_cstr(&s1, "ab", false) == 5);
        ASSERT(mu_str_rfind_cstr(&s1, "ab", true) == 7);
        ASSERT(mu_str_rfind_cstr(&s1, "cd", false) == 8);
        ASSERT(mu_str_rfind_cstr(&s1, "cd", true) == 10);
        ASSERT(mu_str_rfind_cstr(&s1, "cdX", false) == MU_STR_NOT_FOUND);
        ASSERT(mu_str_rfind_cstr(&s1, "cdX", true) == MU_STR_NOT_FOUND);
    } while (false);

    // regression test
    do {
        mu_str_t x;
        mu_str_init_cstr(&x, "A\r\n");
        size_t idx = mu_str_find_cstr(&x, "\r\n", false);
        ASSERT(idx == 1);
    } while (false);

    // size_t mu_str_match(mu_str_t *str, mu_str_predicate_t pred, void *arg);
    // size_t mu_str_rmatch(mu_str_t *str, mu_str_predicate_t pred, void *arg);
    do {
        mu_str_t s1;

        mu_str_init_cstr(&s1, "0123");
        // finds the very first char '0'
        ASSERT(mu_str_match(&s1, is_numeric, NULL, true) == 0); // found '0'
        // hits the end of string without a "not match"
        ASSERT(mu_str_match(&s1, is_numeric, NULL, false) == MU_STR_NOT_FOUND);
        // finds the very last char '3'
        ASSERT(mu_str_rmatch(&s1, is_numeric, NULL, true) == 3); // found '3'
        // hits the begining of string without a "not match"
        ASSERT(mu_str_rmatch(&s1, is_numeric, NULL, false) == MU_STR_NOT_FOUND);

        //                     00000000001111111 1 1 1 2 2
        //                     01234567890123456 7 8 9 0 1
        mu_str_init_cstr(&s1, "0123456789abcDEF \r\n\t\v\f");

        ASSERT(mu_str_match(&s1, is_numeric, NULL, true) == 0);
        ASSERT(mu_str_match(&s1, is_numeric, NULL, false) == 10);
        ASSERT(mu_str_match(&s1, is_hexadecimal, NULL, true) == 0);
        ASSERT(mu_str_match(&s1, is_hexadecimal, NULL, false) == 16);
        ASSERT(mu_str_match(&s1, is_whitespace, NULL, true) == 16);
        ASSERT(mu_str_match(&s1, is_whitespace, NULL, false) == 0);
        ASSERT(mu_str_match(&s1, is_x, NULL, true) == MU_STR_NOT_FOUND);
        ASSERT(mu_str_match(&s1, is_x, NULL, false) == 0);

        ASSERT(mu_str_rmatch(&s1, is_numeric, NULL, true) == 9);
        ASSERT(mu_str_rmatch(&s1, is_numeric, NULL, false) == 21);
        ASSERT(mu_str_rmatch(&s1, is_hexadecimal, NULL, true) == 15);
        ASSERT(mu_str_rmatch(&s1, is_hexadecimal, NULL, false) == 21);
        ASSERT(mu_str_rmatch(&s1, is_whitespace, NULL, true) == 21);
        ASSERT(mu_str_rmatch(&s1, is_whitespace, NULL, false) == 15);
        ASSERT(mu_str_rmatch(&s1, is_x, NULL, true) == MU_STR_NOT_FOUND);
        ASSERT(mu_str_rmatch(&s1, is_x, NULL, false) == 21);

        mu_str_init_cstr(&s1, "");
        ASSERT(mu_str_match(&s1, is_never, NULL, true) == MU_STR_NOT_FOUND);
        ASSERT(mu_str_rmatch(&s1, is_never, NULL, false) == MU_STR_NOT_FOUND);
        ASSERT(mu_str_match(&s1, is_always, NULL, true) == MU_STR_NOT_FOUND);
        ASSERT(mu_str_rmatch(&s1, is_always, NULL, false) == MU_STR_NOT_FOUND);

    } while (false);

    // mu_str_ltrim, mu_str_rtrim, mu_str_trim
    do {
        mu_str_t s1;

        mu_str_init_cstr(&s1, "  abcde  ");
        ASSERT(&s1 == mu_str_ltrim(&s1, is_whitespace, NULL));
        ASSERT(cstr_eq(&s1, "abcde  "));

        mu_str_init_cstr(&s1, "  abcde  ");
        ASSERT(&s1 == mu_str_rtrim(&s1, is_whitespace, NULL));
        ASSERT(cstr_eq(&s1, "  abcde"));

        mu_str_init_cstr(&s1, "  abcde  ");
        ASSERT(&s1 == mu_str_trim(&s1, is_whitespace, NULL));
        ASSERT(cstr_eq(&s1, "abcde"));
    } while (false);

    do {
        mu_str_t s1;
        char buf[5]; // 4 chars max (plus null termination)

        mu_str_init_cstr(&s1, "abcd");
        ASSERT(mu_str_to_cstr(&s1, buf, sizeof(buf)) == true);
        ASSERT(buf[0] == 'a'); // strcmp avoidance...
        ASSERT(buf[1] == 'b');
        ASSERT(buf[2] == 'c');
        ASSERT(buf[3] == 'd');

        mu_str_init_cstr(&s1, "abcde");
        ASSERT(mu_str_to_cstr(&s1, buf, sizeof(buf)) == false);
    } while (false);

    printf("\n...tests complete\n");
}

void test_mu_str_example(void) {
    // Parse an HTML message, extracting the Date: from the header and the
    // contents of the body.
    printf("\nStarting mu_str_example...");
    fflush(stdout);

    const char *HTML = "HTTP/1.1 200 OK\r\n"
                       "Date: Wed, 26 Oct 2022 17:17:34 GMT\r\n"
                       "Content-Type: application/json\r\n"
                       "Content-Length: 27\r\n"
                       "Connection: keep-alive\r\n"
                       "X-Javatime: 1666804654506\r\n"
                       "\r\n"
                       "{\"code\":200,\"message\":\"ok\"}";

    mu_str_t html, date_value, body;
    size_t idx;

    mu_str_init_cstr(&html, HTML);

    // find "Wed, 26 Oct 2022 17:17:34 GMT"
    // Extract the text following "Date: " to end of line
    idx = mu_str_find_cstr(&html, "Date: ", true);
    ASSERT(idx != MU_STR_NOT_FOUND);
    mu_str_slice(&date_value, &html, idx, MU_STR_END);
    idx = mu_str_find_cstr(&date_value, "\r\n", false);
    ASSERT(idx != MU_STR_NOT_FOUND);
    mu_str_slice(&date_value, &date_value, 0, idx);
    ASSERT(cstr_eq(&date_value, "Wed, 26 Oct 2022 17:17:34 GMT"));

    // find "{\"code\":200,\"message\":\"ok\"}"
    // blank \r\n\r\n signifies end of HTML header and start of body
    idx = mu_str_find_cstr(&html, "\r\n\r\n", true);
    ASSERT(idx != MU_STR_NOT_FOUND);
    mu_str_slice(&body, &html, idx, MU_STR_END);
    ASSERT(cstr_eq(&body, "{\"code\":200,\"message\":\"ok\"}"));

    printf("\n...mu_str_example complete\n");
}

typedef struct {
    const char *host_name;
    size_t host_name_len;
    uint16_t host_port;
    bool use_tls;
} http_params_t;

#define HTTP_PREFIX "http://"
#define HTTPS_PREFIX "https://"

static bool is_hostname(uint8_t byte, void *arg) {
    if ((byte >= 'a') && (byte <= 'z')) {
        return true;
    } else if ((byte >= '0') && (byte <= '9')) {
        return true;
    } else if (byte == '.') {
        return true;
    } else if (byte == '-') {
        return true;
    } else {
        return false;
    }
}

static bool is_decimal(uint8_t byte, void *arg) {
    if ((byte >= '0') && (byte <= '9')) {
        return true;
    }
    return false;
}

static uint16_t parse_uint16(const uint8_t *buf, size_t len) {
    uint16_t v = 0;

    while (len-- > 0) {
        v = (v * 10) + (*buf++ - '0');
    }
    return v;
}

http_params_t *parse_http_params(http_params_t *params, const char *url) {
    mu_str_t s1;
    size_t idx;

    mu_str_init_cstr(&s1, url);
    // printf("\n==== parsing '%s'", url);

    if (mu_str_has_prefix_cstr(&s1, HTTP_PREFIX)) {
        // starts with http://
        // printf("\nStarts with http://");
        params->use_tls = false;
        params->host_port = 80; // unless over-ridden by a :<port> spec...
        mu_str_slice(&s1, &s1, strlen(HTTP_PREFIX), MU_STR_END);

    } else if (mu_str_has_prefix_cstr(&s1, HTTPS_PREFIX)) {
        // starts with https://
        // printf("\nStarts with https://");
        params->use_tls = true;
        params->host_port = 443; // unless over-ridden by a :<port> spec...
        mu_str_slice(&s1, &s1, strlen(HTTPS_PREFIX), MU_STR_END);

    } else {
        return NULL;
    }

    // hostname may not start with a '-'
    if (mu_str_has_prefix_cstr(&s1, "-")) {
        // printf("\nhostname may not start with -");
        return NULL;
    }

    // search for first non-hostname char
    idx = mu_str_match(&s1, is_hostname, NULL, false);

    if (idx == MU_STR_NOT_FOUND) {
        // hit end of string without finding any non-hostname chars
        idx = mu_str_length(&s1);
    }

    if (idx == 0) {
        // zero length hostname not allowed.
        // printf("\nZero length hostname not allowed");
        return NULL;

    } else if (mu_str_bytes(&s1)[idx] != ':') {
        // hostname not terminated with a port number
        params->host_name = (const char *)mu_str_bytes(&s1);
        params->host_name_len = idx;
        return params;
    }

    // hostname terminated with a ':' -- parse the following port number
    mu_str_slice(&s1, &s1, idx + 1, MU_STR_END);

    idx = mu_str_match(&s1, is_decimal, NULL, false);
    if (idx == MU_STR_NOT_FOUND) {
        // hit end of string without finding any non-decimal chars
        idx = mu_str_length(&s1);
    }

    if (idx == 0) {
        // printf("\nzero length port # not allowed");
        // zero length port # not allowed
        return NULL;
    }

    // here, all bytes between s1[0] and s1[idx] are guaranteed to be integers.
    // NOTE: we don't check for overflow (values >= 65536)
    params->host_port = parse_uint16(mu_str_bytes(&s1), idx);

    return params;
}

void test_parse_url(void) {
    http_params_t params;

    printf("\nStarting test_parse_url example...");
    fflush(stdout);

    ASSERT(parse_http_params(&params, "http://example.com") == &params);
    ASSERT(strncmp(params.host_name, "example.com", params.host_name_len) == 0);
    ASSERT(params.host_port == 80);
    ASSERT(params.use_tls == false);

    ASSERT(parse_http_params(&params, "https://example.com") == &params);
    ASSERT(strncmp(params.host_name, "example.com", params.host_name_len) == 0);
    ASSERT(params.host_port == 443);
    ASSERT(params.use_tls == true);

    ASSERT(parse_http_params(&params, "http://example.com:8080") == &params);
    ASSERT(strncmp(params.host_name, "example.com", params.host_name_len) == 0);
    ASSERT(params.host_port == 8080);
    ASSERT(params.use_tls == false);

    ASSERT(parse_http_params(&params, "https://example.com:8080") == &params);
    ASSERT(strncmp(params.host_name, "example.com", params.host_name_len) == 0);
    ASSERT(params.host_port == 8080);
    ASSERT(params.use_tls == true);

    ASSERT(parse_http_params(&params, "https://example.com/extra") == &params);
    ASSERT(strncmp(params.host_name, "example.com", params.host_name_len) == 0);
    ASSERT(params.host_port == 443);
    ASSERT(params.use_tls == true);

    ASSERT(parse_http_params(&params, "https://example.com:123/extra") ==
           &params);
    ASSERT(strncmp(params.host_name, "example.com", params.host_name_len) == 0);
    ASSERT(params.host_port == 123);
    ASSERT(params.use_tls == true);

    ASSERT(parse_http_params(&params, "https://example.com/") == &params);
    ASSERT(strncmp(params.host_name, "example.com", params.host_name_len) == 0);
    ASSERT(params.host_port == 443);
    ASSERT(params.use_tls == true);

    ASSERT(parse_http_params(&params, "https://192.168.12.34/") == &params);
    ASSERT(strncmp(params.host_name, "192.168.12.34", params.host_name_len) ==
           0);
    ASSERT(params.host_port == 443);
    ASSERT(params.use_tls == true);

    // These are the valid chars that end a port #
    // "#?/\"

    // pathologies
    ASSERT(parse_http_params(&params, "") == NULL);
    ASSERT(parse_http_params(&params, "http") == NULL);
    ASSERT(parse_http_params(&params, "http:") == NULL);
    ASSERT(parse_http_params(&params, "http:/") == NULL);
    ASSERT(parse_http_params(&params, "http://") == NULL);
    ASSERT(parse_http_params(&params, "http://:") == NULL);
    ASSERT(parse_http_params(&params, "http://:123") == NULL);
    ASSERT(parse_http_params(&params, "http://example.com:") == NULL);
    ASSERT(parse_http_params(&params, "http://example.com:abc") == NULL);
    ASSERT(parse_http_params(&params, "ftp://example.com:123") == NULL);
    // parse_http_params() doesn't investigate the URL past the hostname / port#
    // ASSERT(parse_http_params(&params, "http://example.com/extra:123") ==
    // NULL); This should be caught, but we don't handle it at present.
    // ASSERT(parse_http_params(&params, "http://example.com:123abc") == NULL);
    printf("\n...test_parse_url example complete\n");
}

int main(void) {
    test_mu_str();
    test_mu_str_example();
    test_parse_url();
}

#endif // #ifdef TEST_MU_STR
