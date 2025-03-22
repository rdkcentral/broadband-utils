/**
 * MT7988 Serial Number Tool - Memory Operations Implementation
 *
 * Implementation of memory-mapped register operations.
 */

#include "memory_ops.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <errno.h>

bool read_registers(uint32_t base_address, uint32_t offset, uint32_t *values, int count) {
    int fd;
    void *map_base;
    volatile uint32_t *virt_addr;
    bool success = true;
    
    // Open /dev/mem
    fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (fd < 0) {
        perror("Error opening /dev/mem");
        return false;
    }
    
    // Map the memory region
    map_base = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, fd, base_address);
    if (map_base == MAP_FAILED) {
        perror("Error mapping memory");
        close(fd);
        return false;
    }
    
    // Get the address of the register
    virt_addr = (volatile uint32_t *)((char *)map_base + offset);
    
    // Read the register values
    for (int i = 0; i < count; i++) {
        values[i] = virt_addr[i];
    }
    
    // Cleanup memory mapping
    if (munmap(map_base, 4096) != 0) {
        perror("Error unmapping memory");
        success = false;
    }
    
    close(fd);
    return success;
}
