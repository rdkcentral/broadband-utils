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
// interface.h
#ifndef INTERFACE_H
#define INTERFACE_H

#include <string>
#include <cstdint>

// Structure to hold interface details
struct Interface {
    std::string name;
    uint8_t type; // 01 for LAN, 02 for WiFi
    uint8_t index;
};

#endif // INTERFACE_H
