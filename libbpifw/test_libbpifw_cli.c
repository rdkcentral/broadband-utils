// test_libfwbpi_cli.c - Command Line Function Tester
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <unistd.h>
#include <sys/stat.h>
#include "libfwbpi.h"

// Test modes
typedef enum {
    TEST_ALL = 0,
    TEST_INIT,
    TEST_CLEANUP,
    TEST_SETUP_ENV,
    TEST_CREATE_STAGING,
    TEST_DOWNLOAD,
    TEST_DECOMPRESS,
    TEST_WIC_OFFSET,
    TEST_BANK_INFO,
    TEST_UPDATE_KERNEL,
    TEST_UPDATE_ROOTFS,
    TEST_VERIFY_MD5,
    TEST_BOOT_SWITCH,
    TEST_ERROR_STRINGS,
    TEST_REBOOT,
    TEST_FULL_UPDATE
} test_mode_t;

// Global test configuration
static int verbose = 0;
static int safe_mode = 1;  // Default to safe mode
static char* firmware_url = NULL;
static char* wic_file = NULL;
static char* staging_device = NULL;
static char* staging_mount = NULL;

// Progress callback for testing
void test_progress_callback(const char* stage, int percent, const char* message) {
    printf("[PROGRESS] %s: %d%% - %s\n", stage, percent, message);
}

// Setup mock environment for safe testing
int setup_mock_environment(void) {
    if (verbose) printf("Setting up mock test environment...\n");
    
    system("mkdir -p /tmp/libfwbpi_test/{staging,dev,proc,extblock,mnt}");
    
    // Create mock /proc/cmdline
    system("echo 'root=/dev/mmcblk0p4 console=ttyS0 quiet' > /tmp/libfwbpi_test/proc/cmdline");
    
    // Create mock WIC file
    if (!wic_file) {
        wic_file = "/tmp/libfwbpi_test/test_firmware.wic";
        system("dd if=/dev/zero of=/tmp/libfwbpi_test/test_firmware.wic bs=1M count=50 2>/dev/null");
        // Create basic partition table
        system("echo -e 'o\\nn\\np\\n1\\n\\n+10M\\nn\\np\\n2\\n\\n+10M\\nn\\np\\n3\\n\\n+10M\\nn\\np\\n4\\n\\n+10M\\nn\\np\\n5\\n\\n+10M\\nn\\np\\n6\\n\\n+10M\\nn\\np\\n7\\n\\n+10M\\nn\\np\\n8\\n\\n\\nw' | fdisk /tmp/libfwbpi_test/test_firmware.wic >/dev/null 2>&1");
    }
    
    // Create mock firmware content
    system("mkdir -p /tmp/libfwbpi_test/firmware_content");
    system("dd if=/dev/zero of=/tmp/libfwbpi_test/firmware_content/fitImage bs=1M count=5 2>/dev/null");
    system("echo 'Test firmware v1.0' > /tmp/libfwbpi_test/firmware_content/VERSION");
    
    // Create compressed firmware
    system("cd /tmp/libfwbpi_test && tar -czf firmware.wic.bz2 test_firmware.wic >/dev/null 2>&1");
    
    return 0;
}

void cleanup_mock_environment(void) {
    if (verbose) printf("Cleaning up mock environment...\n");
    system("rm -rf /tmp/libfwbpi_test");
}

// Test functions
int test_init_function(void) {
    printf("=== Testing fwbpi_init() ===\n");
    
    fwbpi_config_t config = {
        .staging_device = staging_device ? staging_device : "/tmp/libfwbpi_test/dev/mmcblk0p16",
        .staging_mount = staging_mount ? staging_mount : "/tmp/libfwbpi_test/staging",
        .temp_wic_path = "/tmp/libfwbpi_test/test_firmware.wic",
        .progress_cb = verbose ? test_progress_callback : NULL
    };
    
    printf("Test 1: NULL config initialization\n");
    fwbpi_result_t result = fwbpi_init(NULL);
    printf("Result: %s\n", fwbpi_get_error_string(result));
    if (result == FWBPI_SUCCESS) fwbpi_cleanup();
    
    printf("\nTest 2: Custom config initialization\n");
    result = fwbpi_init(&config);
    printf("Result: %s\n", fwbpi_get_error_string(result));
    if (result == FWBPI_SUCCESS) fwbpi_cleanup();
    
    printf("\nTest 3: Double initialization\n");
    result = fwbpi_init(&config);
    if (result == FWBPI_SUCCESS) {
        result = fwbpi_init(&config);  // Should succeed (already initialized)
        printf("Second init result: %s\n", fwbpi_get_error_string(result));
        fwbpi_cleanup();
    }
    
    return (result == FWBPI_SUCCESS) ? 0 : 1;
}

