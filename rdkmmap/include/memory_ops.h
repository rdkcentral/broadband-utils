/**
 * MT7988 Serial Number Tool - Memory Operations
 *
 * Definitions for memory-mapped register operations.
 */

#ifndef MEMORY_OPS_H
#define MEMORY_OPS_H

#include <stdint.h>
#include <stdbool.h>

/**
 * Read register values from memory-mapped registers.
 *
 * @param base_address The base physical address to map
 * @param offset The offset from the base address to read from
 * @param values Array to store the read values
 * @param count Number of consecutive registers to read
 * @return true if successful, false otherwise
 */
bool read_registers(uint32_t base_address, uint32_t offset, uint32_t *values, int count);

#endif /* MEMORY_OPS_H */
