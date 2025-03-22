/**
 * MT7988 Serial Number Tool - Serial Number Operations
 *
 * Definitions for serial number processing and storage.
 */

#ifndef SERIAL_NUMBER_H
#define SERIAL_NUMBER_H

#include <stdint.h>
#include <stdbool.h>

/**
 * Save serial number to NVRAM file.
 *
 * @param values Array of register values to use as serial number
 * @param count Number of values in the array
 * @param filename Path to file where serial number should be saved
 * @return true if successful, false otherwise
 */
bool save_serial_number(uint32_t *values, int count, const char *filename);

#endif /* SERIAL_NUMBER_H */
