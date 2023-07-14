/*
 * MIT License
 *
 * Copyright (c) 2010 Serge Zaitsev
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

#include "jsmn.h"
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

/**
 * Allocates a fresh unused token from the token pool.
 */
static jsmntok_t *jsmn_alloc_token(jsmn_parser *parser, jsmntok_t *tokens,
                                   const size_t num_tokens) {
    jsmntok_t *tok;
    if (parser->toknext >= num_tokens) {
        return NULL;
    }
    tok = &tokens[parser->toknext++];
    tok->start = tok->end = -1;
    tok->size = 0;
#ifdef JSMN_PARENT_LINKS
    tok->parent = -1;
#endif
    return tok;
}

/**
 * Fills token type and boundaries.
 */
static void jsmn_fill_token(jsmntok_t *token, const jsmntype_t type,
                            const int start, const int end) {
    token->type = type;
    token->start = start;
    token->end = end;
    token->size = 0;
}

/**
 * Fills next available token with JSON primitive.
 */
static int jsmn_parse_primitive(jsmn_parser *parser, const char *js,
                                const size_t len, jsmntok_t *tokens,
                                const size_t num_tokens) {
    jsmntok_t *token;
    int start;

    start = parser->pos;

    for (; parser->pos < len && js[parser->pos] != '\0'; parser->pos++) {
        switch (js[parser->pos]) {
#ifndef JSMN_STRICT
        /* In strict mode primitive must be followed by "," or "}" or "]" */
        case ':':
#endif
        case '\t':
        case '\r':
        case '\n':
        case ' ':
        case ',':
        case ']':
        case '}':
            goto found;
        default:
            /* to quiet a warning from gcc*/
            break;
        }
        if (js[parser->pos] < 32 || js[parser->pos] >= 127) {
            parser->pos = start;
            return JSMN_ERROR_INVAL;
        }
    }
#ifdef JSMN_STRICT
    /* In strict mode primitive must be followed by a comma/object/array */
    parser->pos = start;
    return JSMN_ERROR_PART;
#endif

found:
    if (tokens == NULL) {
        parser->pos--;
        return 0;
    }
    token = jsmn_alloc_token(parser, tokens, num_tokens);
    if (token == NULL) {
        parser->pos = start;
        return JSMN_ERROR_NOMEM;
    }
    jsmn_fill_token(token, JSMN_PRIMITIVE, start, parser->pos);
#ifdef JSMN_PARENT_LINKS
    token->parent = parser->toksuper;
#endif
    parser->pos--;
    return 0;
}

/**
 * Fills next token with JSON string.
 */
static int jsmn_parse_string(jsmn_parser *parser, const char *js,
                             const size_t len, jsmntok_t *tokens,
                             const size_t num_tokens) {
    jsmntok_t *token;

    int start = parser->pos;

    /* Skip starting quote */
    parser->pos++;

    for (; parser->pos < len && js[parser->pos] != '\0'; parser->pos++) {
        char c = js[parser->pos];

        /* Quote: end of string */
        if (c == '\"') {
            if (tokens == NULL) {
                return 0;
            }
            token = jsmn_alloc_token(parser, tokens, num_tokens);
            if (token == NULL) {
                parser->pos = start;
                return JSMN_ERROR_NOMEM;
            }
            jsmn_fill_token(token, JSMN_STRING, start + 1, parser->pos);
#ifdef JSMN_PARENT_LINKS
            token->parent = parser->toksuper;
#endif
            return 0;
        }

        /* Backslash: Quoted symbol expected */
        if (c == '\\' && parser->pos + 1 < len) {
            int i;
            parser->pos++;
            switch (js[parser->pos]) {
            /* Allowed escaped symbols */
            case '\"':
            case '/':
            case '\\':
            case 'b':
            case 'f':
            case 'r':
            case 'n':
            case 't':
                break;
            /* Allows escaped symbol \uXXXX */
            case 'u':
                parser->pos++;
                for (i = 0;
                     i < 4 && parser->pos < len && js[parser->pos] != '\0';
                     i++) {
                    /* If it isn't a hex character we have an error */
                    if (!((js[parser->pos] >= 48 &&
                           js[parser->pos] <= 57) || /* 0-9 */
                          (js[parser->pos] >= 65 &&
                           js[parser->pos] <= 70) || /* A-F */
                          (js[parser->pos] >= 97 &&
                           js[parser->pos] <= 102))) { /* a-f */
                        parser->pos = start;
                        return JSMN_ERROR_INVAL;
                    }
                    parser->pos++;
                }
                parser->pos--;
                break;
            /* Unexpected symbol */
            default:
                parser->pos = start;
                return JSMN_ERROR_INVAL;
            }
        }
    }
    parser->pos = start;
    return JSMN_ERROR_PART;
}

