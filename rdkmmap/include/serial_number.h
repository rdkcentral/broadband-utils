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
 * limitations under the License
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
