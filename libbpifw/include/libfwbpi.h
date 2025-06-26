#ifndef LIBFWBPI_H
#define LIBFWBPI_H

#include <stdint.h>

// Return codes
typedef enum {
    FWBPI_SUCCESS = 0,
    FWBPI_ERROR_INVALID_PARAM = -1,
    FWBPI_ERROR_FILE_NOT_FOUND = -2,
    FWBPI_ERROR_MOUNT_FAILED = -3,
    FWBPI_ERROR_DOWNLOAD_FAILED = -4,
    FWBPI_ERROR_DECOMPRESS_FAILED = -5,
    FWBPI_ERROR_PARTITION_FAILED = -6,
    FWBPI_ERROR_MD5_MISMATCH = -7,
    FWBPI_ERROR_SYSTEM_CMD_FAILED = -8,
    FWBPI_ERROR_BANK_DETECTION_FAILED = -9
} fwbpi_result_t;

// Bank information structure
typedef struct {
    char active_bank;                    // 'A' or 'B'
    const char* passive_rootfs;          // Passive rootfs partition
    const char* passive_kernel;          // Passive kernel partition
    const char* next_rootfs;             // Next rootfs partition
    int passive_kernel_partition;        // Partition number
    int next_active_partition;           // Next active partition number
    long passive_kernel_offset;          // Offset in WIC image
    long passive_rootfs_offset;          // Offset in WIC image
} fwbpi_bank_info_t;

// Progress callback function type
typedef void (*fwbpi_progress_callback_t)(const char* stage, int percent, const char* message);

// Configuration structure
typedef struct {
    const char* staging_device;          // Default: "/dev/mmcblk0p16"
    const char* staging_mount;           // Default: "/staging"
    const char* temp_wic_path;           // Default: "/tmp/firmware.bin.wic"
    fwbpi_progress_callback_t progress_cb; // Progress callback (optional)
} fwbpi_config_t;

#ifdef __cplusplus
extern "C" {
#endif

// Library initialization and cleanup
fwbpi_result_t fwbpi_init(const fwbpi_config_t* config);
void fwbpi_cleanup(void);

// Main firmware update API
fwbpi_result_t fwbpi_update_firmware(const char* image_path);

// Individual operation APIs (for advanced usage)
fwbpi_result_t fwbpi_setup_environment(void);
fwbpi_result_t fwbpi_create_staging_area(void);
fwbpi_result_t fwbpi_download_firmware(const char* image_path);
fwbpi_result_t fwbpi_decompress_image(void);
fwbpi_result_t fwbpi_get_bank_info(fwbpi_bank_info_t* bank_info);
fwbpi_result_t fwbpi_update_kernel_partition(const fwbpi_bank_info_t* bank_info);
fwbpi_result_t fwbpi_update_rootfs_partition(const fwbpi_bank_info_t* bank_info);
fwbpi_result_t fwbpi_verify_kernel_md5(const fwbpi_bank_info_t* bank_info);
fwbpi_result_t fwbpi_perform_boot_switch(int enable_switch);

// Utility functions
long fwbpi_get_wic_partition_offset(const char* wic_file, int part_index);
const char* fwbpi_get_error_string(fwbpi_result_t result);

// System control
fwbpi_result_t fwbpi_reboot_system(void);

#ifdef __cplusplus
}
#endif

#endif // LIBFWBPI_H