/**
 * Parse JSON string and fill tokens.
 */
int jsmn_parse(jsmn_parser *parser, const char *js, const size_t len,
               jsmntok_t *tokens, const unsigned int num_tokens) {
    int r;
    int i;
    jsmntok_t *token;
    int count = parser->toknext;

    for (; parser->pos < len && js[parser->pos] != '\0'; parser->pos++) {
        char c;
        jsmntype_t type;

        c = js[parser->pos];
        switch (c) {
        case '{':
        case '[':
            count++;
            if (tokens == NULL) {
                break;
            }
            token = jsmn_alloc_token(parser, tokens, num_tokens);
            if (token == NULL) {
                return JSMN_ERROR_NOMEM;
            }
            if (parser->toksuper != -1) {
                jsmntok_t *t = &tokens[parser->toksuper];
#ifdef JSMN_STRICT
                /* In strict mode an object or array can't become a key */
                if (t->type == JSMN_OBJECT) {
                    return JSMN_ERROR_INVAL;
                }
#endif
                t->size++;
#ifdef JSMN_PARENT_LINKS
                token->parent = parser->toksuper;
#endif
            }
            token->type = (c == '{' ? JSMN_OBJECT : JSMN_ARRAY);
            token->start = parser->pos;
            parser->toksuper = parser->toknext - 1;
            break;
        case '}':
        case ']':
            if (tokens == NULL) {
                break;
            }
            type = (c == '}' ? JSMN_OBJECT : JSMN_ARRAY);
#ifdef JSMN_PARENT_LINKS
            if (parser->toknext < 1) {
                return JSMN_ERROR_INVAL;
            }
            token = &tokens[parser->toknext - 1];
            for (;;) {
                if (token->start != -1 && token->end == -1) {
                    if (token->type != type) {
                        return JSMN_ERROR_INVAL;
                    }
                    token->end = parser->pos + 1;
                    parser->toksuper = token->parent;
                    break;
                }
                if (token->parent == -1) {
                    if (token->type != type || parser->toksuper == -1) {
                        return JSMN_ERROR_INVAL;
                    }
                    break;
                }
                token = &tokens[token->parent];
            }
#else
            for (i = parser->toknext - 1; i >= 0; i--) {
                token = &tokens[i];
                if (token->start != -1 && token->end == -1) {
                    if (token->type != type) {
                        return JSMN_ERROR_INVAL;
                    }
                    parser->toksuper = -1;
                    token->end = parser->pos + 1;
                    break;
                }
            }
            /* Error if unmatched closing bracket */
            if (i == -1) {
                return JSMN_ERROR_INVAL;
            }
            for (; i >= 0; i--) {
                token = &tokens[i];
                if (token->start != -1 && token->end == -1) {
                    parser->toksuper = i;
                    break;
                }
            }
#endif
            break;
        case '\"':
            r = jsmn_parse_string(parser, js, len, tokens, num_tokens);
            if (r < 0) {
                return r;
            }
            count++;
            if (parser->toksuper != -1 && tokens != NULL) {
                tokens[parser->toksuper].size++;
            }
            break;
        case '\t':
        case '\r':
        case '\n':
        case ' ':
            break;
        case ':':
            parser->toksuper = parser->toknext - 1;
            break;
        case ',':
            if (tokens != NULL && parser->toksuper != -1 &&
                tokens[parser->toksuper].type != JSMN_ARRAY &&
                tokens[parser->toksuper].type != JSMN_OBJECT) {
#ifdef JSMN_PARENT_LINKS
                parser->toksuper = tokens[parser->toksuper].parent;
#else
                for (i = parser->toknext - 1; i >= 0; i--) {
                    if (tokens[i].type == JSMN_ARRAY ||
                        tokens[i].type == JSMN_OBJECT) {
                        if (tokens[i].start != -1 && tokens[i].end == -1) {
                            parser->toksuper = i;
                            break;
                        }
                    }
                }
#endif
            }
            break;
#ifdef JSMN_STRICT
        /* In strict mode primitives are: numbers and booleans */
        case '-':
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
        case 't':
        case 'f':
        case 'n':
            /* And they must not be keys of the object */
            if (tokens != NULL && parser->toksuper != -1) {
                const jsmntok_t *t = &tokens[parser->toksuper];
                if (t->type == JSMN_OBJECT ||
                    (t->type == JSMN_STRING && t->size != 0)) {
                    return JSMN_ERROR_INVAL;
                }
            }
#else
        /* In non-strict mode every unquoted value is a primitive */
        default:
#endif
            r = jsmn_parse_primitive(parser, js, len, tokens, num_tokens);
            if (r < 0) {
                return r;
            }
            count++;
            if (parser->toksuper != -1 && tokens != NULL) {
                tokens[parser->toksuper].size++;
            }
            break;

#ifdef JSMN_STRICT
        /* Unexpected char in strict mode */
        default:
            return JSMN_ERROR_INVAL;
#endif
        }
    }

    if (tokens != NULL) {
        for (i = parser->toknext - 1; i >= 0; i--) {
            /* Unmatched opened object or array */
            if (tokens[i].start != -1 && tokens[i].end == -1) {
                return JSMN_ERROR_PART;
            }
        }
    }

    return count;
}

