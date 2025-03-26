/*
 * If not stated otherwise in this file or this component's Licenses.txt file the
 * following copyright and licenses apply:
 *
 * Copyright 2025 RDK Management
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
*/

/**
 * @file netlink_monitor.c
 * @brief Implementation of netlink monitoring functionality
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <syslog.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include "../include/netlink_monitor.h"
#include "../include/device_monitor.h"
#include "../include/utils.h"

// Define UDEV netlink constants if not defined in headers
#ifndef NETLINK_KOBJECT_UEVENT
#define NETLINK_KOBJECT_UEVENT 15
#endif

// Global netlink socket
static int nl_socket = -1;

int init_netlink_socket(void) {
    struct sockaddr_nl nl_addr;
    int ret;
    int buf_size = 1024 * 1024; // 1MB buffer for netlink
    
    // Create the netlink socket
    nl_socket = socket(AF_NETLINK, SOCK_RAW, NETLINK_KOBJECT_UEVENT);
    if (nl_socket < 0) {
        log_message(LOG_ERR, "Failed to create netlink socket: %s", strerror(errno));
        return -1;
    }
    
    // Increase the buffer size to avoid missing events
    setsockopt(nl_socket, SOL_SOCKET, SO_RCVBUFFORCE, &buf_size, sizeof(buf_size));
    
    // Set socket to non-blocking
    int flags = fcntl(nl_socket, F_GETFL, 0);
    if (flags < 0) {
        log_message(LOG_ERR, "Failed to get socket flags: %s", strerror(errno));
        close(nl_socket);
        nl_socket = -1;
        return -1;
    }
    
    ret = fcntl(nl_socket, F_SETFL, flags | O_NONBLOCK);
    if (ret < 0) {
        log_message(LOG_ERR, "Failed to set socket non-blocking: %s", strerror(errno));
        close(nl_socket);
        nl_socket = -1;
        return -1;
    }
    
    // Set up socket address
    memset(&nl_addr, 0, sizeof(nl_addr));
    nl_addr.nl_family = AF_NETLINK;
    nl_addr.nl_pid = getpid();
    nl_addr.nl_groups = 1; // UDEV events group
    
    // Bind to the socket
    ret = bind(nl_socket, (struct sockaddr *)&nl_addr, sizeof(nl_addr));
    if (ret < 0) {
        log_message(LOG_ERR, "Failed to bind netlink socket: %s", strerror(errno));
        close(nl_socket);
        nl_socket = -1;
        return -1;
    }
    
    log_message(LOG_INFO, "Netlink socket initialized successfully");
    return 0;
}

void close_netlink_socket(void) {
    if (nl_socket >= 0) {
        close(nl_socket);
        nl_socket = -1;
    }
}

int start_netlink_monitor(pthread_t *thread) {
    // Create the netlink monitoring thread
    if (pthread_create(thread, NULL, netlink_monitor_thread, NULL) != 0) {
        log_message(LOG_ERR, "Failed to create netlink monitoring thread: %s", strerror(errno));
        return -1;
    }
    
    return 0;
}

void parse_netlink_message(const char *buffer, int len) {
    // For debugging: log the entire message
    if (get_debug_mode()) {
        log_message(LOG_DEBUG, "Received netlink message (%d bytes):", len);
        for (int i = 0; i < len && buffer[i]; i++) {
            putchar(buffer[i]);
        }
        putchar('\n');
    }
    
    // Variables to store extracted information
    char device_path[256] = {0};
    char action[32] = {0};
    int is_input_device = 0;
    
    // Parse the buffer line by line
    const char *ptr = buffer;
    const char *end = buffer + len;
    
    while (ptr < end && *ptr) {
        if (strncmp(ptr, "ACTION=", 7) == 0) {
            strncpy(action, ptr + 7, sizeof(action) - 1);
        }
        else if (strncmp(ptr, "DEVNAME=", 8) == 0) {
            if (strstr(ptr + 8, "input/event") != NULL) {
                is_input_device = 1;
                snprintf(device_path, sizeof(device_path), "/dev/%s", ptr + 8);
            }
        }
        
        // Move to next line
        while (ptr < end && *ptr) ptr++;
        ptr++; // Skip the null terminator
    }
    
    // If we found an input device with an action
    if (is_input_device && action[0]) {
        log_message(LOG_INFO, "Input device event: %s %s", action, device_path);
        
        if (strcmp(action, "add") == 0) {
            add_input_device(device_path);
        }
        else if (strcmp(action, "remove") == 0) {
            remove_input_device(device_path);
        }
    }
}

void *netlink_monitor_thread(void *arg) {
    /* Prevent unused parameter warning */
    (void)arg;
    int ret;
    char buffer[8192];
    struct sockaddr_nl nladdr;
    struct msghdr msg;
    struct iovec iov;
    
    log_message(LOG_INFO, "Netlink event monitoring thread started");
    
    // Set up message structures
    memset(&nladdr, 0, sizeof(nladdr));
    nladdr.nl_family = AF_NETLINK;
    nladdr.nl_pid = 0;  // Kernel
    nladdr.nl_groups = 0;
    
    iov.iov_base = buffer;
    iov.iov_len = sizeof(buffer);
    
    memset(&msg, 0, sizeof(msg));
    msg.msg_name = &nladdr;
    msg.msg_namelen = sizeof(nladdr);
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    
    // Main event loop
    while (get_running_state()) {
        ret = recvmsg(nl_socket, &msg, 0);
        
        if (ret < 0) {
            if (errno == EINTR || errno == EAGAIN) {
                // Interrupted or would block - normal conditions
                usleep(10000); // 10ms
                continue;
            }
            
            log_message(LOG_ERR, "Error receiving netlink message: %s", strerror(errno));
            break;
        }
        
        if (ret > 0) {
            // Process the netlink message
            buffer[ret] = '\0'; // Ensure null termination
            parse_netlink_message(buffer, ret);
        }
    }
    
    log_message(LOG_INFO, "Netlink event monitoring thread terminated");
    return NULL;
}
