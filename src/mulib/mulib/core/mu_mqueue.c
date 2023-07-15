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

#include "mu_mqueue.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// *****************************************************************************
// Private types and definitions

// *****************************************************************************
// Private (static) storage

// *****************************************************************************
// Private (forward) declarations

static bool access_queue(mu_mqueue_t *mqueue, void **element, bool fetch);

// *****************************************************************************
// Public code

mu_mqueue_t *mu_mqueue_init(mu_mqueue_t *mqueue, void **storage,
                            size_t capacity, mu_task_t *on_put,
                            mu_task_t *on_get) {
    mqueue->storage = storage;
    mqueue->capacity = capacity;
    mqueue->on_put = on_put;
    mqueue->on_get = on_get;
    return mu_mqueue_reset(mqueue);
}

mu_mqueue_t *mu_mqueue_reset(mu_mqueue_t *mqueue) {
    mqueue->count = 0;
    mqueue->index = 0;
    return mqueue;
}

size_t mu_mqueue_capacity(mu_mqueue_t *mqueue) { return mqueue->capacity; }

size_t mu_mqueue_count(mu_mqueue_t *mqueue) { return mqueue->count; }

bool mu_mqueue_is_empty(mu_mqueue_t *mqueue) { return mqueue->count == 0; }

bool mu_mqueue_is_full(mu_mqueue_t *mqueue) {
    return mqueue->count == mqueue->capacity;
}

bool mu_mqueue_put(mu_mqueue_t *mqueue, void *element) {
    if (!mu_mqueue_is_full(mqueue)) {
        mqueue->storage[mqueue->index] = element;
        mqueue->index = (mqueue->index + 1) % mqueue->capacity;
        mqueue->count += 1;
        mu_task_call(mqueue->on_put, NULL);
        return true;
    } else {
        return false;
    }
}

bool mu_mqueue_get(mu_mqueue_t *mqueue, void **element) {
    return access_queue(mqueue, element, true);
}

bool mu_mqueue_peek(mu_mqueue_t *mqueue, void **element) {
    return access_queue(mqueue, element, false);
}

// *****************************************************************************
// Private (static) code

static bool access_queue(mu_mqueue_t *mqueue, void **element, bool fetch) {
    if (!mu_mqueue_is_empty(mqueue)) {
        // idx = (index - count) MOD capacity, using unsigned arithmetic
        size_t idx = mqueue->capacity + mqueue->index - mqueue->count;
        if (idx >= mqueue->capacity) {
            idx -= mqueue->capacity;
        }
        *element = mqueue->storage[idx];
        if (fetch) {
            mqueue->count -= 1;
            mu_task_call(mqueue->on_get, NULL);
        }
        return true;
    } else {
        *element = NULL;
        return false;
    }
}

// *****************************************************************************
// *****************************************************************************
// Standalone tests
// *****************************************************************************
// *****************************************************************************

// Run this command in to run the standalone tests.
// gcc -Wall -DTEST_MU_MQUEUE -o test_mu_mqueue mu_mqueue.c mu_task.c
// && ./test_mu_mqueue && rm ./test_mu_mqueue

#ifdef TEST_MU_MQUEUE

#include <stdio.h>

#define ASSERT(e) assert(e, #e, __FILE__, __LINE__)
static void assert(bool expr, const char *str, const char *file, int line) {
    if (!expr) {
        printf("\nassertion %s failed at %s:%d", str, file, line);
    }
}

__attribute__((unused)) static void print_sem(mu_mqueue_t *sem) {}

static int put_call_count = 0;
static int get_call_count = 0;

static void on_put_fn(mu_task_t *task, void *arg) {
    (void)arg;
    put_call_count++;
}

static void on_get_fn(mu_task_t *task, void *arg) {
    (void)arg;
    get_call_count++;
}

#define MSG_QUEUE_CAPACITY 3

