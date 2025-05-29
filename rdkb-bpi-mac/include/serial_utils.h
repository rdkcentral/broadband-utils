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
// serial_utils.h
#ifndef SERIAL_UTILS_H
#define SERIAL_UTILS_H

#include <string>
#include <vector>
#include <cstdint>

// Serial number related functions
std::string readSerialNumber(const std::string& path = "");
std::vector<uint8_t> extractBytesFromSerial(const std::string& serialNumber, int bytesNeeded = 3);

#endif // SERIAL_UTILS_H
