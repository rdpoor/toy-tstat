/**
 * @file sock_client.c
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
 *
 */

// *****************************************************************************
// Includes

#include "sock_client.h"

#include <arpa/inet.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

// *****************************************************************************
// Private types and definitions

typedef struct {
    int sock;
    listen_cb_t listen_cb;
    char *recv_buf;
    size_t recv_buflen;
} sock_client_t;

// *****************************************************************************
// Private (static) storage

static sock_client_t s_sock_client;

// *****************************************************************************
// Private (static, forward) declarations

static void *receive_messages(void *arg);

// *****************************************************************************
// Public code

bool sock_client_run(const char *broker_addr, uint16_t broker_port,
                     listen_cb_t listen_cb, char *recv_buf, size_t recv_buflen) {
    struct sockaddr_in server_addr;
    pthread_t thread_id1, thread_id2;

    s_sock_client.listen_cb = listen_cb;
    s_sock_client.recv_buf = recv_buf;
    s_sock_client.recv_buflen = recv_buflen;

    // Create socket
    s_sock_client.s_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (s_sock_client.s_sock < 0) {
        printf("Error while creating socket\n");
        return false;
    }

    // Set port and IP:
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(broker_port);
    server_addr.sin_addr.s_addr = inet_addr(broker_addr);

    // Connect to the server:
    if (connect(s_sock_client.s_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        printf("Error while connecting to server\n");
        return false;
    }

    if(pthread_create(&thread_id1, NULL, receive_messages, &s_sock_client) < 0) {
        printf("Error creating thread\n");
        return false;
    }

    // Close the socket if thread has quit
    pthread_join(thread_id1, NULL);
    close(s_sock);

    return true;

}

bool sock_client_send(const char *s) {
    if (send(s_sock_client.sock, s, strlen(s), 0) < 0) {
        printf("Error in sending data.\n");
    }
}

// *****************************************************************************
// Private (static) code

static void *receive_messages(void *arg) {
    sock_client_t *client = (sock_client_t *)arg;
    char server_message[BUFFER_SIZE];
    size_t n_read = 0;

    do {
        n_read = recv(client->sock, client->recv_buf, client->recv_buflen, 0);
        client->listen_cb(client->recv_buf, n_read);
        if (n_read < 0) {
            printf("Error in receiving data, exiting.\n");
        } else if (n_read == 0) {
            printf("Server closed socket, exiting.\n");
        }
    } while (n_read > 0);

    return NULL;
}

// *****************************************************************************
// End of file
