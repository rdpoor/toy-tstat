/**
 * @file test_mu_str.c
 *
 * MIT License
 *
 * Copyright (c) 2022 - 2023 R. Dunbar Poor
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

#include "mu_str.h"
#include "test_support.h"
#include <stdio.h>
#include <string.h>

// *****************************************************************************
// Local (private) types and definitions

typedef struct {
  const char *host_name;
  size_t host_name_len;
  uint16_t host_port;
  bool use_tls;
} http_params_t;

#define HTTP_PREFIX "http://"
#define HTTPS_PREFIX "https://"

// *****************************************************************************
// Local (private, static) forward declarations

// Return true if str->bytes equals cstr
static bool cstr_eq(mu_str_t *str, const char *cstr);

static bool is_member(uint8_t byte, const char *bytes);

static bool is_numeric(uint8_t byte, void *arg);

static bool is_hexadecimal(uint8_t byte, void *arg);

static bool is_whitespace(uint8_t byte, void *arg);

static bool is_x(uint8_t byte, void *arg);

static bool is_never(uint8_t byte, void *arg);

static bool is_always(uint8_t byte, void *arg);

__attribute__((unused)) static void print_str(mu_str_t *str);

// *****************************************************************************
// Local (private, static) storage

// *****************************************************************************
// Public code

void test_mu_str(void) {

  printf("\nStarting test_mu_str...");
  fflush(stdout);

  // mu_str_init
  do {
    mu_str_t s1;
    const uint8_t buf[] = {65, 66, 67, 68, 69, 70, 71, 72, 73, 74};

    MU_ASSERT(&s1 == mu_str_init(&s1, buf, sizeof(buf)));
    MU_ASSERT(mu_str_bytes(&s1) == buf);
    MU_ASSERT(mu_str_length(&s1) == sizeof(buf));
  } while (false);

  // mu_str_init_cstr
  do {
    mu_str_t s1;
    char *cstr = "ABCDEFGHIJ";

    MU_ASSERT(&s1 == mu_str_init_cstr(&s1, cstr));
    MU_ASSERT(mu_str_bytes(&s1) == (const uint8_t *)cstr);
    MU_ASSERT(mu_str_length(&s1) == strlen(cstr));
  } while (false);

  // mu_str_copy
  do {
    mu_str_t s1, s2;
    uint8_t buf[10];
    mu_str_init(&s1, buf, sizeof(buf));

    MU_ASSERT(&s2 == mu_str_copy(&s2, &s1));
    MU_ASSERT(mu_str_bytes(&s2) == mu_str_bytes(&s1));
    MU_ASSERT(mu_str_length(&s2) == mu_str_length(&s1));
  } while (false);

  // mu_str_compare
  do {
    mu_str_t s1, s2;

    mu_str_init_cstr(&s1, "abcd");
    // strings are equal in content and length
    MU_ASSERT(mu_str_compare(&s1, mu_str_init_cstr(&s2, "abcd")) == 0);
    // s1 is lexographically higher
    MU_ASSERT(mu_str_compare(&s1, mu_str_init_cstr(&s2, "abcc")) > 0);
    // s1 is lexographically lower
    MU_ASSERT(mu_str_compare(&s1, mu_str_init_cstr(&s2, "abce")) < 0);
    // s1 is longer
    MU_ASSERT(mu_str_compare(&s1, mu_str_init_cstr(&s2, "abc")) > 0);
    // s1 is shorter
    MU_ASSERT(mu_str_compare(&s1, mu_str_init_cstr(&s2, "abcde")) < 0);
    // both empty
    MU_ASSERT(mu_str_compare(mu_str_init_cstr(&s1, ""),
                          mu_str_init_cstr(&s2, "")) == 0);
    // s2 empty
    MU_ASSERT(mu_str_compare(mu_str_init_cstr(&s1, "abcd"),
                          mu_str_init_cstr(&s2, "")) > 0);
    // s1 empty
    MU_ASSERT(mu_str_compare(mu_str_init_cstr(&s1, ""),
                          mu_str_init_cstr(&s2, "abcd")) < 0);
  } while (false);

  // mu_str_slice
  do {
    mu_str_t s1, s2;

    mu_str_init_cstr(&s1, "ABCDEFGHIJ");
    // whole slice (indefinite end index)
    MU_ASSERT(&s2 == mu_str_slice(&s2, &s1, 0, MU_STR_END));
    MU_ASSERT(cstr_eq(&s2, "ABCDEFGHIJ"));
    // whole slice (definite end index)
    MU_ASSERT(&s2 == mu_str_slice(&s2, &s1, 0, mu_str_length(&s1)));
    MU_ASSERT(cstr_eq(&s2, "ABCDEFGHIJ"));

    // remove first char (positive start index)
    MU_ASSERT(&s2 == mu_str_slice(&s2, &s1, 1, MU_STR_END));
    MU_ASSERT(cstr_eq(&s2, "BCDEFGHIJ"));
    // remove first char (negative start index)
    MU_ASSERT(&s2 == mu_str_slice(&s2, &s1, -9, MU_STR_END));
    MU_ASSERT(cstr_eq(&s2, "BCDEFGHIJ"));

    // remove last char (positive end index)
    MU_ASSERT(&s2 == mu_str_slice(&s2, &s1, 0, 9));
    MU_ASSERT(cstr_eq(&s2, "ABCDEFGHI"));
    // remove last char (negative end index)
    MU_ASSERT(&s2 == mu_str_slice(&s2, &s1, 0, -1));
    MU_ASSERT(cstr_eq(&s2, "ABCDEFGHI"));

    // extract middle chars (positive indeces)
    MU_ASSERT(&s2 == mu_str_slice(&s2, &s1, 3, 7));
    MU_ASSERT(cstr_eq(&s2, "DEFG"));
    // extract middle chars (negative indeces)
    MU_ASSERT(&s2 == mu_str_slice(&s2, &s1, -7, -3));
    MU_ASSERT(cstr_eq(&s2, "DEFG"));

    // start == end
    MU_ASSERT(&s2 == mu_str_slice(&s2, &s1, 5, 5));
    MU_ASSERT(cstr_eq(&s2, ""));
    // start > end
    MU_ASSERT(&s2 == mu_str_slice(&s2, &s1, 6, 5));
    MU_ASSERT(cstr_eq(&s2, ""));

    // start > end of string
    MU_ASSERT(&s2 == mu_str_slice(&s2, &s1, 20, mu_str_length(&s1)));
    MU_ASSERT(cstr_eq(&s2, ""));
    // end < beginnig of string
    MU_ASSERT(&s2 == mu_str_slice(&s2, &s1, 0, -20));
    MU_ASSERT(cstr_eq(&s2, ""));

  } while (false);

  // bool mu_str_has_prefix(mu_str_t *s1, mu_str_t *s2);
  // bool mu_str_has_prefix_cstr(mu_str_t *s1, const char *cstr);
  // bool mu_str_has_suffix(mu_str_t *s1, mu_str_t *s2);
  // bool mu_str_has_suffix_cstr(mu_str_t *s1, const char *cstr);
  do {
    mu_str_t s1, s2;
    mu_str_init_cstr(&s1, "abcd");

    MU_ASSERT(mu_str_has_prefix(&s1, mu_str_init_cstr(&s2, "")) == true);
    MU_ASSERT(mu_str_has_prefix(&s1, mu_str_init_cstr(&s2, "ab")) == true);
    MU_ASSERT(mu_str_has_prefix(&s1, mu_str_init_cstr(&s2, "cd")) == false);
    MU_ASSERT(mu_str_has_prefix(&s1, mu_str_init_cstr(&s2, "abcd")) == true);
    MU_ASSERT(mu_str_has_prefix(&s1, mu_str_init_cstr(&s2, "abcde")) == false);

    MU_ASSERT(mu_str_has_prefix_cstr(&s1, "") == true);
    MU_ASSERT(mu_str_has_prefix_cstr(&s1, "ab") == true);
    MU_ASSERT(mu_str_has_prefix_cstr(&s1, "cd") == false);
    MU_ASSERT(mu_str_has_prefix_cstr(&s1, "abcd") == true);
    MU_ASSERT(mu_str_has_prefix_cstr(&s1, "abcde") == false);

    MU_ASSERT(mu_str_has_suffix(&s1, mu_str_init_cstr(&s2, "")) == true);
    MU_ASSERT(mu_str_has_suffix(&s1, mu_str_init_cstr(&s2, "ab")) == false);
    MU_ASSERT(mu_str_has_suffix(&s1, mu_str_init_cstr(&s2, "cd")) == true);
    MU_ASSERT(mu_str_has_suffix(&s1, mu_str_init_cstr(&s2, "abcd")) == true);
    MU_ASSERT(mu_str_has_suffix(&s1, mu_str_init_cstr(&s2, "abcde")) == false);

    MU_ASSERT(mu_str_has_suffix_cstr(&s1, "") == true);
    MU_ASSERT(mu_str_has_suffix_cstr(&s1, "ab") == false);
    MU_ASSERT(mu_str_has_suffix_cstr(&s1, "cd") == true);
    MU_ASSERT(mu_str_has_suffix_cstr(&s1, "abcd") == true);
    MU_ASSERT(mu_str_has_suffix_cstr(&s1, "abcde") == false);

  } while (false);

  // size_t mu_str_find(mu_str_t *haystack, mu_str_t *needle, bool skip_substr);
  // size_t mu_str_find_cstr(mu_str_t *haystack, const char *needle, bool
  // skip_substr); size_t mu_str_rfind(mu_str_t *haystack, mu_str_t *needle,
  // bool skip_substr); size_t mu_str_rfind_cstr(mu_str_t *haystack, const char
  // *needle, bool skip_substr);
  do {
    mu_str_t s1, s2;

    //                     0123456789
    mu_str_init_cstr(&s1, "abXcdabYcd");
    MU_ASSERT(mu_str_find(&s1, mu_str_init_cstr(&s2, ""), false) == 0);
    MU_ASSERT(mu_str_find(&s1, mu_str_init_cstr(&s2, ""), true) == 0);
    MU_ASSERT(mu_str_find(&s1, mu_str_init_cstr(&s2, "ab"), false) == 0);
    MU_ASSERT(mu_str_find(&s1, mu_str_init_cstr(&s2, "ab"), true) == 2);
    MU_ASSERT(mu_str_find(&s1, mu_str_init_cstr(&s2, "cd"), false) == 3);
    MU_ASSERT(mu_str_find(&s1, mu_str_init_cstr(&s2, "cd"), true) == 5);
    MU_ASSERT(mu_str_find(&s1, mu_str_init_cstr(&s2, "cdX"), false) ==
           MU_STR_NOT_FOUND);
    MU_ASSERT(mu_str_find(&s1, mu_str_init_cstr(&s2, "cdX"), true) ==
           MU_STR_NOT_FOUND);

    MU_ASSERT(mu_str_find_cstr(&s1, "", false) == 0);
    MU_ASSERT(mu_str_find_cstr(&s1, "", true) == 0);
    MU_ASSERT(mu_str_find_cstr(&s1, "ab", false) == 0);
    MU_ASSERT(mu_str_find_cstr(&s1, "ab", true) == 2);
    MU_ASSERT(mu_str_find_cstr(&s1, "cd", false) == 3);
    MU_ASSERT(mu_str_find_cstr(&s1, "cd", true) == 5);
    MU_ASSERT(mu_str_find_cstr(&s1, "cdX", false) == MU_STR_NOT_FOUND);
    MU_ASSERT(mu_str_find_cstr(&s1, "cdX", true) == MU_STR_NOT_FOUND);

    MU_ASSERT(mu_str_rfind(&s1, mu_str_init_cstr(&s2, ""), false) == 10);
    MU_ASSERT(mu_str_rfind(&s1, mu_str_init_cstr(&s2, ""), true) == 10);
    MU_ASSERT(mu_str_rfind(&s1, mu_str_init_cstr(&s2, "ab"), false) == 5);
    MU_ASSERT(mu_str_rfind(&s1, mu_str_init_cstr(&s2, "ab"), true) == 7);
    MU_ASSERT(mu_str_rfind(&s1, mu_str_init_cstr(&s2, "cd"), false) == 8);
    MU_ASSERT(mu_str_rfind(&s1, mu_str_init_cstr(&s2, "cd"), true) == 10);
    MU_ASSERT(mu_str_rfind(&s1, mu_str_init_cstr(&s2, "cdX"), false) ==
           MU_STR_NOT_FOUND);
    MU_ASSERT(mu_str_rfind(&s1, mu_str_init_cstr(&s2, "cdX"), true) ==
           MU_STR_NOT_FOUND);

    MU_ASSERT(mu_str_rfind_cstr(&s1, "", false) == 10);
    MU_ASSERT(mu_str_rfind_cstr(&s1, "", true) == 10);
    MU_ASSERT(mu_str_rfind_cstr(&s1, "ab", false) == 5);
    MU_ASSERT(mu_str_rfind_cstr(&s1, "ab", true) == 7);
    MU_ASSERT(mu_str_rfind_cstr(&s1, "cd", false) == 8);
    MU_ASSERT(mu_str_rfind_cstr(&s1, "cd", true) == 10);
    MU_ASSERT(mu_str_rfind_cstr(&s1, "cdX", false) == MU_STR_NOT_FOUND);
    MU_ASSERT(mu_str_rfind_cstr(&s1, "cdX", true) == MU_STR_NOT_FOUND);
  } while (false);

  // regression test
  do {
    mu_str_t x;
    mu_str_init_cstr(&x, "A\r\n");
    size_t idx = mu_str_find_cstr(&x, "\r\n", false);
    MU_ASSERT(idx == 1);
  } while (false);

  // size_t mu_str_match(mu_str_t *str, mu_str_predicate_t pred, void *arg);
  // size_t mu_str_rmatch(mu_str_t *str, mu_str_predicate_t pred, void *arg);
  do {
    mu_str_t s1;

    mu_str_init_cstr(&s1, "0123");
    // finds the very first char '0'
    MU_ASSERT(mu_str_match(&s1, is_numeric, NULL, true) == 0); // found '0'
    // hits the end of string without a "not match"
    MU_ASSERT(mu_str_match(&s1, is_numeric, NULL, false) == MU_STR_NOT_FOUND);
    // finds the very last char '3'
    MU_ASSERT(mu_str_rmatch(&s1, is_numeric, NULL, true) == 3); // found '3'
    // hits the begining of string without a "not match"
    MU_ASSERT(mu_str_rmatch(&s1, is_numeric, NULL, false) == MU_STR_NOT_FOUND);

    //                     00000000001111111 1 1 1 2 2
    //                     01234567890123456 7 8 9 0 1
    mu_str_init_cstr(&s1, "0123456789abcDEF \r\n\t\v\f");

    MU_ASSERT(mu_str_match(&s1, is_numeric, NULL, true) == 0);
    MU_ASSERT(mu_str_match(&s1, is_numeric, NULL, false) == 10);
    MU_ASSERT(mu_str_match(&s1, is_hexadecimal, NULL, true) == 0);
    MU_ASSERT(mu_str_match(&s1, is_hexadecimal, NULL, false) == 16);
    MU_ASSERT(mu_str_match(&s1, is_whitespace, NULL, true) == 16);
    MU_ASSERT(mu_str_match(&s1, is_whitespace, NULL, false) == 0);
    MU_ASSERT(mu_str_match(&s1, is_x, NULL, true) == MU_STR_NOT_FOUND);
    MU_ASSERT(mu_str_match(&s1, is_x, NULL, false) == 0);

    MU_ASSERT(mu_str_rmatch(&s1, is_numeric, NULL, true) == 9);
    MU_ASSERT(mu_str_rmatch(&s1, is_numeric, NULL, false) == 21);
    MU_ASSERT(mu_str_rmatch(&s1, is_hexadecimal, NULL, true) == 15);
    MU_ASSERT(mu_str_rmatch(&s1, is_hexadecimal, NULL, false) == 21);
    MU_ASSERT(mu_str_rmatch(&s1, is_whitespace, NULL, true) == 21);
    MU_ASSERT(mu_str_rmatch(&s1, is_whitespace, NULL, false) == 15);
    MU_ASSERT(mu_str_rmatch(&s1, is_x, NULL, true) == MU_STR_NOT_FOUND);
    MU_ASSERT(mu_str_rmatch(&s1, is_x, NULL, false) == 21);

    mu_str_init_cstr(&s1, "");
    MU_ASSERT(mu_str_match(&s1, is_never, NULL, true) == MU_STR_NOT_FOUND);
    MU_ASSERT(mu_str_rmatch(&s1, is_never, NULL, false) == MU_STR_NOT_FOUND);
    MU_ASSERT(mu_str_match(&s1, is_always, NULL, true) == MU_STR_NOT_FOUND);
    MU_ASSERT(mu_str_rmatch(&s1, is_always, NULL, false) == MU_STR_NOT_FOUND);

  } while (false);

  // mu_str_ltrim, mu_str_rtrim, mu_str_trim
  do {
    mu_str_t s1;

    mu_str_init_cstr(&s1, "  abcde  ");
    MU_ASSERT(&s1 == mu_str_ltrim(&s1, is_whitespace, NULL));
    MU_ASSERT(cstr_eq(&s1, "abcde  "));

    mu_str_init_cstr(&s1, "  abcde  ");
    MU_ASSERT(&s1 == mu_str_rtrim(&s1, is_whitespace, NULL));
    MU_ASSERT(cstr_eq(&s1, "  abcde"));

    mu_str_init_cstr(&s1, "  abcde  ");
    MU_ASSERT(&s1 == mu_str_trim(&s1, is_whitespace, NULL));
    MU_ASSERT(cstr_eq(&s1, "abcde"));
  } while (false);

  do {
    mu_str_t s1;
    char buf[5];  // 4 chars max (plus null termination)

    mu_str_init_cstr(&s1, "abcd");
    MU_ASSERT(mu_str_to_cstr(&s1, buf, sizeof(buf)) == true);
    MU_ASSERT(buf[0] == 'a');  // strcmp avoidance...
    MU_ASSERT(buf[1] == 'b');
    MU_ASSERT(buf[2] == 'c');
    MU_ASSERT(buf[3] == 'd');

    mu_str_init_cstr(&s1, "abcde");
    MU_ASSERT(mu_str_to_cstr(&s1, buf, sizeof(buf)) == false);
  } while(false);

  printf("\n   Completed test_mu_str.");
}

#if 0
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
  MU_ASSERT(idx != MU_STR_NOT_FOUND);
  mu_str_slice(&date_value, &html, idx, MU_STR_END);
  idx = mu_str_find_cstr(&date_value, "\r\n", false);
  MU_ASSERT(idx != MU_STR_NOT_FOUND);
  mu_str_slice(&date_value, &date_value, 0, idx);
  MU_ASSERT(cstr_eq(&date_value, "Wed, 26 Oct 2022 17:17:34 GMT"));

  // find "{\"code\":200,\"message\":\"ok\"}"
  // blank \r\n\r\n signifies end of HTML header and start of body
  idx = mu_str_find_cstr(&html, "\r\n\r\n", true);
  MU_ASSERT(idx != MU_STR_NOT_FOUND);
  mu_str_slice(&body, &html, idx, MU_STR_END);
  MU_ASSERT(cstr_eq(&body, "{\"code\":200,\"message\":\"ok\"}"));

  printf("\n...mu_str_example complete\n");
}

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

  MU_ASSERT(parse_http_params(&params, "http://example.com") == &params);
  MU_ASSERT(strncmp(params.host_name, "example.com", params.host_name_len) == 0);
  MU_ASSERT(params.host_port == 80);
  MU_ASSERT(params.use_tls == false);

  MU_ASSERT(parse_http_params(&params, "https://example.com") == &params);
  MU_ASSERT(strncmp(params.host_name, "example.com", params.host_name_len) == 0);
  MU_ASSERT(params.host_port == 443);
  MU_ASSERT(params.use_tls == true);

  MU_ASSERT(parse_http_params(&params, "http://example.com:8080") == &params);
  MU_ASSERT(strncmp(params.host_name, "example.com", params.host_name_len) == 0);
  MU_ASSERT(params.host_port == 8080);
  MU_ASSERT(params.use_tls == false);

  MU_ASSERT(parse_http_params(&params, "https://example.com:8080") == &params);
  MU_ASSERT(strncmp(params.host_name, "example.com", params.host_name_len) == 0);
  MU_ASSERT(params.host_port == 8080);
  MU_ASSERT(params.use_tls == true);

  MU_ASSERT(parse_http_params(&params, "https://example.com/extra") == &params);
  MU_ASSERT(strncmp(params.host_name, "example.com", params.host_name_len) == 0);
  MU_ASSERT(params.host_port == 443);
  MU_ASSERT(params.use_tls == true);

  MU_ASSERT(parse_http_params(&params, "https://example.com:123/extra") ==
         &params);
  MU_ASSERT(strncmp(params.host_name, "example.com", params.host_name_len) == 0);
  MU_ASSERT(params.host_port == 123);
  MU_ASSERT(params.use_tls == true);

  MU_ASSERT(parse_http_params(&params, "https://example.com/") == &params);
  MU_ASSERT(strncmp(params.host_name, "example.com", params.host_name_len) == 0);
  MU_ASSERT(params.host_port == 443);
  MU_ASSERT(params.use_tls == true);

  MU_ASSERT(parse_http_params(&params, "https://192.168.12.34/") == &params);
  MU_ASSERT(strncmp(params.host_name, "192.168.12.34", params.host_name_len) == 0);
  MU_ASSERT(params.host_port == 443);
  MU_ASSERT(params.use_tls == true);

  // These are the valid chars that end a port #
  // "#?/\"

  // pathologies
  MU_ASSERT(parse_http_params(&params, "") == NULL);
  MU_ASSERT(parse_http_params(&params, "http") == NULL);
  MU_ASSERT(parse_http_params(&params, "http:") == NULL);
  MU_ASSERT(parse_http_params(&params, "http:/") == NULL);
  MU_ASSERT(parse_http_params(&params, "http://") == NULL);
  MU_ASSERT(parse_http_params(&params, "http://:") == NULL);
  MU_ASSERT(parse_http_params(&params, "http://:123") == NULL);
  MU_ASSERT(parse_http_params(&params, "http://example.com:") == NULL);
  MU_ASSERT(parse_http_params(&params, "http://example.com:abc") == NULL);
  MU_ASSERT(parse_http_params(&params, "ftp://example.com:123") == NULL);
  // parse_http_params() doesn't investigate the URL past the hostname / port#
  // MU_ASSERT(parse_http_params(&params, "http://example.com/extra:123") == NULL);
  // This should be caught, but we don't handle it at present.
  // MU_ASSERT(parse_http_params(&params, "http://example.com:123abc") == NULL);
  printf("\n...test_parse_url example complete\n");
}

#endif

// *****************************************************************************
// Local (private, static) code

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

