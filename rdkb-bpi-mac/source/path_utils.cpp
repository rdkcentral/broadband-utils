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

// path_utils.cpp
#include "path_utils.h"

// Function to get the MAC addresses file path based on architecture
std::string getMacFilePath() {
#ifdef AARCH64_BUILD
    return "/nvram/mac_addresses.txt";
#else
    // For testing on x86, use current directory
    return "./mac_addresses.txt";
#endif
}

// Function to get the flag file path based on architecture
std::string getFlagFilePath() {
#ifdef AARCH64_BUILD
    return "/nvram/mac_assigned";
#else
    return "./mac_assigned";
#endif
}

// Function to get the serial number file path based on architecture
std::string getSerialNumberFilePath() {
#ifdef AARCH64_BUILD
    return "/nvram/serial_number.txt";
#else
    return "/home/ubuntu/rdk-bpi-mac/serial_number.txt";
#endif
}
