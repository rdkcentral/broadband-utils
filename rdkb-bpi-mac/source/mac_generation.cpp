// mac_generator.cpp
#include "mac_generator.h"
#include "path_utils.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <vector>

std::string generateMacAddress(const Interface& interface, const std::vector<uint8_t>& serialBytes, int increment) {
    std::ostringstream mac;
    
    // First byte: 02 (unicast)
    mac << std::hex << std::setfill('0') << std::setw(2) << 2;
    
    // Second byte: interface type (01 for LAN, 02 for WiFi)
    mac << ":" << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(interface.type);
    
    // Third byte: interface index
    mac << ":" << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(interface.index);
    
    // First two bytes from serial number
    for (size_t i = 0; i < serialBytes.size() - 1; i++) {
        mac << ":" << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(serialBytes[i]);
    }
    
    // Last byte from serial number with increment
    if (!serialBytes.empty()) {
        uint8_t lastByte = serialBytes.back() + static_cast<uint8_t>(increment);
        mac << ":" << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(lastByte);
    }
    
    return mac.str();
}
// Function to check if MAC addresses have already been assigned
bool checkIfMacAssigned(const std::string& flagFile) {
    std::string actualFlagFile = flagFile.empty() ? getFlagFilePath() : flagFile;
    
    std::ifstream file(actualFlagFile);
    return file.good();
}

// Function to mark MAC addresses as assigned
void markMacAssigned(const std::string& flagFile) {
    std::string actualFlagFile = flagFile.empty() ? getFlagFilePath() : flagFile;
    
    std::ofstream file(actualFlagFile);
    if (file) {
        file << "MAC addresses assigned on: ";
        // Add timestamp
        char buffer[128];
        time_t now = time(nullptr);
        struct tm* timeinfo = localtime(&now);
        strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);
        file << buffer << std::endl;
        file.close();
    }
}

// Function to write all MAC addresses to a single file
void writeAllMacAddresses(const std::vector<Interface>& interfaces, const std::vector<std::string>& macAddresses) {
    std::string filePath = getMacFilePath();
    
    std::ofstream file(filePath);
    if (file) {
        // Add header with timestamp
        file << "# MAC addresses generated on: ";
        char buffer[128];
        time_t now = time(nullptr);
        struct tm* timeinfo = localtime(&now);
        strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);
        file << buffer << std::endl;
        
        // Write each interface and its MAC address
        for (size_t i = 0; i < interfaces.size(); i++) {
            if (i < macAddresses.size()) {
                file << interfaces[i].name << " " << macAddresses[i] << std::endl;
            }
        }
        
        file.close();
        std::cout << "Wrote all MAC addresses to " << filePath << std::endl;
    } else {
        std::cerr << "Error: Could not write MAC addresses to " << filePath << std::endl;
    }
}