/**
 * Creates a new parser based over a given buffer with an array of tokens
 * available.
 */
void jsmn_init(jsmn_parser *parser) {
    parser->pos = 0;
    parser->toknext = 0;
    parser->toksuper = -1;
}

int jsmn_parse_pattern(const char *patt_str, jsmntok_t *patt_tokens,
                       int patt_count) {
    jsmn_parser parser;
    jsmn_init(&parser);

    return jsmn_parse(&parser, patt_str, strlen(patt_str), patt_tokens,
                      patt_count);
}

bool jsmn_pattern_matches(const char *str, jsmntok_t *tokens, int count,
                          const char *patt_str, jsmntok_t *patt_tokens,
                          int patt_count, bool allow_extras) {
    if (!allow_extras && (count != patt_count)) {
        return false;
    } else if (count < patt_count) {
        return false;
    }

    for (int i = 0; i < count; i++) {
        jsmntok_t *t = &tokens[i];
        jsmntok_t *p = &patt_tokens[i];

        if ((p->type == JSMN_PRIMITIVE) && (p->end - p->start == 1) &&
            (patt_str[p->start] == '?')) {
            // pattern token is a wildcard -- skip
        } else if ((p->type == JSMN_STRING) || (p->type == JSMN_PRIMITIVE)) {
            // compare only terminal tokens (strings, numbers, booleans)
            int t_len = t->end - t->start;
            int p_len = p->end - p->start;

            if (t->type != p->type) {
                // token type does not match
                return false;
            } else if (t_len != p_len) {
                // token length does not match
                return false;
            } else if (strncmp(&str[t->start], &patt_str[p->start], p_len) !=
                       0) {
                // field does not match.
                return false;
            }
        }
    }
    // made it!
    return true;
}

// *****************************************************************************
// *****************************************************************************
// Standalone Unit Tests
// *****************************************************************************
// *****************************************************************************

/* Run this command in a shell to run the standalone tests.
gcc -g -Wall -DTEST_JSMN -o test_jsmn jsmn.c && ./test_jsmn && rm -rf \
./test_jsmn*
*/

#ifdef TEST_JSMN

#include <stdio.h>
#include <string.h>

#define MAX_TOKENS 100

#define PATT_COMMAND_COUNT 7
static jsmntok_t PATT_COMMAND_TOKENS[PATT_COMMAND_COUNT];
static const char *PATT_COMMAND_STRING =
    "{\"id\":?,\"target\":?,\"command\":?}";

#define PATT_ACTION_GET_REGISTER_COUNT 5
static jsmntok_t
    PATT_ACTION_GET_REGISTER_TOKENS[PATT_ACTION_GET_REGISTER_COUNT];
static const char *PATT_ACTION_GET_REGISTER_STRING =
    "{\"action\":\"get\",\"register\":?}";

#define PATT_ACTION_SET_REGISTER_COUNT 7
static jsmntok_t
    PATT_ACTION_SET_REGISTER_TOKENS[PATT_ACTION_SET_REGISTER_COUNT];
