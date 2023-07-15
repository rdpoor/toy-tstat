/**
 * @file test_mu_mqueue.c
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

#include "mu_mqueue.h"
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

void test_mu_mqueue(void) {
    printf("\nStarting test_mu_mqueue...");

    int item1 = 1;
    int item2 = 2;
    int item3 = 3;
    int item4 = 4;
    int item5 = 5;
    int item6 = 6;
    void *element;
    mu_mqueue_t mqueue;
    void *storage[5];

    // mqueue initializes properly
    MU_ASSERT(mu_mqueue_init(&mqueue, storage, 5, NULL, NULL) == &mqueue);
    MU_ASSERT(mu_mqueue_capacity(&mqueue) == 5);
    MU_ASSERT(mu_mqueue_count(&mqueue) == 0);
    MU_ASSERT(mu_mqueue_is_empty(&mqueue) == true);
    MU_ASSERT(mu_mqueue_is_full(&mqueue) == false);

    // can put 5 (and only 5) elements in the MQUEUE
    MU_ASSERT(mu_mqueue_put(&mqueue, &item1) == true);
    MU_ASSERT(mu_mqueue_is_empty(&mqueue) == false);
    MU_ASSERT(mu_mqueue_is_full(&mqueue) == false);
    MU_ASSERT(mu_mqueue_put(&mqueue, &item2) == true);
    MU_ASSERT(mu_mqueue_put(&mqueue, &item3) == true);
    MU_ASSERT(mu_mqueue_put(&mqueue, &item4) == true);
    MU_ASSERT(mu_mqueue_put(&mqueue, &item5) == true);
    MU_ASSERT(mu_mqueue_is_empty(&mqueue) == false);
    MU_ASSERT(mu_mqueue_is_full(&mqueue) == true);
    MU_ASSERT(mu_mqueue_put(&mqueue, &item6) == false);

    // items come out in FIFO order
    MU_ASSERT(mu_mqueue_get(&mqueue, &element) == true);
    MU_ASSERT(element == &item1);
    MU_ASSERT(mu_mqueue_is_empty(&mqueue) == false);
    MU_ASSERT(mu_mqueue_is_full(&mqueue) == false);
    MU_ASSERT(mu_mqueue_get(&mqueue, &element) == true);
    MU_ASSERT(element == &item2);
    MU_ASSERT(mu_mqueue_get(&mqueue, &element) == true);
    MU_ASSERT(element == &item3);
    MU_ASSERT(mu_mqueue_get(&mqueue, &element) == true);
    MU_ASSERT(element == &item4);
    MU_ASSERT(mu_mqueue_get(&mqueue, &element) == true);
    MU_ASSERT(element == &item5);
    MU_ASSERT(mu_mqueue_is_empty(&mqueue) == true);
    MU_ASSERT(mu_mqueue_is_full(&mqueue) == false);
    MU_ASSERT(mu_mqueue_get(&mqueue, &element) == false);

    // reset clears the queue
    MU_ASSERT(mu_mqueue_put(&mqueue, &item1) == true);
    MU_ASSERT(mu_mqueue_reset(&mqueue) == &mqueue);
    MU_ASSERT(mu_mqueue_capacity(&mqueue) == 5);
    MU_ASSERT(mu_mqueue_count(&mqueue) == 0);
    MU_ASSERT(mu_mqueue_is_empty(&mqueue) == true);
    MU_ASSERT(mu_mqueue_is_full(&mqueue) == false);

    // on_put and on_get are called as expected.
    counting_obj_t on_put;
    counting_obj_t on_get;

    counting_obj_init(&on_put);
    counting_obj_init(&on_get);
    MU_ASSERT(mu_mqueue_init(&mqueue, storage, 5, counting_obj_task(&on_put),
                             counting_obj_task(&on_get)) == &mqueue);
    MU_ASSERT(counting_obj_get_call_count(&on_put) == 0);
    MU_ASSERT(counting_obj_get_call_count(&on_get) == 0);
    MU_ASSERT(mu_mqueue_put(&mqueue, &item1) == true);
    MU_ASSERT(counting_obj_get_call_count(&on_put) == 1);
    MU_ASSERT(counting_obj_get_call_count(&on_get) == 0);
    MU_ASSERT(mu_mqueue_put(&mqueue, &item2) == true);
    MU_ASSERT(counting_obj_get_call_count(&on_put) == 2);
    MU_ASSERT(counting_obj_get_call_count(&on_get) == 0);
    MU_ASSERT(mu_mqueue_put(&mqueue, &item3) == true);
    MU_ASSERT(counting_obj_get_call_count(&on_put) == 3);
    MU_ASSERT(counting_obj_get_call_count(&on_get) == 0);
    MU_ASSERT(mu_mqueue_put(&mqueue, &item4) == true);
    MU_ASSERT(counting_obj_get_call_count(&on_put) == 4);
    MU_ASSERT(counting_obj_get_call_count(&on_get) == 0);
    MU_ASSERT(mu_mqueue_put(&mqueue, &item5) == true);
    MU_ASSERT(counting_obj_get_call_count(&on_put) == 5);
    MU_ASSERT(counting_obj_get_call_count(&on_get) == 0);
    // on_put is not called if queue was full...
    MU_ASSERT(mu_mqueue_put(&mqueue, &item6) == false);
    MU_ASSERT(counting_obj_get_call_count(&on_put) == 5);
    MU_ASSERT(counting_obj_get_call_count(&on_get) == 0);

    MU_ASSERT(mu_mqueue_get(&mqueue, &element) == true);
    MU_ASSERT(counting_obj_get_call_count(&on_put) == 5);
    MU_ASSERT(counting_obj_get_call_count(&on_get) == 1);
    MU_ASSERT(mu_mqueue_get(&mqueue, &element) == true);
    MU_ASSERT(counting_obj_get_call_count(&on_put) == 5);
    MU_ASSERT(counting_obj_get_call_count(&on_get) == 2);
    MU_ASSERT(mu_mqueue_get(&mqueue, &element) == true);
    MU_ASSERT(counting_obj_get_call_count(&on_put) == 5);
    MU_ASSERT(counting_obj_get_call_count(&on_get) == 3);
    MU_ASSERT(mu_mqueue_get(&mqueue, &element) == true);
    MU_ASSERT(counting_obj_get_call_count(&on_put) == 5);
    MU_ASSERT(counting_obj_get_call_count(&on_get) == 4);
    MU_ASSERT(mu_mqueue_get(&mqueue, &element) == true);
    MU_ASSERT(counting_obj_get_call_count(&on_put) == 5);
    MU_ASSERT(counting_obj_get_call_count(&on_get) == 5);
    // on_get is not called if queue was empty
    MU_ASSERT(mu_mqueue_get(&mqueue, &element) == false);
    MU_ASSERT(counting_obj_get_call_count(&on_put) == 5);
    MU_ASSERT(counting_obj_get_call_count(&on_get) == 5);

    printf("\n   Completed test_mu_mqueue.");
}

// *****************************************************************************
// Local (private, static) code
