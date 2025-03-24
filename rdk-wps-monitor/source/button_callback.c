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
 * @file button_callback.c
 * @brief Implementation of button callback functionality
 */

#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include "../include/button_callback.h"
#include "../include/utils.h"

#include <rbus.h>
#include <pthread.h>
#include <unistd.h>

pthread_mutex_t wps_mutex = PTHREAD_MUTEX_INITIALIZER;

#define WPS_DELAY 60 // 60 seconds delay between presses

void set_wps_push_button(rbusHandle_t handle, const char* param) {
    pthread_mutex_lock(&wps_mutex);

    rbusError_t err = rbus_setBoolean(handle, param, true);
    if (err != RBUS_ERROR_SUCCESS) {
        printf("Failed to set %s: %d\n", param, err);
    } else {
        printf("Successfully set %s.\n", param);
    }

    pthread_mutex_unlock(&wps_mutex);
}

// The active callback function
static button_callback event_callback = default_button_callback;

void default_button_callback(const char *device, int button_code, int value) {
    const char *action = (value == 1) ? "PRESSED" : (value == 0) ? "RELEASED" : "REPEATED";
    
    log_message(LOG_INFO, "Device: %s, Button %d %s", 
           device, button_code, action);

    rbusHandle_t handle;
    rbusError_t err;
    char button_name[32];
    snprintf(button_name, sizeof(button_name), "wps_button_%d", button_code);

    // Initialize RBus
    //err = rbus_open(&handle, "wps_push_button");
    err = rbus_open(&handle, button_name);
    if (err != RBUS_ERROR_SUCCESS) {
        printf("Failed to initialize RBus: %d\n", err);
    }

    // Set WPS Push Button for Access Point 1
    log_message(LOG_INFO, "WPS button pressed on device %s - triggering WPS action for 2G", device);
    set_wps_push_button(handle, "Device.WiFi.AccessPoint.1.WPS.X_CISCO_COM_ActivatePushButton");

    // Set WPS Push Button for Access Point 2
    log_message(LOG_INFO, "WPS button pressed on device %s - triggering WPS action for 5G", device);
    set_wps_push_button(handle, "Device.WiFi.AccessPoint.2.WPS.X_CISCO_COM_ActivatePushButton");
    sleep(WPS_DELAY);

    // Close RBus
    rbus_close(handle);
}

void custom_wps_button_callback(const char *device, int button_code, int value) {
    // Check if the button code matches what we're looking for
    // Button codes: KEY_WPS_BUTTON (0x211) or BTN_0 (0x100)
    if (button_code == 0x211 || button_code == 0x100) {
        if (value == 1) { // Button pressed
            log_message(LOG_INFO, "WPS button pressed on device %s - triggering WPS action", device);
            
            // Here you would add code to trigger the actual WPS functionality
            system("echo 'WPS button pressed' >> /tmp/wps_events.log");
        }
    } else {
        // For other buttons, use the default behavior
        default_button_callback(device, button_code, value);
    }
}

void register_button_callback(button_callback callback) {
    if (callback != NULL) {
        event_callback = callback;
    } else {
        event_callback = default_button_callback;
    }
}

button_callback get_button_callback(void) {
    return event_callback;
}
