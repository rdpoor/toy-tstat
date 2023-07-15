/**
 * @file test_mu_spsc.c
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

#include "mu_spsc.h"
#include "test_support.h"
#include <stdio.h>

// *****************************************************************************
// Local (private) types and definitions

#define SPSC_SIZE 4

// *****************************************************************************
// Local (private, static) forward declarations

static void *s_store[SPSC_SIZE];

// *****************************************************************************
// Local (private, static) storage

// *****************************************************************************
// Public code

void test_mu_spsc(void) {
    printf("\nStarting test_mu_spsc...");

    mu_spsc_t spsc;
    // size must be a power of two and greater than one
    MU_ASSERT(mu_spsc_init(&spsc, s_store, 1) == MU_SPSC_ERR_SIZE);
    MU_ASSERT(mu_spsc_init(&spsc, s_store, 3) == MU_SPSC_ERR_SIZE);
    MU_ASSERT(mu_spsc_init(&spsc, s_store, SPSC_SIZE) == MU_SPSC_ERR_NONE);

    // uint16_t mu_spsc_capacity(mu_spsc_t *q);
    MU_ASSERT(mu_spsc_capacity(&spsc) == SPSC_SIZE-1);

    int item1 = 1;
    int item2 = 2;
    int item3 = 3;
    int item4 = 4;
    void *element;

    // mu_spsc_err_t mu_spsc_put(mu_spsc_t *q, mu_spsc_item_t item);
    MU_ASSERT(mu_spsc_put(&spsc, &item1) == MU_SPSC_ERR_NONE);
    MU_ASSERT(mu_spsc_put(&spsc, &item2) == MU_SPSC_ERR_NONE);
    MU_ASSERT(mu_spsc_put(&spsc, &item3) == MU_SPSC_ERR_NONE);
    MU_ASSERT(mu_spsc_put(&spsc, &item4) == MU_SPSC_ERR_FULL);

    // mu_spsc_err_t mu_spsc_get(mu_spsc_t *q, mu_spsc_item_t *item);
    // FIFO order is preserved
    MU_ASSERT(mu_spsc_get(&spsc, &element) == MU_SPSC_ERR_NONE);
    MU_ASSERT(element == &item1);
    MU_ASSERT(mu_spsc_get(&spsc, &element) == MU_SPSC_ERR_NONE);
    MU_ASSERT(element == &item2);
    MU_ASSERT(mu_spsc_get(&spsc, &element) == MU_SPSC_ERR_NONE);
    MU_ASSERT(element == &item3);
    MU_ASSERT(mu_spsc_get(&spsc, &element) == MU_SPSC_ERR_EMPTY);

    printf("\n   Completed test_mu_spsc.");
}

// *****************************************************************************
// Local (private, static) code
