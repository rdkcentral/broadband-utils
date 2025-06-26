#define _GNU_SOURCE
#define _POSIX_C_SOURCE 200809L

#include "libfwbpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mount.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/wait.h>

// Check for setenv availability
#if defined(_GNU_SOURCE) || defined(_POSIX_C_SOURCE)
#define HAVE_SETENV 1
#endif

#define MAX_PATH 512
#define MAX_CMD 1024
#define SECTOR_SIZE 512

// Global library state
static fwbpi_config_t g_config = {0};
static int g_initialized = 0;

// Partition constants
static const char* ROOTFS_A = "/dev/mmcblk0p4";
static const char* ROOTFS_B = "/dev/mmcblk0p8";
static const char* KERNEL_A = "/dev/mmcblk0p3";
static const char* KERNEL_B = "/dev/mmcblk0p7";

// Default configuration
static const fwbpi_config_t DEFAULT_CONFIG = {
    .staging_device = "/dev/mmcblk0p16",
    .staging_mount = "/staging",
    .temp_wic_path = "/tmp/firmware.bin.wic",
    .progress_cb = NULL
};

// Internal helper functions
static int execute_command(const char* cmd);
static void report_progress(const char* stage, int percent, const char* message);
static int safe_setenv(const char* name, const char* value, int overwrite);

//=============================================================================
// Public API Implementation
//=============================================================================

fwbpi_result_t fwbpi_init(const fwbpi_config_t* config) {
    printf("Funcation Name is %s\n",__FUNCTION__); 
    if (g_initialized) {
        return FWBPI_SUCCESS; // Already initialized
    }

    // Use provided config or defaults
    if (config) {
        g_config = *config;
    } else {
        g_config = DEFAULT_CONFIG;
    }

    // Fill in NULL fields with defaults
    if (!g_config.staging_device) g_config.staging_device = DEFAULT_CONFIG.staging_device;
    if (!g_config.staging_mount) g_config.staging_mount = DEFAULT_CONFIG.staging_mount;
    if (!g_config.temp_wic_path) g_config.temp_wic_path = DEFAULT_CONFIG.temp_wic_path;

    g_initialized = 1;
    report_progress("init", 0, "Library initialized");
    return FWBPI_SUCCESS;
}

void fwbpi_cleanup(void) {
     printf("Funcation Name is %s\n",__FUNCTION__);
    if (!g_initialized) return;

    // Cleanup any mounted filesystems
    char cmd[MAX_CMD];
    snprintf(cmd, sizeof(cmd), "umount %s 2>/dev/null || true", g_config.staging_mount);
    execute_command(cmd);

    // Remove temporary files
    execute_command("rm -f /tmp/firmware.bin.wic");
    execute_command("rm -f passivekernel_*.md5");
    execute_command("rm -f /tmp/bl2_backup.img /tmp/fip_backup.bin");

    g_initialized = 0;
    report_progress("cleanup", 100, "Library cleanup complete");
}

fwbpi_result_t fwbpi_update_firmware(const char* image_path) {
    fwbpi_result_t result;
    fwbpi_bank_info_t bank_info;
    int boot_switch_required = 1;

    if (!g_initialized) {
        return FWBPI_ERROR_INVALID_PARAM;
    }

    if (!image_path) {
        return FWBPI_ERROR_INVALID_PARAM;
    }

    report_progress("update", 0, "Starting firmware update");

    // Setup environment
    result = fwbpi_setup_environment();
    if (result != FWBPI_SUCCESS) return result;
    report_progress("update", 10, "Environment setup complete");

    // Create staging area
    result = fwbpi_create_staging_area();
    if (result != FWBPI_SUCCESS) return result;
    report_progress("update", 20, "Staging area created");

    // Download firmware
    result = fwbpi_download_firmware(image_path);
    if (result != FWBPI_SUCCESS) return result;
    report_progress("update", 30, "Firmware downloaded");

    // Decompress image
    result = fwbpi_decompress_image();
    if (result != FWBPI_SUCCESS) return result;
    report_progress("update", 40, "Image decompressed");

    // Get bank information
    result = fwbpi_get_bank_info(&bank_info);
    if (result != FWBPI_SUCCESS) return result;
    report_progress("update", 50, "Bank information determined");

    // Update kernel partition
    result = fwbpi_update_kernel_partition(&bank_info);
    if (result != FWBPI_SUCCESS) return result;
    report_progress("update", 70, "Kernel partition updated");

    // Update rootfs partition
    result = fwbpi_update_rootfs_partition(&bank_info);
    if (result != FWBPI_SUCCESS) return result;
    report_progress("update", 80, "Rootfs partition updated");

    // Verify kernel MD5
    result = fwbpi_verify_kernel_md5(&bank_info);
    if (result != FWBPI_SUCCESS) {
        boot_switch_required = 0;
        report_progress("update", 85, "Kernel verification failed - skipping boot switch");
    } else {
        report_progress("update", 90, "Kernel verification successful");
    }

    // Perform boot switch
    result = fwbpi_perform_boot_switch(boot_switch_required);
    if (result != FWBPI_SUCCESS) return result;

    report_progress("update", 100, "Firmware update complete");
    return FWBPI_SUCCESS;
}