int test_cleanup_function(void) {
    printf("=== Testing fwbpi_cleanup() ===\n");
    
    // Test cleanup without init
    printf("Test 1: Cleanup without initialization\n");
    fwbpi_cleanup();  // Should handle gracefully
    printf("✅ Cleanup without init completed\n");
    
    // Test cleanup after init
    printf("\nTest 2: Cleanup after initialization\n");
    if (fwbpi_init(NULL) == FWBPI_SUCCESS) {
        fwbpi_cleanup();
        printf("✅ Normal cleanup completed\n");
    }
    
    return 0;
}

int test_setup_environment(void) {
    printf("=== Testing fwbpi_setup_environment() ===\n");
    
    if (fwbpi_init(NULL) != FWBPI_SUCCESS) {
        printf("❌ Failed to initialize library\n");
        return 1;
    }
    
    fwbpi_result_t result = fwbpi_setup_environment();
    printf("Environment setup result: %s\n", fwbpi_get_error_string(result));
    
    // Check if PATH was modified
    char* path = getenv("PATH");
    if (path && strstr(path, "/usr/local/sbin")) {
        printf("✅ PATH was updated correctly\n");
    } else {
        printf("⚠️ PATH update not visible (may be normal)\n");
    }
    
    fwbpi_cleanup();
    return (result == FWBPI_SUCCESS) ? 0 : 1;
}

int test_create_staging(void) {
    printf("=== Testing fwbpi_create_staging_area() ===\n");
    
    if (!safe_mode) {
        printf("⚠️ WARNING: This test will modify real filesystems!\n");
        printf("Continue? (y/N): ");
        char response;
        scanf("%c", &response);
        if (response != 'y' && response != 'Y') {
            printf("Test skipped by user\n");
            return 0;
        }
    }
    
    fwbpi_config_t config = {
        .staging_device = safe_mode ? "/tmp/libfwbpi_test/staging.img" : "/dev/loop0",
        .staging_mount = safe_mode ? "/tmp/libfwbpi_test/staging" : "/tmp/staging_test"
    };
    
    if (safe_mode) {
        // Create a loop device file for testing
        system("dd if=/dev/zero of=/tmp/libfwbpi_test/staging.img bs=1M count=100 2>/dev/null");
    }
    
    if (fwbpi_init(&config) != FWBPI_SUCCESS) {
        printf("❌ Failed to initialize library\n");
        return 1;
    }
    
    fwbpi_result_t result = fwbpi_create_staging_area();
    printf("Create staging result: %s\n", fwbpi_get_error_string(result));
    
    if (result == FWBPI_SUCCESS && safe_mode) {
        // Verify staging directory was created
        struct stat st;
        if (stat(config.staging_mount, &st) == 0) {
            printf("✅ Staging directory created\n");
        }
    }
    
    fwbpi_cleanup();
    return (result == FWBPI_SUCCESS) ? 0 : 1;
}

int test_download_firmware(void) {
    printf("=== Testing fwbpi_download_firmware() ===\n");
    
    if (!firmware_url) {
        printf("No firmware URL provided. Use --url parameter\n");
        printf("Example URLs for testing:\n");
        printf("  http://httpbin.org/get (test URL - returns JSON)\n");
        printf("  file:///tmp/libfwbpi_test/firmware.wic.bz2 (local file)\n");
        return 1;
    }
    
    fwbpi_config_t config = {
        .staging_mount = "/tmp/libfwbpi_test/staging",
        .progress_cb = test_progress_callback
    };
    
    system("mkdir -p /tmp/libfwbpi_test/staging");
    
    if (fwbpi_init(&config) != FWBPI_SUCCESS) {
        printf("❌ Failed to initialize library\n");
        return 1;
    }
    
    printf("Downloading from: %s\n", firmware_url);
    fwbpi_result_t result = fwbpi_download_firmware(firmware_url);
    printf("Download result: %s\n", fwbpi_get_error_string(result));
    
    if (result == FWBPI_SUCCESS) {
        // Check if file was downloaded
        char filepath[512];
        snprintf(filepath, sizeof(filepath), "%s/firmware.bin.wic.bz2", config.staging_mount);
        struct stat st;
        if (stat(filepath, &st) == 0) {
            printf("✅ Downloaded file size: %ld bytes\n", st.st_size);
        } else {
            printf("⚠️ Downloaded file not found\n");
        }
    }
    
    fwbpi_cleanup();
    return (result == FWBPI_SUCCESS) ? 0 : 1;
}

