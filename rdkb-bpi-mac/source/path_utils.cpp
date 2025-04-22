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