static const char *PATT_ACTION_SET_REGISTER_STRING =
    "{\"action\":\"set\",\"register\":?,\"value\":?}";

#define PATT_ACTION_INSTALL_COUNT 7
static jsmntok_t PATT_ACTION_INSTALL_TOKENS[PATT_ACTION_INSTALL_COUNT];
static const char *PATT_ACTION_INSTALL_STRING =
    "{\"action\":\"install\",\"url\":?,\"checksum\":?}";

#define PATT_ACTION_REBOOT_COUNT 3
static jsmntok_t PATT_ACTION_REBOOT_TOKENS[PATT_ACTION_REBOOT_COUNT];
static const char *PATT_ACTION_REBOOT_STRING = "{\"action\":\"reboot\"}";

#define ASSERT(e) assert(e, #e, __FILE__, __LINE__)
static void assert(bool expr, const char *str, const char *file, int line) {
    if (!expr) {
        printf("\nassertion %s failed at %s:%d", str, file, line);
    }
}

static const char *token_type_name(jsmntype_t t) {
    switch (t) {
    case JSMN_UNDEFINED:
        return "UNDEFINED";
    case JSMN_OBJECT:
        return "OBJECT   ";
    case JSMN_ARRAY:
        return "ARRAY    ";
    case JSMN_STRING:
        return "STRING   ";
    case JSMN_PRIMITIVE:
        return "PRIMITIVE";
    default:
        return "?        ";
    }
}

static void jsmn_explain(jsmntok_t *tokens, int count, const char *str) {
    if (count < 0) {
        printf("jsmn_parse error %d\n", count);
    } else {
        for (int i = 0; i < count; i++) {
            jsmntok_t *token = &tokens[i];
            printf("\n%2d: %s %02d \'%.*s\'", i, token_type_name(token->type),
                   token->size, token->end - token->start, &str[token->start]);
        }
    }
}

static bool verify_contents(jsmntok_t *tokens, const char *expected[],
                            int count, const char *str) {
    for (int i = 0; i < count; i++) {
        jsmntok_t *token = &tokens[i];
        size_t len = token->end - token->start;
        const char *got = &str[token->start];
        if (strncmp(got, expected[i], len) != 0) {
            return false;
        }
    }
    return true;
}

static bool verify_types(jsmntok_t *tokens, jsmntype_t *expected, int count,
                         const char *str) {
    for (int i = 0; i < count; i++) {
        jsmntok_t *token = &tokens[i];
        if (expected[i] != token->type) {
            return false;
        }
    }
    return true;
}

static void test_parsing(void) {
    printf("\nStarting test_parsing...");
    fflush(stdout);

    int token_count;
    jsmntok_t tokens[MAX_TOKENS];
    jsmn_parser parser;
    const char *str;

    str = "[{\"a\":1, \"b\":2, \"c\":[4, 5, 6], \"d\":{}, \"e\":null}]";
    jsmn_init(&parser);
    token_count = jsmn_parse(&parser, str, strlen(str), tokens, MAX_TOKENS);
    ASSERT(token_count == 15);
    ASSERT(verify_types(
        tokens,
        (jsmntype_t[]){JSMN_ARRAY, JSMN_OBJECT, JSMN_STRING, JSMN_PRIMITIVE,
                       JSMN_STRING, JSMN_PRIMITIVE, JSMN_STRING, JSMN_ARRAY,
                       JSMN_PRIMITIVE, JSMN_PRIMITIVE, JSMN_PRIMITIVE,
                       JSMN_STRING, JSMN_OBJECT, JSMN_STRING, JSMN_PRIMITIVE},
        token_count, str));
    ASSERT(verify_contents(
        tokens,
        (const char *[]){
            "[{\"a\":1, \"b\":2, \"c\":[4, 5, 6], \"d\":{}, \"e\":null}]",
            "{\"a\":1, \"b\":2, \"c\":[4, 5, 6], \"d\":{}, \"e\":null}", "a",
            "1", "b", "2", "c", "[4, 5, 6]", "4", "5", "6", "d", "{}", "e",
            "null"},
        token_count, str));

    str = "[1, [2, [3, [4], 5], 6], 7]";
    jsmn_init(&parser);
    token_count = jsmn_parse(&parser, str, strlen(str), tokens, MAX_TOKENS);
    ASSERT(token_count == 11);
    ASSERT(verify_types(tokens,
                        (jsmntype_t[]){
                            JSMN_ARRAY,     // [
                            JSMN_PRIMITIVE, // 1
                            JSMN_ARRAY,     // [
                            JSMN_PRIMITIVE, // 2
                            JSMN_ARRAY,     // [
                            JSMN_PRIMITIVE, // 3
                            JSMN_ARRAY,     // [
                            JSMN_PRIMITIVE, // 4
                            JSMN_PRIMITIVE, // 5
                            JSMN_PRIMITIVE, // 6
                            JSMN_PRIMITIVE, // 7

                        },
                        token_count, str));
    ASSERT(verify_contents(tokens,
                           (const char *[]){"[1, [2, [3, [4], 5], 6], 7]", "1",
                                            "[2, [3, [4], 5], 6]", "2",
                                            "[3, [4], 5]", "3", "[4]", "4", "5",
                                            "6", "7"},
                           token_count, str));

    printf("\n...test_parsing complete\n");
}

