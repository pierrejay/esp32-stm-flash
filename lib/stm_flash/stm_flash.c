#include "stm_flash.h"

static const char *TAG_STM_FLASH = "stm_flash";

esp_err_t writeTask(FILE *flash_file)
{
    logI(TAG_STM_FLASH, "%s", "Write Task");

    char loadAddress[4] = {0x08, 0x00, 0x00, 0x00};
    char block[256] = {0};
    int curr_block = 0, bytes_read = 0;

    fseek(flash_file, 0, SEEK_SET);
    setupSTM();

    while ((bytes_read = fread(block, 1, 256, flash_file)) > 0)
    {
        curr_block++;
        logI(TAG_STM_FLASH, "Writing block: %d", curr_block);
        // ESP_LOG_BUFFER_HEXDUMP("Block:  ", block, sizeof(block), ESP_LOG_DEBUG);

        esp_err_t ret = flashPage(loadAddress, block);
        if (ret == ESP_FAIL)
        {
            return ESP_FAIL;
        }

        incrementLoadAddress(loadAddress);
        printf("\n");

        memset(block, 0xff, 256);
    }

    return ESP_OK;
}

esp_err_t readTask(FILE *flash_file)
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

        esp_err_t ret = readPage(readAddress, block);
        if (ret == ESP_FAIL)
        {
            return ESP_FAIL;
        }

        incrementLoadAddress(readAddress);
        printf("\n");

        memset(block, 0xff, 256);
    }

    return ESP_OK;
}

esp_err_t flashSTM(const char *file_name)
{
    esp_err_t err = ESP_FAIL;
    FILE *flash_file = NULL;
    
    // Vérifier l'état de SPIFFS
    size_t total = 0, used = 0;
    if (esp_spiffs_info(NULL, &total, &used) == ESP_OK) {
        logI(TAG_STM_FLASH, "SPIFFS Status - Total: %d, Used: %d", total, used);
    } else {
        logE(TAG_STM_FLASH, "Failed to get SPIFFS status!");
    }
    
    char file_path[FILE_PATH_MAX];
    sprintf(file_path, "%s%s", BASE_PATH, file_name);
    logD(TAG_STM_FLASH, "Looking for file: %s", file_path);

    // Vérifier si le fichier existe
    struct stat st;
    if (stat(file_path, &st) != 0) {
        logE(TAG_STM_FLASH, "File not found: %s", file_path);
        return ESP_FAIL;
    }
    logI(TAG_STM_FLASH, "Found file, size: %ld bytes", st.st_size);

    // Vérifier la taille du fichier
    if (st.st_size == 0) {
        logE(TAG_STM_FLASH, "File is empty!");
        return ESP_FAIL;
    }
    if (st.st_size > MAX_FLASH_SIZE) {
        logE(TAG_STM_FLASH, "File too large: %ld bytes (max: %d)", st.st_size, MAX_FLASH_SIZE);
        return ESP_FAIL;
    }

    // Déplacer initGPIO() avant isSTMPresent()
    initGPIO();
    
    if (!isSTMPresent()) {
        logE(TAG_STM_FLASH, "STM32 not detected, aborting flash!");
        return ESP_FAIL;
    }
    
    flash_file = fopen(file_path, "rb");
    if (flash_file == NULL) {
        logE(TAG_STM_FLASH, "Failed to open file");
        return ESP_FAIL;
    }

    do {
        logI(TAG_STM_FLASH, "%s", "Writing STM32 Memory");
        IS_ESP_OK(writeTask(flash_file));

            logI(TAG_STM_FLASH, "%s", "Reading STM32 Memory");
            IS_ESP_OK(readTask(flash_file));

        err = ESP_OK;
        logI(TAG_STM_FLASH, "%s", "STM32 Flashed Successfully!!!");
    } while (0);

    if (flash_file != NULL) {
        fclose(flash_file);
    }
    endConn();  // Toujours appeler endConn même en cas d'erreur

    return err;
}