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
