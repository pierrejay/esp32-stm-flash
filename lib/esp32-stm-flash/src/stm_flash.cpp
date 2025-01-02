#include "stm_flash.h"

namespace stm32flash {
namespace internal {

static const char *TAG_STM_FLASH = "stm_flash";

FlashStatus flashSTM(const char *file_name, gpio_num_t reset_pin, gpio_num_t boot0_pin, gpio_num_t uart_tx, gpio_num_t uart_rx, uart_port_t uart_num)
{
    FILE *flash_file = NULL;

    // Initialize SPIFFS
    if (initSPIFFS() != stm32flash::SUCCESS) {
        logE(TAG_STM_FLASH, "Failed to initialize SPIFFS, aborting flash!");
        return stm32flash::ERROR_SPIFFS_INIT;
    }
    
    // Check SPIFFS status
    size_t total = 0, used = 0;
    if (esp_spiffs_info(NULL, &total, &used) == ESP_OK) {
        logI(TAG_STM_FLASH, "SPIFFS Status - Total: %d, Used: %d", total, used);
    } else {
        logE(TAG_STM_FLASH, "Failed to get SPIFFS status, aborting flash!");
        return stm32flash::ERROR_SPIFFS_INIT;
    }
    
    char file_path[FILE_PATH_MAX];
    sprintf(file_path, "%s%s", BASE_PATH, file_name);

    // Check if file exists
    struct stat st;
    if (stat(file_path, &st) != 0) {
        logE(TAG_STM_FLASH, "File not found: %s, aborting flash!", file_path);
        return stm32flash::ERROR_FILE_NOT_FOUND;
    }
    logI(TAG_STM_FLASH, "Found file, size: %ld bytes", st.st_size);

    // Check file size
    if (st.st_size == 0) {
        logE(TAG_STM_FLASH, "File is empty, aborting flash!");
        return stm32flash::ERROR_FILE_EMPTY;
    }
    if (st.st_size > MAX_FLASH_SIZE) {
        logE(TAG_STM_FLASH, "File too large: %ld bytes (max: %d), aborting flash!", st.st_size, MAX_FLASH_SIZE);
        return stm32flash::ERROR_FILE_TOO_LARGE;
    }

    // Enter flash mode
    if (setFlashMode(reset_pin, boot0_pin, uart_num, true) != stm32flash::SUCCESS) {
        logE(TAG_STM_FLASH, "Failed to set flash mode, aborting flash!");
        return stm32flash::ERROR_GPIO_INIT;
    }

    // Initialize UART
    if (initFlashUART(uart_num, uart_tx, uart_rx) != stm32flash::SUCCESS) {
        logE(TAG_STM_FLASH, "Failed to initialize UART, aborting flash!");
        return stm32flash::ERROR_UART_INIT;
    }
    
    // Check if STM32 is present
    if (isSTMPresent(reset_pin, uart_num) != stm32flash::SUCCESS) {
        logE(TAG_STM_FLASH, "STM32 not detected, aborting flash!");
        return stm32flash::ERROR_STM_NOT_FOUND;
    }
    
    // Open file
    flash_file = fopen(file_path, "rb");
    if (flash_file == NULL) {
        logE(TAG_STM_FLASH, "Failed to open file, aborting flash!");
        return stm32flash::ERROR_CANNOT_OPEN_FILE;
    }

    // Execute flash sequence
    do {
        logI(TAG_STM_FLASH, "%s", "Writing STM32 Memory");
        stm32flash::FlashStatus status = writeTask(flash_file, reset_pin, uart_num);
        if (status != stm32flash::SUCCESS) {
            logE(TAG_STM_FLASH, "Write failed, aborting flash!");
            return status;
        }

        logI(TAG_STM_FLASH, "%s", "Reading STM32 Memory");
        status = readTask(flash_file, uart_num);
        if (status != stm32flash::SUCCESS) {
            logE(TAG_STM_FLASH, "Read & Verification failed, aborting flash!");
            return status;
        }

        logI(TAG_STM_FLASH, "%s", "STM32 Flashed Successfully!!!");
    } while (0);

    // Close file
    if (flash_file != NULL) {
        fclose(flash_file);
    }
    
    // Disable flash mode and reboot STM32
    setFlashMode(reset_pin, boot0_pin, uart_num, false);

    return stm32flash::SUCCESS;
}

FlashStatus writeTask(FILE *flash_file, gpio_num_t reset_pin, uart_port_t uart_num)
{
    logI(TAG_STM_FLASH, "%s", "Starting Write Task");

    char loadAddress[4] = {0x08, 0x00, 0x00, 0x00};
    char block[256] = {0};
    int curr_block = 0, bytes_read = 0;

    // Start at the beginning of the file
    fseek(flash_file, 0, SEEK_SET);

    // Setup STM32 to receive the .bin file
    stm32flash::FlashStatus status = setupSTM(reset_pin, uart_num);
    if (status != stm32flash::SUCCESS) {
        return status;
    }

    // Write the .bin file to the STM32
    while ((bytes_read = fread(block, 1, 256, flash_file)) > 0)
    {
        curr_block++;
        logI(TAG_STM_FLASH, "Writing block: %d", curr_block);
        // ESP_LOG_BUFFER_HEXDUMP("Block:  ", block, sizeof(block), ESP_LOG_DEBUG);

        esp_err_t ret = flashPage(loadAddress, block, uart_num);
        if (ret == ESP_FAIL)
        {
            return stm32flash::ERROR_WRITE_FAILED;
        }

        incrementLoadAddress(loadAddress);
        printf("\n");

        memset(block, 0xff, 256);
    }

    logI(TAG_STM_FLASH, "%s", "Write Task Completed");
    return stm32flash::SUCCESS;
}

FlashStatus readTask(FILE *flash_file, uart_port_t uart_num)
{
    logI(TAG_STM_FLASH, "%s", "Starting Read & Verification Task");
    char readAddress[4] = {0x08, 0x00, 0x00, 0x00};

    char block[257] = {0};
    int curr_block = 0, bytes_read = 0;

    // Start at the beginning of the file
    fseek(flash_file, 0, SEEK_SET);

    // Read the .bin file from the STM32
    while ((bytes_read = fread(block, 1, 256, flash_file)) > 0)
    {
        curr_block++;
        logI(TAG_STM_FLASH, "Reading block: %d", curr_block);
        // ESP_LOG_BUFFER_HEXDUMP("Block:  ", block, sizeof(block), ESP_LOG_DEBUG);

        esp_err_t ret = readPage(readAddress, block, uart_num);
        if (ret == ESP_FAIL)
        {
            return stm32flash::ERROR_READ_FAILED;
        }

        incrementLoadAddress(readAddress);
        printf("\n");

        memset(block, 0xff, 256);
    }

    logI(TAG_STM_FLASH, "%s", "Read & Verification Task Completed");
    return stm32flash::SUCCESS;
}

} // namespace internal
} // namespace stm32flash