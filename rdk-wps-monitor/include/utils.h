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
 * @file utils.h
 * @brief Utility functions for the netlink button monitor daemon
 */

#ifndef UTILS_H
#define UTILS_H

#include <stdbool.h>
#include <syslog.h>

/**
 * @brief Initialize logging
 * 
 * @param daemon_mode Whether running in daemon mode
 * @param debug_mode Whether running in debug mode
 */
void log_init(bool daemon_mode, bool debug_mode);

/**
 * @brief Log a message to syslog and stdout if in debug mode
 * 
 * @param level The log level (LOG_INFO, LOG_WARNING, etc.)
 * @param fmt Format string followed by arguments
 */
void log_message(int level, const char *fmt, ...);

/**
 * @brief Daemonize the process
 * 
 * @return 0 on success, -1 on failure
 */
int daemonize(void);

/**
 * @brief Write PID to file
 * 
 * @param pid_file Path to PID file
 * @return 0 on success, -1 on failure
 */
int write_pid_file(const char *pid_file);

/**
 * @brief Remove PID file
 * 
 * @param pid_file Path to PID file
 */
void remove_pid_file(const char *pid_file);

/**
 * @brief Set the program's running state
 * 
 * @param state New state (true = running, false = stop)
 */
void set_running_state(bool state);

/**
 * @brief Get the program's running state
 * 
 * @return Current running state
 */
bool get_running_state(void);

/**
 * @brief Set debug mode
 * 
 * @param debug Whether debug mode is enabled
 */
void set_debug_mode(bool debug);

/**
 * @brief Get debug mode state
 * 
 * @return Current debug mode state
 */
bool get_debug_mode(void);

#endif /* UTILS_H */
