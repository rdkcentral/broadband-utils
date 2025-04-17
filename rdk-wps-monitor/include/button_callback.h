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
 * @file button_callback.h
 * @brief Button event callback handling for netlink button monitor
 */

#ifndef BUTTON_CALLBACK_H
#define BUTTON_CALLBACK_H

/**
 * @brief Callback function type definition for button events
 */
typedef void (*button_callback)(const char *device, int button_code, int value);

/**
 * @brief Default button callback implementation
 * 
 * @param device Path to the input device
 * @param button_code Button code that triggered the event
 * @param value Button value (1=pressed, 0=released, 2=repeated)
 */
void default_button_callback(const char *device, int button_code, int value);

/**
 * @brief Custom WPS button callback implementation
 * 
 * @param device Path to the input device
 * @param button_code Button code that triggered the event
 * @param value Button value (1=pressed, 0=released, 2=repeated)
 */
void custom_wps_button_callback(const char *device, int button_code, int value);

/**
 * @brief Register a callback function for button events
 * 
 * @param callback The callback function to register
 */
void register_button_callback(button_callback callback);

/**
 * @brief Get the currently registered callback function
 * 
 * @return The currently registered callback function
 */
button_callback get_button_callback(void);

#endif /* BUTTON_CALLBACK_H */