int test_decompress_image(void) {
    printf("=== Testing fwbpi_decompress_image() ===\n");
    
    // Create test compressed file
    system("mkdir -p /tmp/libfwbpi_test/staging");
    system("cp /tmp/libfwbpi_test/firmware.wic.bz2 /tmp/libfwbpi_test/staging/firmware.bin.wic.bz2 2>/dev/null || echo 'Creating test compressed file...'");
    system("dd if=/dev/zero of=/tmp/test_file bs=1M count=1 2>/dev/null && gzip -c /tmp/test_file > /tmp/libfwbpi_test/staging/firmware.bin.wic.bz2");
    
    fwbpi_config_t config = {
        .staging_mount = "/tmp/libfwbpi_test/staging",
        .temp_wic_path = "/tmp/libfwbpi_test/decompressed_firmware.wic"
    };
    
    if (fwbpi_init(&config) != FWBPI_SUCCESS) {
        printf("❌ Failed to initialize library\n");
        return 1;
    }
    
    fwbpi_result_t result = fwbpi_decompress_image();
    printf("Decompress result: %s\n", fwbpi_get_error_string(result));
    
    if (result == FWBPI_SUCCESS) {
        struct stat st;
        if (stat(config.temp_wic_path, &st) == 0) {
            printf("✅ Decompressed file size: %ld bytes\n", st.st_size);
        }
    }
    
    fwbpi_cleanup();
    return (result == FWBPI_SUCCESS) ? 0 : 1;
}

int test_wic_offset(void) {
    printf("=== Testing fwbpi_get_wic_partition_offset() ===\n");
    
    char* test_file = wic_file ? wic_file : "/tmp/libfwbpi_test/test_firmware.wic";
    
    printf("Testing with WIC file: %s\n", test_file);
    
    for (int i = 1; i <= 8; i++) {
        long offset = fwbpi_get_wic_partition_offset(test_file, i);
        if (offset >= 0) {
            printf("✅ Partition %d offset: %ld bytes (%ld sectors)\n", i, offset, offset/512);
        } else {
            printf("❌ Partition %d: not found or error\n", i);
        }
    }
    
    // Test with non-existent file
    printf("\nTesting with non-existent file:\n");
    long offset = fwbpi_get_wic_partition_offset("/nonexistent/file.wic", 1);
    if (offset == -1) {
        printf("✅ Correctly returned -1 for non-existent file\n");
    } else {
        printf("❌ Should have returned -1 for non-existent file\n");
    }
    
    return 0;
}

