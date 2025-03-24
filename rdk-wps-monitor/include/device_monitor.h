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
 * @file device_monitor.h
 * @brief Input device monitoring functionality
 */

#ifndef DEVICE_MONITOR_H
#define DEVICE_MONITOR_H

#include <pthread.h>

/**
 * @brief Maximum number of input devices to monitor
 */
#define MAX_INPUT_DEVICES 32

/**
 * @brief Thread data structure for device monitors
 */
typedef struct {
    char *device_path;
    int thread_active;
} device_thread_data_t;

/**
 * @brief Initialize device monitoring subsystem
 * 
 * @return 0 on success, -1 on failure
 */
int device_monitor_init(void);

/**
 * @brief Clean up device monitoring subsystem
 */
void device_monitor_cleanup(void);

/**
 * @brief Add a new input device to monitor
 * 
 * @param device_path Path to the input device
 * @return 0 on success, -1 on failure
 */
int add_input_device(const char *device_path);

/**
 * @brief Remove a device from monitoring
 * 
 * @param device_path Path to the input device
 */
void remove_input_device(const char *device_path);

/**
 * @brief Scan and add existing input devices
 */
void scan_existing_devices(void);

/**
 * @brief Thread function to monitor a specific input device
 * 
 * @param arg Thread argument (device_thread_data_t *)
 * @return NULL
 */
void *device_monitor_thread(void *arg);

/**
 * @brief Print a summary of monitored devices
 */
void print_monitored_devices(void);

#endif /* DEVICE_MONITOR_H */
