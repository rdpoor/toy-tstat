/**
 * @file test_mu_time.c
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

#include "mu_time.h"
#include "test_support.h"
#include <stdbool.h>
#include <stdio.h>

// *****************************************************************************
// Local (private) types and definitions

// *****************************************************************************
// Local (private, static) forward declarations

// *****************************************************************************
// Local (private, static) storage

// *****************************************************************************
// Public code

void test_mu_time(void) {
    printf("\nStarting test_mu_time...");
    // NOTE: the only published way to create a mu_time_abs_t object is by a
    // call to mu_time_now().  And the only way to create a mu_time_rel_t is
    // by a call to mu_time_ms_to_rel().  So we derive everything from those
    // two functions.
    //
    // However, this does not give us an implementation indepdendent means for
    // testing for rollover, etc.

    mu_time_init();

    mu_time_abs_t now = mu_time_now();
    mu_time_rel_t z_rel = mu_time_ms_to_rel(0);
    mu_time_rel_t p_rel = mu_time_ms_to_rel(1);
    mu_time_rel_t m_rel = mu_time_ms_to_rel(-1);

    // mu_time_abs_t mu_time_offset(mu_time_abs_t t, mu_time_rel_t dt);
    mu_time_abs_t now_z = mu_time_offset(now, z_rel);
    mu_time_abs_t now_p = mu_time_offset(now, p_rel);
    mu_time_abs_t now_m = mu_time_offset(now, m_rel);

    MU_ASSERT(mu_time_equals(now_z, now));
    MU_ASSERT(mu_time_equals(mu_time_offset(now_p, m_rel), now));
    MU_ASSERT(mu_time_equals(mu_time_offset(now_m, p_rel), now));

    // mu_time_rel_t mu_time_difference(mu_time_abs_t t1, mu_time_abs_t t2);
    MU_ASSERT(mu_time_equals(mu_time_difference(now, now_z), z_rel));
    MU_ASSERT(mu_time_equals(mu_time_difference(now_z, now), z_rel));
    MU_ASSERT(mu_time_equals(mu_time_difference(now, now_p), m_rel));
    MU_ASSERT(mu_time_equals(mu_time_difference(now_p, now), p_rel));
    MU_ASSERT(mu_time_equals(mu_time_difference(now, now_m), p_rel));
    MU_ASSERT(mu_time_equals(mu_time_difference(now_m, now), m_rel));

    // bool mu_time_precedes(mu_time_abs_t t1, mu_time_abs_t t2);
    MU_ASSERT(mu_time_precedes(now, now_z) == false);
    MU_ASSERT(mu_time_precedes(now_z, now) == false);
    MU_ASSERT(mu_time_precedes(now, now_p) == true);
    MU_ASSERT(mu_time_precedes(now_p, now) == false);
    MU_ASSERT(mu_time_precedes(now, now_m) == false);
    MU_ASSERT(mu_time_precedes(now_m, now) == true);

    // bool mu_time_equals(mu_time_abs_t t1, mu_time_abs_t t2);
    MU_ASSERT(mu_time_equals(now, now) == true);
    MU_ASSERT(mu_time_equals(now, now_p) == false);
    MU_ASSERT(mu_time_equals(now_p, now) == false);

    // bool mu_time_follows(mu_time_abs_t t1, mu_time_abs_t t2);
    MU_ASSERT(mu_time_follows(now, now_z) == false);
    MU_ASSERT(mu_time_follows(now_z, now) == false);
    MU_ASSERT(mu_time_follows(now, now_p) == false);
    MU_ASSERT(mu_time_follows(now_p, now) == true);
    MU_ASSERT(mu_time_follows(now, now_m) == true);
    MU_ASSERT(mu_time_follows(now_m, now) == false);

    // int mu_time_rel_to_ms(mu_time_rel_t dt);
    MU_ASSERT(mu_time_rel_to_ms(z_rel) == 0);
    MU_ASSERT(mu_time_rel_to_ms(p_rel) == 1);
    MU_ASSERT(mu_time_rel_to_ms(m_rel) == -1);

    // mu_time_rel_t mu_time_ms_to_rel(int ms);
    // assumed...

    printf("\n   Completed test_mu_time.");
}

// *****************************************************************************
// Local (private, static) code
