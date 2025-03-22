/**
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
