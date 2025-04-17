/*
 * If not stated otherwise in this file or this component's LICENSE file the
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
 * @file utils.c
 * @brief Implementation of utility functions
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <syslog.h>
#include <sys/stat.h>
#include "../include/utils.h"

// Global variables
static volatile bool running = true;
static bool debug_mode = false;

void log_init(bool daemon_mode, bool debug_mode) {
    /* Prevent unused parameter warning */
    (void)debug_mode;
    int log_options = LOG_PID;
    
    // In foreground mode, also log to stderr
    if (!daemon_mode) {
        log_options |= LOG_PERROR;
    }
    
    openlog("netlink-button-monitor", log_options, LOG_DAEMON);
}

void log_message(int level, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    
    // Always log to syslog
    vsyslog(level, fmt, args);
    
    // If in debug mode, also log to stdout
    if (get_debug_mode()) {
        va_list args_copy;
        va_copy(args_copy, args);
        vprintf(fmt, args_copy);
        printf("\n");
        va_end(args_copy);
    }
    
    va_end(args);
}

int daemonize(void) {
    pid_t pid, sid;
    
    // Fork off the parent process
    pid = fork();
    if (pid < 0) {
        return -1;
    }
    
    // If we got a good PID, then we can exit the parent process
    if (pid > 0) {
        exit(EXIT_SUCCESS);
    }
    
    // Change the file mode mask
    umask(0);
    
    // Open logs
    openlog("netlink-button-monitor", LOG_PID, LOG_DAEMON);
    
    // Create a new SID for the child process
    sid = setsid();
    if (sid < 0) {
        syslog(LOG_ERR, "Failed to create new session, exiting.");
        return -1;
    }
    
    // Change the current working directory
    if ((chdir("/")) < 0) {
        syslog(LOG_ERR, "Failed to change directory, exiting.");
        return -1;
    }
    
    // Close standard file descriptors
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
    
    return 0;
}

int write_pid_file(const char *pid_file) {
    FILE *f = fopen(pid_file, "w");
    if (!f) {
        log_message(LOG_WARNING, "Could not create PID file %s: %s", 
                   pid_file, strerror(errno));
        return -1;
    }
    
    fprintf(f, "%d\n", getpid());
    fclose(f);
    return 0;
}

void remove_pid_file(const char *pid_file) {
    unlink(pid_file);
}

void set_running_state(bool state) {
    running = state;
}

bool get_running_state(void) {
    return running;
}

void set_debug_mode(bool debug) {
    debug_mode = debug;
}

bool get_debug_mode(void) {
    return debug_mode;
}
