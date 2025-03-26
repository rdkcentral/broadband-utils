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
 * @file main.c
 * @brief Main program entry for the netlink button monitor daemon
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <syslog.h>
#include <pthread.h>

#include "../include/utils.h"
#include "../include/button_callback.h"
#include "../include/device_monitor.h"
#include "../include/netlink_monitor.h"

#define PID_FILE "/var/run/netlink-button-monitor.pid"

// Signal handler
static void sigterm_handler(int sig) {
    log_message(LOG_NOTICE, "Caught signal %d, cleaning up and exiting...", sig);
    set_running_state(false);
}

// Show usage
static void show_usage(const char *prog_name) {
    printf("Usage: %s [options]\n", prog_name);
    printf("Options:\n");
    printf("  -f, --foreground     Run in foreground (not as daemon)\n");
    printf("  -c, --custom-callback Use custom WPS button callback\n");
    printf("  -d, --debug          Enable debug output\n");
    printf("  -h, --help           Show this help message\n");
}

// Main function
int main(int argc, char *argv[]) {
    pthread_t netlink_thread;
    bool daemon_mode = true; // Run as daemon by default
    bool use_custom_callback = false;
    
    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-f") == 0 || strcmp(argv[i], "--foreground") == 0) {
            daemon_mode = false;
        } else if (strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--custom-callback") == 0) {
            use_custom_callback = true;
        } else if (strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--debug") == 0) {
            set_debug_mode(true);
        } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            show_usage(argv[0]);
            return 0;
        }
    }
    
    // Set up signal handlers
    signal(SIGTERM, sigterm_handler);
    signal(SIGINT, sigterm_handler);
    
    // Run as daemon if requested
    if (daemon_mode && !get_debug_mode()) {
        if (daemonize() < 0) {
            fprintf(stderr, "Failed to daemonize process\n");
            return 1;
        }
    } else {
        // For foreground mode, initialize logging
        log_init(false, get_debug_mode());
    }
    
    log_message(LOG_NOTICE, "Netlink button monitor daemon starting up");
    
    // Initialize device monitoring subsystem
    if (device_monitor_init() < 0) {
        log_message(LOG_ERR, "Failed to initialize device monitoring, exiting");
        return 1;
    }
    
    // Initialize netlink socket
    if (init_netlink_socket() < 0) {
        log_message(LOG_ERR, "Failed to initialize netlink socket, exiting");
        device_monitor_cleanup();
        return 1;
    }
    
    // Scan for existing input devices and start monitoring
    scan_existing_devices();
    
    // Register custom callback if requested
    if (use_custom_callback) {
        log_message(LOG_INFO, "Using custom WPS button callback");
        register_button_callback(custom_wps_button_callback);
    }
    
    // Start the netlink monitoring thread
    if (start_netlink_monitor(&netlink_thread) < 0) {
        log_message(LOG_ERR, "Failed to start netlink monitoring thread, exiting");
        close_netlink_socket();
        device_monitor_cleanup();
        return 1;
    }
    
    // Write PID file for daemon management
    if (daemon_mode) {
        if (write_pid_file(PID_FILE) < 0) {
            log_message(LOG_WARNING, "Could not create PID file");
        }
    }
    
    log_message(LOG_NOTICE, "Netlink button monitor daemon started successfully");
    
    // In debug mode, print a summary of monitored devices
    if (get_debug_mode()) {
        print_monitored_devices();
    }
    
    // Main thread - just wait for signals or thread completion
    pthread_join(netlink_thread, NULL);
    
    // Cleanup
    log_message(LOG_NOTICE, "Netlink button monitor daemon shutting down");
    
    // Close the netlink socket
    close_netlink_socket();
    
    // Clean up device monitoring
    device_monitor_cleanup();
    
    // Remove PID file
    if (daemon_mode) {
        remove_pid_file(PID_FILE);
    }
    
    return 0;
}
