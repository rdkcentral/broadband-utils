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
// serial_utils.cpp
#include "serial_utils.h"
#include "path_utils.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>

// Function to read serial number from file
std::string readSerialNumber(const std::string& path) {
    // Determine path based on provided parameter or default
    std::string filePath = path.empty() ? getSerialNumberFilePath() : path;
    
    std::ifstream file(filePath);
    if (!file) {
        std::cerr << "Error: Cannot open serial number file: " << filePath << std::endl;
        return "";
    }
    
    std::string serialNumber;
    std::getline(file, serialNumber);
    file.close();
    return serialNumber;
}

// Function to extract bytes from serial number
std::vector<uint8_t> extractBytesFromSerial(const std::string& serialNumber, int bytesNeeded) {
    std::vector<uint8_t> bytes;
    
    // Use the last bytesNeeded*2 characters (each byte is 2 hex characters)
    int startPos = std::max(0, static_cast<int>(serialNumber.length()) - bytesNeeded * 2);
    
    for (size_t i = startPos; i < serialNumber.length(); i += 2) {
        if (i + 1 < serialNumber.length()) {
            std::string byteStr = serialNumber.substr(i, 2);
            uint8_t byte = 0;
            unsigned int temp = 0;
            std::istringstream(byteStr) >> std::hex >> temp;
            byte = static_cast<uint8_t>(temp);
            bytes.push_back(byte);
        }
    }
    
    // If we didn't get enough bytes, pad with zeros
    while (bytes.size() < static_cast<size_t>(bytesNeeded)) {
        bytes.insert(bytes.begin(), 0);
    }
    
    // If we got too many bytes, keep only the last bytesNeeded
    if (bytes.size() > static_cast<size_t>(bytesNeeded)) {
        bytes.erase(bytes.begin(), bytes.end() - bytesNeeded);
    }
    
    return bytes;
}