fwbpi_result_t fwbpi_setup_environment(void) {
    // Source configuration files (ignore errors for optional files)
    execute_command(". /etc/include.properties 2>/dev/null || true");
    execute_command(". /etc/device.properties 2>/dev/null || true");
    
    // Set environment variables
    char* current_path = getenv("PATH");
    char* current_ld_path = getenv("LD_LIBRARY_PATH");
    
    char new_path[MAX_PATH];
    char new_ld_path[MAX_PATH];
    
    snprintf(new_path, sizeof(new_path), "%s:/usr/bin:/bin:/usr/local/bin:/sbin:/usr/local/lighttpd/sbin:/usr/local/sbin", 
             current_path ? current_path : "");
    snprintf(new_ld_path, sizeof(new_ld_path), "%s:/usr/local/lib", 
             current_ld_path ? current_ld_path : "");
    
    safe_setenv("PATH", new_path, 1);
    safe_setenv("LD_LIBRARY_PATH", new_ld_path, 1);
    
    return FWBPI_SUCCESS;
}

fwbpi_result_t fwbpi_create_staging_area(void) {
    char cmd[MAX_CMD];
    
    snprintf(cmd, sizeof(cmd), "mkdir -p %s", g_config.staging_mount);
    execute_command(cmd);
    
    snprintf(cmd, sizeof(cmd), "mkfs.ext4 -F %s -L staging", g_config.staging_device);
    if (execute_command(cmd) != 0) {
        return FWBPI_ERROR_PARTITION_FAILED;
    }
    
    if (mount(g_config.staging_device, g_config.staging_mount, "ext4", 0, NULL) != 0) {
        return FWBPI_ERROR_MOUNT_FAILED;
    }
    
    return FWBPI_SUCCESS;
}

fwbpi_result_t fwbpi_download_firmware(const char* image_path) {
    char cmd[MAX_CMD];
    
    snprintf(cmd, sizeof(cmd), "wget -O %s/firmware.bin.wic.bz2 \"%s\"", 
             g_config.staging_mount, image_path);
    
    if (execute_command(cmd) != 0) {
        return FWBPI_ERROR_DOWNLOAD_FAILED;
    }
    
    snprintf(cmd, sizeof(cmd), "chmod 644 %s/firmware.bin.wic.bz2", g_config.staging_mount);
    execute_command(cmd);
    
    return FWBPI_SUCCESS;
}

fwbpi_result_t fwbpi_decompress_image(void) {
    char cmd[MAX_CMD];
    
    snprintf(cmd, sizeof(cmd), "bzcat %s/firmware.bin.wic.bz2 > %s", 
             g_config.staging_mount, g_config.temp_wic_path);
    
    if (execute_command(cmd) != 0) {
        return FWBPI_ERROR_DECOMPRESS_FAILED;
    }
    
    snprintf(cmd, sizeof(cmd), "chmod 644 %s", g_config.temp_wic_path);
    execute_command(cmd);
    
    return FWBPI_SUCCESS;
}

long fwbpi_get_wic_partition_offset(const char* wic_file, int part_index) {
    char cmd[MAX_CMD];
    FILE* fp;
    long sector = 0;
    
    if (access(wic_file, F_OK) != 0) {
        return -1;
    }
    
    snprintf(cmd, sizeof(cmd), 
        "fdisk -l \"%s\" | awk -v idx=\"%d\" -v wic=\"%s\" '"
        "$1 ~ \"^\"wic\"[0-9]+$\" {count++} count == idx {print $2; exit}'",
        wic_file, part_index, wic_file);
    
    fp = popen(cmd, "r");
    if (fp == NULL) {
        return -1;
    }
    
    if (fscanf(fp, "%ld", &sector) != 1) {
        pclose(fp);
        return -1;
    }
    
    pclose(fp);
    return sector * SECTOR_SIZE;
}

