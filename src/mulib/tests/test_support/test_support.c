/**
 * @file test_mu_timer.c
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

#include "test_support.h"

#include "mu_task.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

// *****************************************************************************
// Local (private) types and definitions

// *****************************************************************************
// Local (private, static) forward declarations

static void counting_obj_fn(mu_task_t *task, void *arg);

// *****************************************************************************
// Local (private, static) storage

// *****************************************************************************
// Public code

void _mu_assert(bool expr, const char *str, const char *file, int line) {
  if (!expr) {
    printf("\nassertion %s failed at %s:%d", str, file, line);
  }
}

counting_obj_t *counting_obj_init(counting_obj_t *counting_obj) {
	mu_task_init(&counting_obj->task, counting_obj_fn, (mu_task_state_t)0);
	return counting_obj_reset(counting_obj);
}

mu_task_t *counting_obj_task(counting_obj_t *counting_obj) {
	return &counting_obj->task;
}

counting_obj_t *counting_obj_reset(counting_obj_t *counting_obj) {
	counting_obj->call_count = 0;
	return counting_obj;
}

int counting_obj_get_call_count(counting_obj_t *counting_obj) {
	return counting_obj->call_count;
}

int counting_obj_increment_call_count(counting_obj_t *counting_obj) {
	counting_obj->call_count += 1;
	return counting_obj_get_call_count(counting_obj);
}

// *****************************************************************************
// Local (private, static) code

static void counting_obj_fn(mu_task_t *task, void *arg) {
	counting_obj_t *self = MU_TASK_CTX(task, counting_obj_t, task);
	counting_obj_increment_call_count(self);
}
