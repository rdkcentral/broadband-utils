/**
 * MT7988 Serial Number Tool - Main Entry Point
 *
 * Reads specific registers and saves the values as a serial number to NVRAM
 * without 0x prefixes and without gaps.
 */

#include <stdio.h>
#include <stdlib.h>
#include "memory_ops.h"
#include "serial_number.h"
#include "config.h"

int main(int argc, char *argv[]) {
    uint32_t read_values[SERIAL_REG_COUNT];
    
    // Print the command being emulated
    printf("./rdkmmap --dump 0x140 --count 16\n");
    
    // Read the registers
    if (!read_registers(MT7988_REG_BASE, MT7988_REG_OFFSET, read_values, SERIAL_REG_COUNT)) {
        return EXIT_FAILURE;
    }
    
    // Save the serial number to NVRAM
    if (!save_serial_number(read_values, SERIAL_REG_COUNT, NVRAM_FILE)) {
        return EXIT_FAILURE;
    }
    
    // Print the serial number to stdout
    printf("Serial number saved to %s: ", NVRAM_FILE);
    for (int i = 0; i < SERIAL_REG_COUNT; i++) {
        printf("%08x", read_values[i]);
    }
    printf("\n");
    
    return EXIT_SUCCESS;
}