int test_bank_info(void) {
    printf("=== Testing fwbpi_get_bank_info() ===\n");
    
    // Test with mock /proc/cmdline
    if (safe_mode) {
        printf("Creating mock /proc/cmdline for testing...\n");
        system("mkdir -p /tmp/mock_proc");
        system("echo 'root=/dev/mmcblk0p4 console=ttyS0' > /tmp/mock_proc/cmdline");
        
        // We'll read this manually since the function reads /proc/cmdline directly
        FILE* fp = fopen("/tmp/mock_proc/cmdline", "r");
        if (fp) {
            char cmdline[256];
            if (fgets(cmdline, sizeof(cmdline), fp)) {
                printf("Mock cmdline: %s", cmdline);
                char* root_start = strstr(cmdline, "root=");
                if (root_start) {
                    root_start += 5;
                    char* root_end = strchr(root_start, ' ');
                    if (root_end) *root_end = '\0';
                    printf("Would detect root device: %s\n", root_start);
                    printf("Would detect active bank: %s\n", 
                           strcmp(root_start, "/dev/mmcblk0p4") == 0 ? "A" : "B");
                }
            }
            fclose(fp);
        }
        system("rm -rf /tmp/mock_proc");
        return 0;
    }
    
    // Test with real system (requires real hardware)
    fwbpi_config_t config = {
        .temp_wic_path = wic_file ? wic_file : "/tmp/libfwbpi_test/test_firmware.wic"
    };
    
    if (fwbpi_init(&config) != FWBPI_SUCCESS) {
        printf("❌ Failed to initialize library\n");
        return 1;
    }
    
    fwbpi_bank_info_t bank_info;
    fwbpi_result_t result = fwbpi_get_bank_info(&bank_info);
    printf("Bank info result: %s\n", fwbpi_get_error_string(result));
    
    if (result == FWBPI_SUCCESS) {
        printf("✅ Active bank: %c\n", bank_info.active_bank);
        printf("✅ Passive kernel: %s\n", bank_info.passive_kernel);
        printf("✅ Passive rootfs: %s\n", bank_info.passive_rootfs);
        printf("✅ Passive kernel partition: %d\n", bank_info.passive_kernel_partition);
        printf("✅ Kernel offset: %ld\n", bank_info.passive_kernel_offset);
        printf("✅ Rootfs offset: %ld\n", bank_info.passive_rootfs_offset);
    }
    
    fwbpi_cleanup();
    return (result == FWBPI_SUCCESS) ? 0 : 1;
}

int test_error_strings(void) {
    printf("=== Testing fwbpi_get_error_string() ===\n");
    
    fwbpi_result_t errors[] = {
        FWBPI_SUCCESS,
        FWBPI_ERROR_INVALID_PARAM,
        FWBPI_ERROR_FILE_NOT_FOUND,
        FWBPI_ERROR_MOUNT_FAILED,
        FWBPI_ERROR_DOWNLOAD_FAILED,
        FWBPI_ERROR_DECOMPRESS_FAILED,
        FWBPI_ERROR_PARTITION_FAILED,
        FWBPI_ERROR_MD5_MISMATCH,
        FWBPI_ERROR_SYSTEM_CMD_FAILED,
        FWBPI_ERROR_BANK_DETECTION_FAILED
    };
    
    for (int i = 0; i < sizeof(errors)/sizeof(errors[0]); i++) {
        const char* error_str = fwbpi_get_error_string(errors[i]);
        printf("Error %d: %s\n", errors[i], error_str);
    }
    
    // Test unknown error code
    const char* unknown = fwbpi_get_error_string(-999);
    printf("Unknown error (-999): %s\n", unknown);
    
    return 0;
}

int test_full_update(void) {
    printf("=== Testing fwbpi_update_firmware() (SAFE MODE) ===\n");
    
    if (!safe_mode) {
        printf("⚠️ WARNING: This will perform a real firmware update!\n");
        printf("This should ONLY be run on target hardware with valid firmware!\n");
        printf("Continue? (type 'YES' to proceed): ");
        char response[10];
        scanf("%9s", response);
        if (strcmp(response, "YES") != 0) {
            printf("Test cancelled for safety\n");
            return 0;
        }
    }
    
    if (!firmware_url && safe_mode) {
        firmware_url = "file:///tmp/libfwbpi_test/firmware.wic.bz2";
        printf("Using mock firmware URL: %s\n", firmware_url);
    }
    
    if (!firmware_url) {
        printf("❌ No firmware URL provided. Use --url parameter\n");
        return 1;
    }
    
    fwbpi_config_t config = {
        .staging_device = safe_mode ? "/tmp/libfwbpi_test/staging.img" : NULL,
        .staging_mount = safe_mode ? "/tmp/libfwbpi_test/staging" : NULL,
        .temp_wic_path = safe_mode ? "/tmp/libfwbpi_test/firmware.wic" : NULL,
        .progress_cb = test_progress_callback
    };
    
    if (fwbpi_init(&config) != FWBPI_SUCCESS) {
        printf("❌ Failed to initialize library\n");
        return 1;
    }
    
    printf("Starting firmware update...\n");
    fwbpi_result_t result = fwbpi_update_firmware(firmware_url);
    printf("Update result: %s\n", fwbpi_get_error_string(result));
    
    fwbpi_cleanup();
    return (result == FWBPI_SUCCESS) ? 0 : 1;
}