static void explore_parsing(void) {
    printf("\nStarting explore_parsing...");
    fflush(stdout);

    int count;
    jsmntok_t tokens[MAX_TOKENS];
    jsmn_parser parser;
    const char *str;

    jsmn_init(&parser);
    str = "[{\"a\":1, \"b\":2, \"c\":[4, 5, 6], \"d\":{}, \"e\":null}]";
    count = jsmn_parse(&parser, str, strlen(str), tokens, MAX_TOKENS);
    jsmn_explain(tokens, count, str);

    printf("\n");

    jsmn_init(&parser);
    str = "[{\"a\":1, \"b\":2, \"c\":?, \"d\":?, \"e\":null}]";
    count = jsmn_parse(&parser, str, strlen(str), tokens, MAX_TOKENS);
    jsmn_explain(tokens, count, str);

    printf("\n...explore_parsing complete\n");
}

static void test_matchers(void) {
    printf("\nStarting test_matchers...");
    fflush(stdout);

    const char *str;
    int count;
    jsmntok_t tokens[MAX_TOKENS];
    jsmn_parser parser;

    ASSERT(jsmn_parse_pattern(PATT_COMMAND_STRING, PATT_COMMAND_TOKENS,
                              PATT_COMMAND_COUNT) == PATT_COMMAND_COUNT);
    ASSERT(jsmn_parse_pattern(PATT_ACTION_GET_REGISTER_STRING,
                              PATT_ACTION_GET_REGISTER_TOKENS,
                              PATT_ACTION_GET_REGISTER_COUNT) ==
           PATT_ACTION_GET_REGISTER_COUNT);
    ASSERT(jsmn_parse_pattern(PATT_ACTION_SET_REGISTER_STRING,
                              PATT_ACTION_SET_REGISTER_TOKENS,
                              PATT_ACTION_SET_REGISTER_COUNT) ==
           PATT_ACTION_SET_REGISTER_COUNT);
    ASSERT(jsmn_parse_pattern(
               PATT_ACTION_INSTALL_STRING, PATT_ACTION_INSTALL_TOKENS,
               PATT_ACTION_INSTALL_COUNT) == PATT_ACTION_INSTALL_COUNT);
    ASSERT(jsmn_parse_pattern(
               PATT_ACTION_REBOOT_STRING, PATT_ACTION_REBOOT_TOKENS,
               PATT_ACTION_REBOOT_COUNT) == PATT_ACTION_REBOOT_COUNT);

    // should match
    jsmn_init(&parser);
    str = "{\"id\":4134951269897337134,\"target\":\"F8:F0:05:9B:C5:4B\","
          "\"command\":\"Arbitrary String\"}";
    count = jsmn_parse(&parser, str, strlen(str), tokens, MAX_TOKENS);
    ASSERT(jsmn_pattern_matches(str, tokens, count, PATT_COMMAND_STRING,
                                PATT_COMMAND_TOKENS, PATT_COMMAND_COUNT,
                                false));

    // should fail with extra tokens and allow_extras = false
    jsmn_init(&parser);
    str = "{\"id\":4134951269897337134,\"target\":\"F8:F0:05:9B:C5:4B\","
          "\"command\":\"Arbitrary String\"}, 123";
    count = jsmn_parse(&parser, str, strlen(str), tokens, MAX_TOKENS);
    ASSERT(!jsmn_pattern_matches(str, tokens, count, PATT_COMMAND_STRING,
                                 PATT_COMMAND_TOKENS, PATT_COMMAND_COUNT,
                                 false));

    // should pass with extra tokens and allow_extras = true
    jsmn_init(&parser);
    str = "{\"id\":4134951269897337134,\"target\":\"F8:F0:05:9B:C5:4B\","
          "\"command\":\"Arbitrary String\"}, 123";
    count = jsmn_parse(&parser, str, strlen(str), tokens, MAX_TOKENS);
    ASSERT(jsmn_pattern_matches(str, tokens, count, PATT_COMMAND_STRING,
                                PATT_COMMAND_TOKENS, PATT_COMMAND_COUNT, true));

    // should not match ("tArGeT" field mismatch)
    jsmn_init(&parser);
    str = "{\"id\":4134951269897337134,\"tArGeT\":\"F8:F0:05:9B:C5:4B\","
          "\"command\":\"Arbitrary String\"}";
    count = jsmn_parse(&parser, str, strlen(str), tokens, MAX_TOKENS);
    ASSERT(!jsmn_pattern_matches(str, tokens, count, PATT_COMMAND_STRING,
                                 PATT_COMMAND_TOKENS, PATT_COMMAND_COUNT,
                                 true));

    // should match
    jsmn_init(&parser);
    str = "{\"action\":\"get\",\"register\":\"1\"}";
    count = jsmn_parse(&parser, str, strlen(str), tokens, MAX_TOKENS);
    ASSERT(jsmn_pattern_matches(str, tokens, count,
                                PATT_ACTION_GET_REGISTER_STRING,
                                PATT_ACTION_GET_REGISTER_TOKENS,
                                PATT_ACTION_GET_REGISTER_COUNT, false));

    // should match a number value
    jsmn_init(&parser);
    str = "{\"action\":\"set\",\"register\":\"1\",\"value\":123}";
    count = jsmn_parse(&parser, str, strlen(str), tokens, MAX_TOKENS);
    ASSERT(jsmn_pattern_matches(str, tokens, count,
                                PATT_ACTION_SET_REGISTER_STRING,
                                PATT_ACTION_SET_REGISTER_TOKENS,
                                PATT_ACTION_SET_REGISTER_COUNT, false));

    // should match a string value
    jsmn_init(&parser);
    str = "{\"action\":\"set\",\"register\":\"1\",\"value\":123}";
    count = jsmn_parse(&parser, str, strlen(str), tokens, MAX_TOKENS);
    ASSERT(jsmn_pattern_matches(str, tokens, count,
                                PATT_ACTION_SET_REGISTER_STRING,
                                PATT_ACTION_SET_REGISTER_TOKENS,
                                PATT_ACTION_SET_REGISTER_COUNT, false));

    // should match a boolean value
    jsmn_init(&parser);
    str = "{\"action\":\"set\",\"register\":\"1\",\"value\":true}";
    count = jsmn_parse(&parser, str, strlen(str), tokens, MAX_TOKENS);
    ASSERT(jsmn_pattern_matches(str, tokens, count,
                                PATT_ACTION_SET_REGISTER_STRING,
                                PATT_ACTION_SET_REGISTER_TOKENS,
                                PATT_ACTION_SET_REGISTER_COUNT, false));

    // should match install command
    jsmn_init(&parser);
    str =
        "{\"action\":\"install\",\"url\":\"https://"
        "someurl\",\"checksum\":\"6a02d9b170d97a34d52c7fc45623a3c413b11b72\"}";
    count = jsmn_parse(&parser, str, strlen(str), tokens, MAX_TOKENS);
    ASSERT(jsmn_pattern_matches(str, tokens, count, PATT_ACTION_INSTALL_STRING,
                                PATT_ACTION_INSTALL_TOKENS,
                                PATT_ACTION_INSTALL_COUNT, false));

    // should match reboot command
    jsmn_init(&parser);
    str = "{\"action\":\"reboot\"}";
    count = jsmn_parse(&parser, str, strlen(str), tokens, MAX_TOKENS);
    ASSERT(jsmn_pattern_matches(str, tokens, count, PATT_ACTION_REBOOT_STRING,
                                PATT_ACTION_REBOOT_TOKENS,
                                PATT_ACTION_REBOOT_COUNT, false));

    printf("\n...test_matchers complete\n");
}

int main(void) {
    test_parsing();
    explore_parsing();
    test_matchers();
}

#endif
