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
