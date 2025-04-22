// path_utils.h
#ifndef PATH_UTILS_H
#define PATH_UTILS_H

#include <string>

// Functions to get file paths based on architecture
std::string getMacFilePath();
std::string getFlagFilePath();
std::string getSerialNumberFilePath();

#endif // PATH_UTILS_H
