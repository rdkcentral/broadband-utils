/**
 * MT7988 Serial Number Tool - Serial Number Operations Implementation
 *
 * Implementation of serial number processing and storage.
 */

#include "serial_number.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

bool save_serial_number(uint32_t *values, int count, const char *filename) {
    FILE *nvram_file;
    
    // Open NVRAM file for writing
    nvram_file = fopen(filename, "w");
    if (nvram_file == NULL) {
        perror("Error opening NVRAM file");
        return false;
    }
    
    // Write values to file without 0x prefix and without spaces, with newline at end
    for (int i = 0; i < count; i++) {
        fprintf(nvram_file, "%08x", values[i]);
    }
    fprintf(nvram_file, "\n");  // Add newline at the end
    
    // Close the file
    fclose(nvram_file);
    
    return true;
}