fwbpi_result_t fwbpi_get_bank_info(fwbpi_bank_info_t* bank_info) {
    FILE* fp;
    char cmdline[MAX_PATH];
    char active_rootfs[MAX_PATH];
    
    if (!bank_info) {
        return FWBPI_ERROR_INVALID_PARAM;
    }
    
    // Read current active rootfs from /proc/cmdline
    fp = fopen("/proc/cmdline", "r");
    if (!fp) {
        return FWBPI_ERROR_BANK_DETECTION_FAILED;
    }
    
    if (!fgets(cmdline, sizeof(cmdline), fp)) {
        fclose(fp);
        return FWBPI_ERROR_BANK_DETECTION_FAILED;
    }
    fclose(fp);
    
    // Extract root= parameter
    char* root_start = strstr(cmdline, "root=");
    if (!root_start) {
        return FWBPI_ERROR_BANK_DETECTION_FAILED;
    }
    
    root_start += 5; // Skip "root="
    char* root_end = strchr(root_start, ' ');
    if (root_end) {
        *root_end = '\0';
    }
    strcpy(active_rootfs, root_start);
    
    // Determine bank configuration
    if (strcmp(active_rootfs, ROOTFS_A) == 0) {
        bank_info->active_bank = 'A';
        bank_info->passive_rootfs = ROOTFS_B;
        bank_info->passive_kernel = KERNEL_B;
        bank_info->passive_kernel_partition = 7;
        bank_info->next_active_partition = 7;
        bank_info->next_rootfs = "/dev/mmcblk0p8";
    } else {
        bank_info->active_bank = 'B';
        bank_info->passive_rootfs = ROOTFS_A;
        bank_info->passive_kernel = KERNEL_A;
        bank_info->passive_kernel_partition = 3;
        bank_info->next_active_partition = 3;
        bank_info->next_rootfs = "/dev/mmcblk0p4";
    }
    
    // Get partition offsets
    bank_info->passive_kernel_offset = fwbpi_get_wic_partition_offset(
        g_config.temp_wic_path, bank_info->passive_kernel_partition);
    bank_info->passive_rootfs_offset = fwbpi_get_wic_partition_offset(
        g_config.temp_wic_path, (bank_info->passive_kernel_partition == 7) ? 8 : 4);
    
    if (bank_info->passive_kernel_offset < 0 || bank_info->passive_rootfs_offset < 0) {
        return FWBPI_ERROR_PARTITION_FAILED;
    }
    
    return FWBPI_SUCCESS;
}

fwbpi_result_t fwbpi_update_kernel_partition(const fwbpi_bank_info_t* bank_info) {
    char cmd[MAX_CMD];
    
    if (!bank_info) {
        return FWBPI_ERROR_INVALID_PARAM;
    }
    
    execute_command("mkdir -p /extblock/kernel_from_image");
    
    snprintf(cmd, sizeof(cmd), "mount -o loop,offset=%ld %s /extblock/kernel_from_image", 
             bank_info->passive_kernel_offset, g_config.temp_wic_path);
    if (execute_command(cmd) != 0) {
        return FWBPI_ERROR_MOUNT_FAILED;
    }
    
    // Unmount passive kernel if mounted
    snprintf(cmd, sizeof(cmd), "umount %s 2>/dev/null || true", bank_info->passive_kernel);
    execute_command(cmd);
    
    snprintf(cmd, sizeof(cmd), "mkfs.vfat -F 32 %s", bank_info->passive_kernel);
    if (execute_command(cmd) != 0) {
        execute_command("umount /extblock/kernel_from_image");
        return FWBPI_ERROR_PARTITION_FAILED;
    }
    
    execute_command("mkdir -p /mnt/boot_b");
    snprintf(cmd, sizeof(cmd), "mount %s /mnt/boot_b", bank_info->passive_kernel);
    if (execute_command(cmd) != 0) {
        execute_command("umount /extblock/kernel_from_image");
        return FWBPI_ERROR_MOUNT_FAILED;
    }
    
    // Check if fitImage exists
    if (access("/extblock/kernel_from_image/fitImage", F_OK) != 0) {
        execute_command("umount /extblock/kernel_from_image");
        execute_command("umount /mnt/boot_b");
        return FWBPI_ERROR_FILE_NOT_FOUND;
    }
    
    if (execute_command("rsync -a --delete /extblock/kernel_from_image/ /mnt/boot_b/") != 0) {
        execute_command("umount /extblock/kernel_from_image");
        execute_command("umount /mnt/boot_b");
        return FWBPI_ERROR_SYSTEM_CMD_FAILED;
    }
    
    execute_command("sync");
    execute_command("umount /extblock/kernel_from_image");
    execute_command("umount /mnt/boot_b");
    execute_command("rm -rf /extblock/kernel_from_image");
    
    return FWBPI_SUCCESS;
}

