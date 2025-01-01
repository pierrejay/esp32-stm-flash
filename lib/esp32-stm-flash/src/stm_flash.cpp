#include "stm_flash.h"

static const char *TAG_STM_FLASH = "stm_flash";

stm32flash::FlashStatus writeTask(FILE *flash_file, gpio_num_t reset_pin, gpio_num_t boot0_pin, uart_port_t uart_num)
{
    logI(TAG_STM_FLASH, "%s", "Write Task");

    char loadAddress[4] = {0x08, 0x00, 0x00, 0x00};
    char block[256] = {0};
    int curr_block = 0, bytes_read = 0;

    fseek(flash_file, 0, SEEK_SET);
    if (isSTMPresent(reset_pin, boot0_pin, uart_num) != stm32flash::SUCCESS) {
        return stm32flash::ERROR_STM_NOT_FOUND;
    }
    stm32flash::FlashStatus status = setupSTM(reset_pin, uart_num);
    if (status != stm32flash::SUCCESS) {
        return status;
    }

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

    return stm32flash::SUCCESS;
}

stm32flash::FlashStatus readTask(FILE *flash_file, uart_port_t uart_num)
{
    logI(TAG_STM_FLASH, "%s", "Read & Verification Task");
    char readAddress[4] = {0x08, 0x00, 0x00, 0x00};

    char block[257] = {0};
    int curr_block = 0, bytes_read = 0;

    fseek(flash_file, 0, SEEK_SET);

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

    return stm32flash::SUCCESS;
}

stm32flash::FlashStatus flashSTM(const char *file_name, gpio_num_t reset_pin, gpio_num_t boot0_pin, uart_port_t uart_num)
{
    FILE *flash_file = NULL;
    
    // Vérifier l'état de SPIFFS
    size_t total = 0, used = 0;
    if (esp_spiffs_info(NULL, &total, &used) == ESP_OK) {
        logI(TAG_STM_FLASH, "SPIFFS Status - Total: %d, Used: %d", total, used);
    } else {
        logE(TAG_STM_FLASH, "Failed to get SPIFFS status!");
        return stm32flash::ERROR_SPIFFS_INIT;
    }
    
    char file_path[FILE_PATH_MAX];
    sprintf(file_path, "%s%s", BASE_PATH, file_name);

    // Vérifier si le fichier existe
    struct stat st;
    if (stat(file_path, &st) != 0) {
        logE(TAG_STM_FLASH, "File not found: %s", file_path);
        return stm32flash::ERROR_FILE_NOT_FOUND;
    }
    logI(TAG_STM_FLASH, "Found file, size: %ld bytes", st.st_size);

    // Vérifier la taille du fichier
    if (st.st_size == 0) {
        logE(TAG_STM_FLASH, "File is empty!");
        return stm32flash::ERROR_FILE_EMPTY;
    }
    if (st.st_size > MAX_FLASH_SIZE) {
        logE(TAG_STM_FLASH, "File too large: %ld bytes (max: %d)", st.st_size, MAX_FLASH_SIZE);
        return stm32flash::ERROR_FILE_TOO_LARGE;
    }

    // Déplacer initGPIO() avant isSTMPresent()
    if (initGPIO(reset_pin, boot0_pin) != stm32flash::SUCCESS) {
        return stm32flash::ERROR_GPIO_INIT;
    }
    
    if (isSTMPresent(reset_pin, boot0_pin, uart_num) != stm32flash::SUCCESS) {
        logE(TAG_STM_FLASH, "STM32 not detected, aborting flash!");
        return stm32flash::ERROR_STM_NOT_FOUND;
    }
    
    flash_file = fopen(file_path, "rb");
    if (flash_file == NULL) {
        logE(TAG_STM_FLASH, "Failed to open file");
        return stm32flash::ERROR_CANNOT_OPEN_FILE;
    }

    do {
        logI(TAG_STM_FLASH, "%s", "Writing STM32 Memory");
        stm32flash::FlashStatus status = writeTask(flash_file, reset_pin, boot0_pin, uart_num);
        if (status != stm32flash::SUCCESS) {
            return status;
        }

        logI(TAG_STM_FLASH, "%s", "Reading STM32 Memory");
        status = readTask(flash_file, uart_num);
        if (status != stm32flash::SUCCESS) {
            return status;
        }

        logI(TAG_STM_FLASH, "%s", "STM32 Flashed Successfully!!!");
    } while (0);

    if (flash_file != NULL) {
        fclose(flash_file);
    }
    endConn(reset_pin, boot0_pin);  // Toujours appeler endConn même en cas d'erreur

    return stm32flash::SUCCESS;
}
