// mac_generator.h
#ifndef MAC_GENERATOR_H
#define MAC_GENERATOR_H

#include "interface.h"
#include <string>
#include <vector>
#include <cstdint>

// MAC address generation functions
std::string generateMacAddress(const Interface& interface, const std::vector<uint8_t>& serialBytes, int increment = 0);
bool checkIfMacAssigned(const std::string& flagFile = "");
void markMacAssigned(const std::string& flagFile = "");
void writeAllMacAddresses(const std::vector<Interface>& interfaces, const std::vector<std::string>& macAddresses);

#endif // MAC_GENERATOR_H
