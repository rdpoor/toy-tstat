/**
 * MIT License
 *
 * Copyright (c) 2021-2023 R. Dunbar Poor <rdpoor@gmail.com>
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
 * @file: mu_mqueue.h
 *
 * @brief A mqueue maintains a queue of one or more pointer-sized objects.
 *
 * When an element is added, mqueue optionally notifies an on_put task, and
 * when an element is removed, mqueue optionally notifies an on_get task.
 * In addition to inter-task message queues, you can use mqueue to create
 * efficent semaphores, mutexes and other forms of locks.
 */

#ifndef _MU_MQUEUE_H_
#define _MU_MQUEUE_H_

// *****************************************************************************
// Includes

#include "mu_task.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// *****************************************************************************
// C++ Compatibility

#ifdef __cplusplus
extern "C" {
#endif

// *****************************************************************************
// Public types and definitions

typedef struct {
    void **storage;    // user-supplied storage for queued items
    size_t capacity;   // maximum number of items that can be stored
    size_t count;      // number of items currently in the queue
    size_t index;      // index of next item to be stored
    mu_task_t *on_put; // task to invoke when an item is stored
    mu_task_t *on_get; // task to invoke when an item is fetched
} mu_mqueue_t;

// *****************************************************************************
// Public declarations

/**
 * @brief Initialize a message queue.
 *
 * @prarm mqueue A mqueue struct to be used in subsequent operations.
 * @param storage User-supplied storage to hold pointer-sized elements.
 * @param capacity The number of elements in the user-supplied storage.
 * @param on_put A task to be invoked when an item is added to the queue.  May
 *        be NULL, in which case no notification is made.
 * @param on_get A task to be invoked when an item is removed from the queue.
 *        May be null, in which case no notification is made.
 * @return mqueue
 */
mu_mqueue_t *mu_mqueue_init(mu_mqueue_t *mqueue, void **storage,
                            size_t capacity, mu_task_t *on_put,
                            mu_task_t *on_get);

/**
 * @brief Remove all items from the queue.
 *
 * @param mqueue The queue set up by a previous call to Mu_mqueue_init()
 * @return mqueue
 */
mu_mqueue_t *mu_mqueue_reset(mu_mqueue_t *mqueue);

/**
 * @brief Return the maximum number of elements the mqueue can hold.
 *
 * @param mqueue The queue set up by a previous call to Mu_mqueue_init()
 * @return The maximum number of element that the mqueue can hold.
 */
size_t mu_mqueue_capacity(mu_mqueue_t *mqueue);

/**
 * @brief Return the current number of elements in the mqueue.
 *
 * @param mqueue The queue set up by a previous call to Mu_mqueue_init()
 * @return The current number of elements in the mqueue.
 */
size_t mu_mqueue_count(mu_mqueue_t *mqueue);

/**
 * @brief Return true if the mqueue has zero elements.
 *
 * @param mqueue The queue set up by a previous call to Mu_mqueue_init()
 * @return true if the mqueue has zero elements.
 */
bool mu_mqueue_is_empty(mu_mqueue_t *mqueue);

/**
 * @brief Return true if the mqueue is full.
 *
 * @param mqueue The queue set up by a previous call to Mu_mqueue_init()
 * @return true if mu_mqueue_count() equals capacity.
 */
bool mu_mqueue_is_full(mu_mqueue_t *mqueue);

/**
 * @brief Insert an element into the queue.
 *
 * If the queue is not full before the call to mu_mqueue_put():
 * - The element is added to the tail of the queue
 * - The on_put task is invoked if it is non-NULL
 * - The function returns true
 *
 * If the queue is full before the call to mu_mqueue_put():
 * - The function returns false
 *
 * @param mqueue The queue set up by a previous call to Mu_mqueue_init()
 * @param element The pointer-sized element to be added to the queue
 * @return True if the element was added, false otherwise.
 */
bool mu_mqueue_put(mu_mqueue_t *mqueue, void *element);

/**
 * @brief Remove an element from the queue.
 *
 * If the queue is not empty before the call to mu_mqueue_get():
 * - The element is removed from the head of the queue
 * - The on_get task is invoked if it is non-NULL
 * - The function returns true
 *
 * If the queue is empty before the call to mu_mqueue_get():
 * - The function returns false
 *
 * @param mqueue The queue set up by a previous call to Mu_mqueue_init()
 * @param element A pointer to the location to receive the element
 * @return True if the element was fetched, false otherwise.
 */
bool mu_mqueue_get(mu_mqueue_t *mqueue, void **element);

/**
 * @brief Return the head of the queue.
 *
 * If the queue is not empty before the call to mu_mqueue_get():
 * - The element at the head of the queue is returned by reference
 * - The function returns true
 *
 * If the queue is empty before the call to mu_mqueue_get():
 * - The function returns false
 *
 * @param mqueue The queue set up by a previous call to Mu_mqueue_init()
 * @param element A pointer to the location to receive the element
 * @return True if the element was present, false otherwise.
 */
bool mu_mqueue_peek(mu_mqueue_t *mqueue, void **element);

// *****************************************************************************
// End of file

#ifdef __cplusplus
}
#endif

#endif /* #ifndef _MU_MQUEUE_H_ */