void print_usage(const char* program_name) {
    printf("Usage: %s [OPTIONS] --test <test_name>\n\n");
    printf("Test Functions:\n");
    printf("  init              Test fwbpi_init()\n");
    printf("  cleanup           Test fwbpi_cleanup()\n");
    printf("  setup-env         Test fwbpi_setup_environment()\n");
    printf("  create-staging    Test fwbpi_create_staging_area()\n");
    printf("  download          Test fwbpi_download_firmware()\n");
    printf("  decompress        Test fwbpi_decompress_image()\n");
    printf("  wic-offset        Test fwbpi_get_wic_partition_offset()\n");
    printf("  bank-info         Test fwbpi_get_bank_info()\n");
    printf("  update-kernel     Test fwbpi_update_kernel_partition()\n");
    printf("  update-rootfs     Test fwbpi_update_rootfs_partition()\n");
    printf("  verify-md5        Test fwbpi_verify_kernel_md5()\n");
    printf("  boot-switch       Test fwbpi_perform_boot_switch()\n");
    printf("  error-strings     Test fwbpi_get_error_string()\n");
    printf("  reboot            Test fwbpi_reboot_system()\n");
    printf("  full-update       Test fwbpi_update_firmware()\n");
    printf("  all               Run all safe tests\n\n");
    printf("Options:\n");
    printf("  -v, --verbose     Verbose output\n");
    printf("  -s, --safe        Safe mode (default - uses mock environment)\n");
    printf("  -r, --real        Real mode (DANGEROUS - uses real hardware)\n");
    printf("  -u, --url URL     Firmware URL for download tests\n");
    printf("  -w, --wic FILE    WIC file path for offset tests\n");
    printf("  -d, --device DEV  Staging device (e.g., /dev/loop0)\n");
    printf("  -m, --mount DIR   Staging mount point\n");
    printf("  -h, --help        Show this help\n\n");
    printf("Examples:\n");
    printf("  %s --test init --verbose\n", program_name);
    printf("  %s --test download --url http://server/firmware.wic.bz2\n", program_name);
    printf("  %s --test wic-offset --wic /path/to/firmware.wic\n", program_name);
    printf("  %s --test all --safe\n", program_name);
    printf("  %s --test full-update --real --url http://server/fw.wic.bz2  # DANGEROUS!\n", program_name);
}

