/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2025 RDK Management
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
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
