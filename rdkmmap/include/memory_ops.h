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
