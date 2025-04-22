// main.cpp
#include "interface.h"
#include "mac_generator.h"
#include "serial_utils.h"
#include <iostream>
#include <vector>

int main() {
    // Check if MAC addresses have already been assigned
    //if (checkIfMacAssigned()) {
      //  std::cout << "MAC addresses have already been assigned. Exiting." << std::endl;
        //return 0;
    //}
    
    // Define interfaces
    std::vector<Interface> interfaces = {
        {"lan0", 0x01, 0x00},
        {"lan1", 0x01, 0x01},
        {"lan2", 0x01, 0x02},
        {"lan3", 0x01, 0x03},
        {"wifi0", 0x02, 0x00},
        {"wifi1", 0x02, 0x01},
        {"wifi2", 0x02, 0x02}
    };
    
    // Read serial number
    std::string serialNumber = readSerialNumber();
    if (serialNumber.empty()) {
        std::cerr << "Error: Could not read serial number" << std::endl;
        return 1;
    }
    
    std::cout << "Read serial number: " << serialNumber << std::endl;
    
    // Extract bytes from serial number
    std::vector<uint8_t> serialBytes = extractBytesFromSerial(serialNumber);
    if (serialBytes.size() < 3) {
        std::cerr << "Error: Not enough bytes in serial number" << std::endl;
        return 1;
    }
    
    // Generate MAC addresses for each interface
    std::vector<std::string> macAddresses;
       
    for (size_t i = 0; i < interfaces.size(); i++) {
         std::string macAddress = generateMacAddress(interfaces[i], serialBytes, static_cast<int>(i));
         std::cout << "Generated MAC address for " << interfaces[i].name << ": " << macAddress << std::endl;
        // Store MAC address for later writing to file
        macAddresses.push_back(macAddress);
    }
    
    // Write all MAC addresses to a single file
    writeAllMacAddresses(interfaces, macAddresses);
    
    // Mark as assigned
    markMacAssigned();
    std::cout << "MAC addresses written to file successfully" << std::endl;
    
    return 0;
}
