/**
 * @file sock_client.h
 *
 * MIT License
 *
 * Copyright (c) 2023 PRO1 IAQ, INC.
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
 * @brief Easy to use socket interface to a broker
 */

#ifndef _SOCK_CLIENT_H_
#define _SOCK_CLIENT_H_

// *****************************************************************************
// Includes

#include <stdint.h>

// *****************************************************************************
// C++ compatibility

#ifdef __cplusplus
extern "C" {
#endif

// *****************************************************************************
// Public types and definitions

/**
 * Signature for a listener function.
 *
 * @param buf The user-supplied buffer to receive the message from the broker.
 * @param n_bytes The number of bytes received or 0 if server closed, negative
 *        on error.
 */
typedef void (*listen_cb_t)(const char *buf, ssize_t n_bytes);

// *****************************************************************************
// Public declarations

/**
 * @brief Manage a socket connection to a broker.
 *
 * Note
 * @param broker_addr The ipv4 broker address, e.g. "127.0.0.0"
 * @param broker_port THe broker's port, e.g. 8000
 * @param listen_cb The function to call when a string is received from broker
 * @return true if the connection completes.
 */
bool sock_client_run(const char *broker_addr, uint16_t broker_port,
                     listen_cb_t listen_cb, char *recv_buf, size_t recv_buflen);

/**
 * @brief Send a message to the broker for broadcasting.
 *
 * @param s The string to send.
 * @return true if the send completes.
 */
void sock_client_send(const char *s);

// *****************************************************************************
// End of file

#ifdef __cplusplus
}
#endif

#endif /* #ifndef _SOCK_CLIENT_H_ */
