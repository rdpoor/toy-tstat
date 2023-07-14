/**
 * @file ui_serial.c
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

#include "ui_serial.h"

#include "relay_id.h"
#include "ui.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// *****************************************************************************
// Private types and definitions

#define RECV_BUF_LEN 300

// *****************************************************************************
// Private (static, forward) declarations

static void display_ambient(int ambient);
static void display_setpoint(int setpoint);
static void display_relay(relay_id_t relay, bool energized);
static void button_was_pressed(bool up);

// *****************************************************************************
// Private (static) storage

static const ui_api_t s_ui_api = {
    .display_ambient = display_ambient,
    .display_setpoint = display_setpoint,
    .display_relay = display_relay,
    .button_was_pressed = button_was_pressed
};

static const ui_t s_ui = {.api = s_ui_api, .ctx = NULL};

static s_recv_buf[RECV_BUF_LEN];

// *****************************************************************************
// Public code

int main(int argc, char *argv[]) {
    if (argc != 3) {
        usage();
    }
    const char *addr = argv[1];
    uint16_t port = atoi(argv[2]);
    printf("\nConnecting to broker at %s:%d", addr, port);

    ui_init(&s_ui);

    if (!sock_client_init(addr, port, listen_cb, s_recv_buf, sizeof(s_recv_buf))) {
        printf("\nError: coud not connect to the broker");
        exit();
    }
}

// *****************************************************************************
// Private (static) code

static void display_ambient(int ambient) {
    printf("\n# ambient set to %d", ambient);
}

static void display_setpoint(int setpoint) {
    printf("\n# setpoint set to %d", setpoint);
}

static void display_relay(relay_id_t relay, bool energized) {
    printf("\n# %d %s", relay, energized ? "energized" : "de-energized");
}

static void button_was_pressed(bool up) {
    printf("{\"topic\":\"ctrl\", \"fn\":\"button_pressed\", \"args\":{%s}}\n", up:"true":"false");
    // TBD
}


// *****************************************************************************
// End of file


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024

int socket_desc;

void *receive_messages(void *socket_desc) {
    char server_message[BUFFER_SIZE];
    int new_socket = *(int*)socket_desc;

    while(1) {
        if (recv(new_socket, server_message, sizeof(server_message), 0) < 0) {
            printf("Error in receiving data.\n");
        } else {
            printf("%s", server_message);
            fflush(stdout); // Flush stdout buffer
        }
    }

    return NULL;
}

void *send_messages(void *socket_desc) {
    char client_message[BUFFER_SIZE];
    int new_socket = *(int*)socket_desc;

    while(1) {
        if (fgets(client_message, sizeof(client_message), stdin) != NULL) {
            if (send(new_socket, client_message, strlen(client_message), 0) < 0) {
                printf("Error in sending data.\n");
            }
        }
    }

    return NULL;
}

int main(int argc, char *argv[]) {
    struct sockaddr_in server_addr;

    // Create socket
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_desc < 0) {
        printf("Error while creating socket\n");
        return -1;
    }

    // Set port and IP:
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(2000); // for example
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); // for example

    // Connect to the server:
    if (connect(socket_desc, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        printf("Error while connecting to server\n");
        return -1;
    }

    pthread_t thread_id1, thread_id2;
    if(pthread_create(&thread_id1, NULL, receive_messages, (void *)&socket_desc) < 0) {
        printf("Error creating thread\n");
        return -1;
    }

    if(pthread_create(&thread_id2, NULL, send_messages, (void *)&socket_desc) < 0) {
        printf("Error creating thread\n");
        return -1;
    }

    pthread_join(thread_id1, NULL);
    pthread_join(thread_id2, NULL);

    // Close the socket at the end
    close(socket_desc);

    return 0;
}