fwbpi_result_t fwbpi_update_rootfs_partition(const fwbpi_bank_info_t* bank_info) {
    char cmd[MAX_CMD];
    
    if (!bank_info) {
        return FWBPI_ERROR_INVALID_PARAM;
    }
    
    execute_command("mkdir -p /tmp/rootfs_from_image");
    
    snprintf(cmd, sizeof(cmd), "mount -o loop,offset=%ld %s /tmp/rootfs_from_image", 
             bank_info->passive_rootfs_offset, g_config.temp_wic_path);
    if (execute_command(cmd) != 0) {
        return FWBPI_ERROR_MOUNT_FAILED;
    }
    
    // Unmount passive rootfs if mounted
    snprintf(cmd, sizeof(cmd), "umount %s 2>/dev/null || true", bank_info->passive_rootfs);
    execute_command(cmd);
    
    snprintf(cmd, sizeof(cmd), "mkfs.ext4 -F %s", bank_info->passive_rootfs);
    if (execute_command(cmd) != 0) {
        execute_command("umount /tmp/rootfs_from_image");
        return FWBPI_ERROR_PARTITION_FAILED;
    }
    
    execute_command("mkdir -p /mnt/rootfs_b");
    snprintf(cmd, sizeof(cmd), "mount %s /mnt/rootfs_b", bank_info->passive_rootfs);
    if (execute_command(cmd) != 0) {
        execute_command("umount /tmp/rootfs_from_image");
        return FWBPI_ERROR_MOUNT_FAILED;
    }
    
    if (execute_command("rsync -a --delete /tmp/rootfs_from_image/ /mnt/rootfs_b/") != 0) {
        execute_command("umount /tmp/rootfs_from_image");
        execute_command("umount /mnt/rootfs_b");
        return FWBPI_ERROR_SYSTEM_CMD_FAILED;
    }
    
    execute_command("sync");
    execute_command("umount /tmp/rootfs_from_image");
    execute_command("umount /mnt/rootfs_b");
    execute_command("rm -rf /tmp/rootfs_from_image");
    
    return FWBPI_SUCCESS;
}

fwbpi_result_t fwbpi_verify_kernel_md5(const fwbpi_bank_info_t* bank_info) {
    char cmd[MAX_CMD];
    FILE *fp1, *fp2;
    char expected_md5[33], written_md5[33];
    
    if (!bank_info) {
        return FWBPI_ERROR_INVALID_PARAM;
    }
    
    // Mount and get MD5 of written kernel
    execute_command("mkdir -p /mnt/verify_kernel");
    snprintf(cmd, sizeof(cmd), "mount %s /mnt/verify_kernel", bank_info->passive_kernel);
    execute_command(cmd);
    
    execute_command("md5sum /mnt/verify_kernel/fitImage > passivekernel_written.md5");
    execute_command("umount /mnt/verify_kernel");
    
    // Mount and get MD5 of expected kernel
    execute_command("mkdir -p /extblock/kernel_from_image");
    snprintf(cmd, sizeof(cmd), "mount -o loop,offset=%ld %s /extblock/kernel_from_image", 
             bank_info->passive_kernel_offset, g_config.temp_wic_path);
    if (execute_command(cmd) != 0) {
        return FWBPI_ERROR_MOUNT_FAILED;
    }
    
    execute_command("md5sum /extblock/kernel_from_image/fitImage > passivekernel_expected.md5");
    execute_command("umount /extblock/kernel_from_image");
    
    // Read MD5 values
    fp1 = fopen("passivekernel_expected.md5", "r");
    fp2 = fopen("passivekernel_written.md5", "r");
    
    if (!fp1 || !fp2) {
        if (fp1) fclose(fp1);
        if (fp2) fclose(fp2);
        return FWBPI_ERROR_FILE_NOT_FOUND;
    }
    
    if (fscanf(fp1, "%32s", expected_md5) != 1 || fscanf(fp2, "%32s", written_md5) != 1) {
        fclose(fp1);
        fclose(fp2);
        return FWBPI_ERROR_FILE_NOT_FOUND;
    }
    
    fclose(fp1);
    fclose(fp2);
    
    if (strcmp(expected_md5, written_md5) != 0) {
        return FWBPI_ERROR_MD5_MISMATCH;
    }
    
    return FWBPI_SUCCESS;
}

