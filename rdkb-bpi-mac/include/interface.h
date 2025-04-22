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
