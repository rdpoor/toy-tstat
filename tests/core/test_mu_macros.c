/**
 * @file test_mu_macros.c
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

#include "mu_macros.h"
#include "test_support.h"
#include <stdio.h>

// *****************************************************************************
// Local (private) types and definitions

// *****************************************************************************
// Local (private, static) forward declarations

// *****************************************************************************
// Local (private, static) storage

// *****************************************************************************
// Public code

void test_mu_macros(void) {
    printf("\nStarting test_mu_macros...");

    // printf("Running test_MU_MAX...\n");
    MU_ASSERT(MU_MAX(1, 2) == 2);
    MU_ASSERT(MU_MAX(-1, -2) == -1);
    MU_ASSERT(MU_MAX(3.2, 3.0) == 3.2);
    MU_ASSERT(MU_MAX(0, 0) == 0);

    // printf("Running test_MU_MIN...\n");
    MU_ASSERT(MU_MIN(1, 2) == 1);
    MU_ASSERT(MU_MIN(-1, -2) == -2);
    MU_ASSERT(MU_MIN(3.2, 3.0) == 3.0);
    MU_ASSERT(MU_MIN(0, 0) == 0);

    // printf("Running test_MU_ABS...\n");
    MU_ASSERT(MU_ABS(5) == 5);
    MU_ASSERT(MU_ABS(-5) == 5);
    MU_ASSERT(MU_ABS(0) == 0);
    MU_ASSERT(MU_ABS(3.5) == 3.5);
    MU_ASSERT(MU_ABS(-3.5) == 3.5);

    // printf("Running test_MU_SIGNUM...\n");
    MU_ASSERT(MU_SIGNUM(5) == 1);
    MU_ASSERT(MU_SIGNUM(-5) == -1);
    MU_ASSERT(MU_SIGNUM(0) == 0);
    MU_ASSERT(MU_SIGNUM(3.5) == 1);
    MU_ASSERT(MU_SIGNUM(-3.5) == -1);

    // printf("Running test_MU_CLAMP...\n");
    MU_ASSERT(MU_CLAMP(1, 2, 3) == 2);
    MU_ASSERT(MU_CLAMP(1, 0, 3) == 1);
    MU_ASSERT(MU_CLAMP(1, 4, 3) == 3);
    MU_ASSERT(MU_CLAMP(-5, 0, 5) == 0);
    MU_ASSERT(MU_CLAMP(0, 5, 0) == 0);

    printf("\n   Completed test_mu_macros.");

}

// *****************************************************************************
// Local (private, static) code
