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
 * MT7988 Serial Number Tool - Configuration
 *
 * Configuration constants and definitions for the MT7988 Serial Number Tool.
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>

// Register definitions
#define MT7988_REG_BASE 0x11f50000
#define MT7988_REG_OFFSET 0x140
#define SERIAL_REG_COUNT 4

// NVRAM file path
#define NVRAM_FILE "/nvram/serial_number.txt"

#endif /* CONFIG_H */
