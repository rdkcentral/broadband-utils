/*
 * If not stated otherwise in this file or this component's Licenses.txt file the
 * following copyright and licenses apply:
 *
 * Copyright 2015 RDK Management
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

/**********************************************************************
   Copyright [2014] [Cisco Systems, Inc.]
 
   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at
 
       http://www.apache.org/licenses/LICENSE-2.0
 
   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
**********************************************************************/

/**
 * @file netlink_monitor.h
 * @brief Netlink monitoring functionality
 */

#ifndef NETLINK_MONITOR_H
#define NETLINK_MONITOR_H

#include <pthread.h>

/**
 * @brief Initialize the netlink socket
 * 
 * @return 0 on success, -1 on failure
 */
int init_netlink_socket(void);

/**
 * @brief Close the netlink socket
 */
void close_netlink_socket(void);

/**
 * @brief Start the netlink monitoring thread
 * 
 * @param thread Pointer to pthread_t to store thread ID
 * @return 0 on success, -1 on failure
 */
int start_netlink_monitor(pthread_t *thread);

/**
 * @brief Event monitoring thread for netlink
 * 
 * @param arg Thread argument (unused)
 * @return NULL
 */
void *netlink_monitor_thread(void *arg);

/**
 * @brief Parse a netlink message for input devices
 * 
 * @param buffer Message buffer
 * @param len Length of the message
 */
void parse_netlink_message(const char *buffer, int len);

#endif /* NETLINK_MONITOR_H */