fwbpi_result_t fwbpi_perform_boot_switch(int enable_switch) {
    if (enable_switch) {
        // Backup current BL2 and FIP
        if (execute_command("dd if=/dev/mmcblk0p1 of=/tmp/bl2_backup.img bs=512") != 0 ||
            execute_command("dd if=/dev/mmcblk0p2 of=/tmp/fip_backup.bin bs=512") != 0) {
            return FWBPI_ERROR_SYSTEM_CMD_FAILED;
        }
        
        // Switch to new BL2 and FIP
        if (execute_command("dd if=/dev/mmcblk0p5 of=/dev/mmcblk0p1 bs=512 conv=fsync") != 0 ||
            execute_command("dd if=/dev/mmcblk0p6 of=/dev/mmcblk0p2 bs=512 conv=fsync") != 0) {
            return FWBPI_ERROR_SYSTEM_CMD_FAILED;
        }
        
        // Copy backup to alternate slots
        if (execute_command("dd if=/tmp/bl2_backup.img of=/dev/mmcblk0p5 bs=512 conv=fsync") != 0 ||
            execute_command("dd if=/tmp/fip_backup.bin of=/dev/mmcblk0p6 bs=512 conv=fsync") != 0) {
            return FWBPI_ERROR_SYSTEM_CMD_FAILED;
        }
    }
    
    return FWBPI_SUCCESS;
}

const char* fwbpi_get_error_string(fwbpi_result_t result) {
    switch (result) {
        case FWBPI_SUCCESS: return "Success";
        case FWBPI_ERROR_INVALID_PARAM: return "Invalid parameter";
        case FWBPI_ERROR_FILE_NOT_FOUND: return "File not found";
        case FWBPI_ERROR_MOUNT_FAILED: return "Mount operation failed";
        case FWBPI_ERROR_DOWNLOAD_FAILED: return "Download failed";
        case FWBPI_ERROR_DECOMPRESS_FAILED: return "Decompression failed";
        case FWBPI_ERROR_PARTITION_FAILED: return "Partition operation failed";
        case FWBPI_ERROR_MD5_MISMATCH: return "MD5 checksum mismatch";
        case FWBPI_ERROR_SYSTEM_CMD_FAILED: return "System command failed";
        case FWBPI_ERROR_BANK_DETECTION_FAILED: return "Bank detection failed";
        default: return "Unknown error";
    }
}

fwbpi_result_t fwbpi_reboot_system(void) {
    return execute_command("reboot -f") == 0 ? FWBPI_SUCCESS : FWBPI_ERROR_SYSTEM_CMD_FAILED;
}

//=============================================================================
// Internal helper functions
//=============================================================================

static int safe_setenv(const char* name, const char* value, int overwrite) {
#ifdef HAVE_SETENV
    return setenv(name, value, overwrite);
#else
    // Fallback implementation for systems without setenv
    if (!overwrite && getenv(name)) {
        return 0; // Variable exists and overwrite is 0
    }
    
    size_t name_len = strlen(name);
    size_t value_len = strlen(value);
    char* env_string = malloc(name_len + value_len + 2); // +2 for '=' and '\0'
    
    if (!env_string) {
        return -1;
    }
    
    sprintf(env_string, "%s=%s", name, value);
    return putenv(env_string);
    // Note: env_string is not freed as putenv may store the pointer
#endif
}

static int execute_command(const char* cmd) {
    return system(cmd);
}

static void report_progress(const char* stage, int percent, const char* message) {
    if (g_config.progress_cb) {
        g_config.progress_cb(stage, percent, message);
    }
}