static void test_mu_mqueue(void) {
    printf("\nStarting test_mu_mqueue...");

    mu_mqueue_t mqueue;
    void *storage[MSG_QUEUE_CAPACITY];
    mu_task_t on_put, on_get;
    char *obj_a = "a";
    char *obj_b = "b";
    char *obj_c = "c";
    char *obj_d = "d";
    void *obj;

    mu_task_init(&on_put, on_put_fn, 0, NULL);
    mu_task_init(&on_get, on_get_fn, 0, NULL);

    // init returns sem.
    ASSERT(mu_mqueue_init(&mqueue, storage, MSG_QUEUE_CAPACITY, NULL, NULL) ==
           &mqueue);

    // on an empty queue...
    ASSERT(mu_mqueue_capacity(&mqueue) == MSG_QUEUE_CAPACITY);
    ASSERT(mu_mqueue_count(&mqueue) == 0);
    ASSERT(!mu_mqueue_is_full(&mqueue) == true);
    ASSERT(!mu_mqueue_is_empty(&mqueue) == false);

    // fill the queue...
    ASSERT(mu_mqueue_put(&mqueue, obj_a) == true);
    ASSERT(mu_mqueue_count(&mqueue) == 1);
    ASSERT(!mu_mqueue_is_full(&mqueue) == true);
    ASSERT(!mu_mqueue_is_empty(&mqueue) == true);

    ASSERT(mu_mqueue_put(&mqueue, obj_b) == true);
    ASSERT(mu_mqueue_count(&mqueue) == 2);
    ASSERT(!mu_mqueue_is_full(&mqueue) == true);
    ASSERT(!mu_mqueue_is_empty(&mqueue) == true);

    ASSERT(mu_mqueue_put(&mqueue, obj_c) == true);
    ASSERT(mu_mqueue_count(&mqueue) == MSG_QUEUE_CAPACITY);
    ASSERT(!mu_mqueue_is_full(&mqueue) == false);
    ASSERT(!mu_mqueue_is_empty(&mqueue) == true);

    // attempting to add to a full queue
    ASSERT(mu_mqueue_put(&mqueue, obj_d) == false);
    ASSERT(mu_mqueue_count(&mqueue) == MSG_QUEUE_CAPACITY);
    ASSERT(!mu_mqueue_is_full(&mqueue) == false);
    ASSERT(!mu_mqueue_is_empty(&mqueue) == true);

    // FIFO order is preserved on get

    ASSERT(mu_mqueue_get(&mqueue, &obj) == true);
    ASSERT(obj == obj_a);
    ASSERT(mu_mqueue_count(&mqueue) == 2);
    ASSERT(!mu_mqueue_is_full(&mqueue) == true);
    ASSERT(!mu_mqueue_is_empty(&mqueue) == true);

    ASSERT(mu_mqueue_get(&mqueue, &obj) == true);
    ASSERT(obj == obj_b);
    ASSERT(mu_mqueue_count(&mqueue) == 1);
    ASSERT(!mu_mqueue_is_full(&mqueue) == true);
    ASSERT(!mu_mqueue_is_empty(&mqueue) == true);

    ASSERT(mu_mqueue_get(&mqueue, &obj) == true);
    ASSERT(obj == obj_c);
    ASSERT(mu_mqueue_count(&mqueue) == 0);
    ASSERT(!mu_mqueue_is_full(&mqueue) == true);
    ASSERT(!mu_mqueue_is_empty(&mqueue) == false);

    // attempting to get on an empty queue
    ASSERT(mu_mqueue_get(&mqueue, &obj) == false);
    ASSERT(obj == NULL);
    ASSERT(mu_mqueue_count(&mqueue) == 0);
    ASSERT(!mu_mqueue_is_full(&mqueue) == true);
    ASSERT(!mu_mqueue_is_empty(&mqueue) == false);

    // mu_mqueue_reset empties the queue
    ASSERT(mu_mqueue_put(&mqueue, obj_a) == true);
    ASSERT(mu_mqueue_count(&mqueue) == 1);
    ASSERT(!mu_mqueue_is_full(&mqueue) == true);
    ASSERT(!mu_mqueue_is_empty(&mqueue) == true);

    // test callback triggers
    ASSERT(mu_mqueue_init(&mqueue, storage, MSG_QUEUE_CAPACITY, &on_put,
                          &on_get) == &mqueue);
    ASSERT(put_call_count == 0);
    ASSERT(get_call_count == 0);

    ASSERT(mu_mqueue_put(&mqueue, obj_a) == true);
    ASSERT(put_call_count == 1);
    ASSERT(get_call_count == 0);

    ASSERT(mu_mqueue_get(&mqueue, &obj) == true);
    ASSERT(put_call_count == 1);
    ASSERT(get_call_count == 1);

    printf("\n...test_mu_mqueue complete\n");
}

int main(void) { test_mu_mqueue(); }

#endif // #ifdef TEST_MU_MQUEUE