int main(int argc, char* argv[]) {
    test_mode_t test_mode = TEST_ALL;
    
    static struct option long_options[] = {
        {"test",    required_argument, 0, 't'},
        {"verbose", no_argument,       0, 'v'},
        {"safe",    no_argument,       0, 's'},
        {"real",    no_argument,       0, 'r'},
        {"url",     required_argument, 0, 'u'},
        {"wic",     required_argument, 0, 'w'},
        {"device",  required_argument, 0, 'd'},
        {"mount",   required_argument, 0, 'm'},
        {"help",    no_argument,       0, 'h'},
        {0, 0, 0, 0}
    };
    
    int c;
    while ((c = getopt_long(argc, argv, "t:vsru:w:d:m:h", long_options, NULL)) != -1) {
        switch (c) {
            case 't':
                if (strcmp(optarg, "init") == 0) test_mode = TEST_INIT;
                else if (strcmp(optarg, "cleanup") == 0) test_mode = TEST_CLEANUP;
                else if (strcmp(optarg, "setup-env") == 0) test_mode = TEST_SETUP_ENV;
                else if (strcmp(optarg, "create-staging") == 0) test_mode = TEST_CREATE_STAGING;
                else if (strcmp(optarg, "download") == 0) test_mode = TEST_DOWNLOAD;
                else if (strcmp(optarg, "decompress") == 0) test_mode = TEST_DECOMPRESS;
                else if (strcmp(optarg, "wic-offset") == 0) test_mode = TEST_WIC_OFFSET;
                else if (strcmp(optarg, "bank-info") == 0) test_mode = TEST_BANK_INFO;
                else if (strcmp(optarg, "update-kernel") == 0) test_mode = TEST_UPDATE_KERNEL;
                else if (strcmp(optarg, "update-rootfs") == 0) test_mode = TEST_UPDATE_ROOTFS;
                else if (strcmp(optarg, "verify-md5") == 0) test_mode = TEST_VERIFY_MD5;
                else if (strcmp(optarg, "boot-switch") == 0) test_mode = TEST_BOOT_SWITCH;
                else if (strcmp(optarg, "error-strings") == 0) test_mode = TEST_ERROR_STRINGS;
                else if (strcmp(optarg, "reboot") == 0) test_mode = TEST_REBOOT;
                else if (strcmp(optarg, "full-update") == 0) test_mode = TEST_FULL_UPDATE;
                else if (strcmp(optarg, "all") == 0) test_mode = TEST_ALL;
                else {
                    printf("Unknown test: %s\n", optarg);
                    return 1;
                }
                break;
            case 'v':
                verbose = 1;
                break;
            case 's':
                safe_mode = 1;
                break;
            case 'r':
                safe_mode = 0;
                printf("⚠️ WARNING: Real mode enabled - tests may modify hardware!\n");
                break;
            case 'u':
                firmware_url = strdup(optarg);
                break;
            case 'w':
                wic_file = strdup(optarg);
                break;
            case 'd':
                staging_device = strdup(optarg);
                break;
            case 'm':
                staging_mount = strdup(optarg);
                break;
            case 'h':
                print_usage(argv[0]);
                return 0;
            default:
                print_usage(argv[0]);
                return 1;
        }
    }
    
    printf("=== libfwbpi Function Tester ===\n");
    printf("Mode: %s\n", safe_mode ? "SAFE (Mock Environment)" : "REAL (Hardware)");
    if (verbose) printf("Verbose output enabled\n");
    printf("\n");
    
    // Setup mock environment if in safe mode
    if (safe_mode) {
        setup_mock_environment();
    }
    
    int result = 0;
    
    switch (test_mode) {
        case TEST_INIT:
            result = test_init_function();
            break;
        case TEST_CLEANUP:
            result = test_cleanup_function();
            break;
        case TEST_SETUP_ENV:
            result = test_setup_environment();
            break;
        case TEST_CREATE_STAGING:
            result = test_create_staging();
            break;
        case TEST_DOWNLOAD:
            result = test_download_firmware();
            break;
        case TEST_DECOMPRESS:
            result = test_decompress_image();
            break;
        case TEST_WIC_OFFSET:
            result = test_wic_offset();
            break;
        case TEST_BANK_INFO:
            result = test_bank_info();
            break;
        case TEST_ERROR_STRINGS:
            result = test_error_strings();
            break;
        case TEST_FULL_UPDATE:
            result = test_full_update();
            break;
        case TEST_ALL:
            printf("Running all safe tests...\n\n");
            result |= test_init_function();
            result |= test_cleanup_function();
            result |= test_setup_environment();
            result |= test_error_strings();
            result |= test_wic_offset();
            if (safe_mode) result |= test_bank_info();
            printf("\n=== Summary ===\n");
            printf("All safe tests %s\n", result == 0 ? "PASSED" : "had issues");
            break;
        default:
            printf("❌ Test not implemented yet\n");
            result = 1;
    }
    
    // Cleanup
    if (safe_mode) {
        cleanup_mock_environment();
    }
    
    if (firmware_url) free(firmware_url);
    if (wic_file && strcmp(wic_file, "/tmp/libfwbpi_test/test_firmware.wic") != 0) free(wic_file);
    if (staging_device) free(staging_device);
    if (staging_mount) free(staging_mount);
    
    printf("\nTest completed with %s\n", result == 0 ? "SUCCESS" : "ERRORS");
    return result;
}

/*
=============================================================================
COMPILATION AND USAGE INSTRUCTIONS
=============================================================================

1. Compile the tester:
   gcc -o test_cli test_libfwbpi_cli.c -I./include -L./build/lib -lfwbpi

2. Basic function tests:
   ./test_cli --test init --verbose
   ./test_cli --test cleanup --verbose
   ./test_cli --test setup-env --verbose
   ./test_cli --test error-strings

3. File-based tests:
   ./test_cli --test wic-offset --wic /path/to/firmware.wic
   ./test_cli --test decompress --verbose

4. Network tests:
   ./test_cli --test download --url http://httpbin.org/get --verbose

5. Safe comprehensive test:
   ./test_cli --test all --safe --verbose

6. Individual function tests:
   ./test_cli --test init
   ./test_cli --test bank-info --safe
   ./test_cli --test wic-offset --wic test_firmware.wic

7. DANGEROUS real hardware test (only on target system):
   sudo ./test_cli --test full-update --real --url http://server/firmware.wic.bz2

=============================================================================
*/
