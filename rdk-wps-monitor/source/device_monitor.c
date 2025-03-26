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
 * @file device_monitor.c
 * @brief Implementation of device monitoring functionality
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <syslog.h>
#include <dirent.h>
#include <linux/input.h>
#include <sys/stat.h>
#include "../include/device_monitor.h"
#include "../include/button_callback.h"
#include "../include/utils.h"

// Thread management
static pthread_t device_threads[MAX_INPUT_DEVICES];
static device_thread_data_t thread_data[MAX_INPUT_DEVICES];
static int num_device_threads = 0;
static pthread_mutex_t thread_mutex = PTHREAD_MUTEX_INITIALIZER;

int device_monitor_init(void) {
    // Initialize thread data
    for (int i = 0; i < MAX_INPUT_DEVICES; i++) {
        thread_data[i].device_path = NULL;
        thread_data[i].thread_active = 0;
    }
    
    return 0;
}

void device_monitor_cleanup(void) {
    pthread_mutex_lock(&thread_mutex);
    
    for (int i = 0; i < num_device_threads; i++) {
        if (thread_data[i].thread_active) {
            thread_data[i].thread_active = 0;
            pthread_mutex_unlock(&thread_mutex);
            pthread_join(device_threads[i], NULL);
            pthread_mutex_lock(&thread_mutex);
            
            if (thread_data[i].device_path) {
                free(thread_data[i].device_path);
                thread_data[i].device_path = NULL;
            }
        }
    }
    
    pthread_mutex_unlock(&thread_mutex);
}

int add_input_device(const char *device_path) {
    int i;
    pthread_mutex_lock(&thread_mutex);
    
    // Check if this device is already being monitored
    for (i = 0; i < num_device_threads; i++) {
        if (thread_data[i].thread_active && 
            strcmp(thread_data[i].device_path, device_path) == 0) {
            pthread_mutex_unlock(&thread_mutex);
            return 0; // Already monitoring this device
        }
    }
    
    // Find an available slot
    for (i = 0; i < MAX_INPUT_DEVICES; i++) {
        if (!thread_data[i].thread_active) {
            break;
        }
    }
    
    if (i == MAX_INPUT_DEVICES) {
        log_message(LOG_ERR, "Maximum number of monitored devices reached");
        pthread_mutex_unlock(&thread_mutex);
        return -1;
    }
    
    // Set up the thread data
    thread_data[i].device_path = strdup(device_path);
    thread_data[i].thread_active = 1;
    
    // Create the monitoring thread
    if (pthread_create(&device_threads[i], NULL, device_monitor_thread, &thread_data[i]) != 0) {
        log_message(LOG_ERR, "Failed to create monitoring thread for %s", device_path);
        free(thread_data[i].device_path);
        thread_data[i].device_path = NULL;
        thread_data[i].thread_active = 0;
        pthread_mutex_unlock(&thread_mutex);
        return -1;
    }
    
    if (i >= num_device_threads) {
        num_device_threads = i + 1;
    }
    
    pthread_mutex_unlock(&thread_mutex);
    return 0;
}

void remove_input_device(const char *device_path) {
    pthread_mutex_lock(&thread_mutex);
    
    for (int i = 0; i < num_device_threads; i++) {
        if (thread_data[i].thread_active && 
            strcmp(thread_data[i].device_path, device_path) == 0) {
            thread_data[i].thread_active = 0;
            // The thread will detect this and exit
            log_message(LOG_INFO, "Marked device %s for removal", device_path);
            break;
        }
    }
    
    pthread_mutex_unlock(&thread_mutex);
}

void scan_existing_devices(void) {
    DIR *dir;
    struct dirent *entry;
    char path[512]; /* Increase buffer size to safely handle long device names */
    
    log_message(LOG_INFO, "Scanning existing input devices");
    
    dir = opendir("/dev/input");
    if (!dir) {
        log_message(LOG_ERR, "Failed to open /dev/input directory");
        return;
    }
    
    while ((entry = readdir(dir)) != NULL) {
        if (strncmp(entry->d_name, "event", 5) == 0) {
            /* Use a safer approach to prevent truncation */
            if (snprintf(path, sizeof(path), "/dev/input/%s", entry->d_name) >= (int)sizeof(path)) {
                log_message(LOG_WARNING, "Device name too long: %s", entry->d_name);
                continue;
            }
            log_message(LOG_INFO, "Found input device: %s", path);
            add_input_device(path);
        }
    }
    
    closedir(dir);
}

void *device_monitor_thread(void *arg) {
    device_thread_data_t *data = (device_thread_data_t *)arg;
    char *device_path = data->device_path;
    int fd;
    struct input_event ev;
    ssize_t n;
    
    log_message(LOG_INFO, "Started monitoring thread for device: %s", device_path);
    
    // Open the input device
    fd = open(device_path, O_RDONLY | O_NONBLOCK);
    if (fd < 0) {
        log_message(LOG_ERR, "Error opening device %s: %s", device_path, strerror(errno));
        data->thread_active = 0;
        return NULL;
    }
    
    // Main monitoring loop
    while (get_running_state() && data->thread_active) {
        n = read(fd, &ev, sizeof(ev));
        
        if (n == sizeof(ev)) {
            // Check if it's a key/button event
            if (ev.type == EV_KEY) {
                button_callback callback = get_button_callback();
                if (callback) {
                    callback(device_path, ev.code, ev.value);
                }
            }
        } else if (n < 0) {
            if (errno != EAGAIN) {
                log_message(LOG_ERR, "Error reading from device %s: %s", device_path, strerror(errno));
                break;
            }
            // No data available right now, sleep a bit
            usleep(10000); // 10ms
        }
    }
    
    close(fd);
    log_message(LOG_INFO, "Stopped monitoring thread for device: %s", device_path);
    
    data->thread_active = 0;
    return NULL;
}

void print_monitored_devices(void) {
    int active_count = 0;
    
    pthread_mutex_lock(&thread_mutex);
    
    log_message(LOG_INFO, "Currently monitoring the following input devices:");
    for (int i = 0; i < num_device_threads; i++) {
        if (thread_data[i].thread_active) {
            log_message(LOG_INFO, "  - %s", thread_data[i].device_path);
            active_count++;
        }
    }
    
    if (active_count == 0) {
        log_message(LOG_INFO, "  No active device monitors");
    }
    
    pthread_mutex_unlock(&thread_mutex);
}
