#include "stm_pro_mode.h"

static const char *TAG_STM_PRO = "stm_pro_mode";

//Functions for custom adjustments
stm32flash::FlashStatus initFlashUART(uart_port_t uart_num, gpio_num_t tx, gpio_num_t rx)
{
    const uart_config_t uart_config = {
        .baud_rate = UART_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_EVEN,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };

    esp_err_t err = uart_driver_install(uart_num, UART_BUF_SIZE * 2, 0, 0, NULL, 0);
    if (err != ESP_OK) {
        logE(TAG_STM_PRO, "Failed to install UART driver");
        return stm32flash::ERROR_UART_INIT;
    }

    err = uart_param_config(uart_num, &uart_config);
    if (err != ESP_OK) {
        logE(TAG_STM_PRO, "Failed to configure UART");
        return stm32flash::ERROR_UART_INIT;
    }

    err = uart_set_pin(uart_num, tx, rx, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    if (err != ESP_OK) {
        logE(TAG_STM_PRO, "Failed to set UART pins");
        return stm32flash::ERROR_UART_INIT;
    }

    logI(TAG_STM_PRO, "Initialized Flash UART successfully");
    return stm32flash::SUCCESS;
}

stm32flash::FlashStatus initSPIFFS(void)
{
    logI(TAG_STM_PRO, "%s", "Initializing SPIFFS");

    esp_vfs_spiffs_conf_t conf =
        {
            .base_path = "/spiffs",
            .partition_label = NULL,
            .max_files = 5,
            .format_if_mount_failed = true};

    esp_err_t ret = esp_vfs_spiffs_register(&conf);

    if (ret != ESP_OK)
    {
        if (ret == ESP_FAIL)
        {
            logE(TAG_STM_PRO, "%s", "Failed to mount or format filesystem");
        }
        else if (ret == ESP_ERR_NOT_FOUND)
        {
            logE(TAG_STM_PRO, "%s", "Failed to find SPIFFS partition");
        }
        else
        {
            logE(TAG_STM_PRO, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
        }
        return stm32flash::ERROR_SPIFFS_INIT;
    }

/*  // Formatting SPIFFS - Use only for debugging
    if (esp_spiffs_format(NULL) != ESP_OK)
    {
        logE(TAG_STM_PRO, "%s", "Failed to format SPIFFS");
        return stm32flash::ERROR_SPIFFS_INIT;
    }
*/

    size_t total, used;
    if (esp_spiffs_info(NULL, &total, &used) == ESP_OK)
    {
        logI(TAG_STM_PRO, "Partition size: total: %d, used: %d", total, used);
        return stm32flash::SUCCESS;
    }
    return stm32flash::ERROR_SPIFFS_INIT;
}

void resetSTM(gpio_num_t reset_pin)
{
    logI(TAG_STM_PRO, "%s", "Starting RESET Procedure");

    gpio_set_level(reset_pin, LOW);
    vTaskDelay(100 / portTICK_PERIOD_MS);
    gpio_set_level(reset_pin, HIGH);
    vTaskDelay(500 / portTICK_PERIOD_MS);

    logI(TAG_STM_PRO, "%s", "Finished RESET Procedure");
}

stm32flash::FlashStatus setupSTM(gpio_num_t reset_pin, uart_port_t uart_num)
{
    logI(TAG_STM_PRO, "%s", "Starting STM32 Setup Procedure");

    resetSTM(reset_pin);
    if (!cmdSync(uart_num)) return stm32flash::ERROR_STM_SYNC_FAILED;
    if (!cmdGet(uart_num)) return stm32flash::ERROR_STM_GET_COMMANDS_FAILED;
    if (!cmdVersion(uart_num)) return stm32flash::ERROR_STM_GET_VERSION_FAILED;
    if (!cmdId(uart_num)) return stm32flash::ERROR_STM_GET_ID_FAILED;
    cmdErase(uart_num);
    cmdExtErase(uart_num);

    logI(TAG_STM_PRO, "%s", "STM32 Setup Procedure Completed");
    return stm32flash::SUCCESS;
}

int cmdSync(uart_port_t uart_num)
{
    logI(TAG_STM_PRO, "%s", "SYNC");

    char bytes[] = {0x7F};
    int resp = 1;
    return sendBytes(bytes, sizeof(bytes), resp, uart_num);
}

int cmdGet(uart_port_t uart_num)
{
    logI(TAG_STM_PRO, "%s", "GET");

    char bytes[] = {0x00, 0xFF};
    int resp = 15;
    return sendBytes(bytes, sizeof(bytes), resp, uart_num);
}

int cmdVersion(uart_port_t uart_num)
{
    logI(TAG_STM_PRO, "%s", "GET VERSION & READ PROTECTION STATUS");

    char bytes[] = {0x01, 0xFE};
    int resp = 5;
    return sendBytes(bytes, sizeof(bytes), resp, uart_num);
}

int cmdId(uart_port_t uart_num)
{
    logI(TAG_STM_PRO, "%s", "CHECK ID");
    char bytes[] = {0x02, 0xFD};
    int resp = 5;
    return sendBytes(bytes, sizeof(bytes), resp, uart_num);
}

int cmdErase(uart_port_t uart_num)
{
    logI(TAG_STM_PRO, "%s", "ERASE MEMORY");
    char bytes[] = {0x43, 0xBC};
    int resp = 1;
    int a = sendBytes(bytes, sizeof(bytes), resp, uart_num);

    if (a == 1)
    {
        char params[] = {0xFF, 0x00};
        resp = 1;

        return sendBytes(params, sizeof(params), resp, uart_num);
    }
    return 0;
}

int cmdExtErase(uart_port_t uart_num)
{
    logI(TAG_STM_PRO, "%s", "EXTENDED ERASE MEMORY");
    char bytes[] = {0x44, 0xBB};
    int resp = 1;
    int a = sendBytes(bytes, sizeof(bytes), resp, uart_num);

    if (a == 1)
    {
        char params[] = {0xFF, 0xFF, 0x00};
        resp = 1;

        return sendBytes(params, sizeof(params), resp, uart_num);
    }
    return 0;
}

int cmdWrite(uart_port_t uart_num)
{
    logI(TAG_STM_PRO, "%s", "WRITE MEMORY");
    char bytes[2] = {0x31, 0xCE};
    int resp = 1;
    return sendBytes(bytes, sizeof(bytes), resp, uart_num);
}

int cmdRead(uart_port_t uart_num)
{
    logI(TAG_STM_PRO, "%s", "READ MEMORY");
    char bytes[2] = {0x11, 0xEE};
    int resp = 1;
    return sendBytes(bytes, sizeof(bytes), resp, uart_num);
}

int loadAddress(const char adrMS, const char adrMI, const char adrLI, const char adrLS, uart_port_t uart_num)
{
    char xor_ = adrMS ^ adrMI ^ adrLI ^ adrLS;
    char params[] = {adrMS, adrMI, adrLI, adrLS, xor_};
    int resp = 1;

    // ESP_LOG_BUFFER_HEXDUMP("LOAD ADDR", params, sizeof(params), ESP_LOG_DEBUG);
    return sendBytes(params, sizeof(params), resp, uart_num);
}

int sendBytes(const char *bytes, int count, int resp, uart_port_t uart_num)
{
    sendData(TAG_STM_PRO, bytes, count, uart_num);
    int length = waitForSerialData(resp, SERIAL_TIMEOUT, uart_num);

    if (length > 0) {
        
        uint8_t data[length];
        const int rxBytes = uart_read_bytes(uart_num, data, length, 1000 / portTICK_RATE_MS);

        if (rxBytes > 0 && data[0] == ACK)
        {
            logI(TAG_STM_PRO, "%s", "Sync Success");
            // ESP_LOG_BUFFER_HEXDUMP("SYNC", data, rxBytes, ESP_LOG_DEBUG);
            return 1;
        }
        else
        {
            logE(TAG_STM_PRO, "%s", "Sync Failure");
            return 0;
        }
    }
    else
    {
        logE(TAG_STM_PRO, "%s", "Serial Timeout");
        return 0;
    }

    return 0;
}

int sendData(const char *logName, const char *data, const int count, uart_port_t uart_num)
{
    const int txBytes = uart_write_bytes(uart_num, data, count);
    return txBytes;
}

int waitForSerialData(int dataCount, int timeout, uart_port_t uart_num)
{
    int timer = 0;
    int length = 0;
    while (timer < timeout)
    {
        uart_get_buffered_data_len(uart_num, (size_t *)&length);
        if (length >= dataCount)
        {
            return length;
        }
        vTaskDelay(1 / portTICK_PERIOD_MS);
        timer++;
    }
    return 0;
}

void incrementLoadAddress(char *loadAddr)
{
    loadAddr[2] += 0x1;

    if (loadAddr[2] == 0)
    {
        loadAddr[1] += 0x1;

        if (loadAddr[1] == 0)
        {
            loadAddr[0] += 0x1;
        }
    }
}

esp_err_t flashPage(const char *address, const char *data, uart_port_t uart_num)
{
    logI(TAG_STM_PRO, "%s", "Flashing Page");

    if (cmdWrite(uart_num) != 1) {
        logE(TAG_STM_PRO, "Write command failed");
        return ESP_FAIL;
    }

    if (loadAddress(address[0], address[1], address[2], address[3], uart_num) != 1) {
        logE(TAG_STM_PRO, "Load address failed");
        return ESP_FAIL;
    }

    //ESP_LOG_BUFFER_HEXDUMP("FLASH PAGE", data, 256, ESP_LOG_DEBUG);

    char xor_ = 0xFF;
    char sz = 0xFF;

    sendData(TAG_STM_PRO, &sz, 1, uart_num);

    for (int i = 0; i < 256; i++)
    {
        sendData(TAG_STM_PRO, &data[i], 1, uart_num);
        xor_ ^= data[i];
    }

    sendData(TAG_STM_PRO, &xor_, 1, uart_num);

    int length = waitForSerialData(1, SERIAL_TIMEOUT, uart_num);
    if (length > 0)
    {
        uint8_t data[length];
        const int rxBytes = uart_read_bytes(uart_num, data, length, 1000 / portTICK_RATE_MS);
        if (rxBytes > 0 && data[0] == ACK)
        {
            logI(TAG_STM_PRO, "%s", "Flash Success");
            return ESP_OK;
        }
        else
        {
            logE(TAG_STM_PRO, "%s", "Flash Failure");
            return ESP_FAIL;
        }
    }
    else
    {
        logE(TAG_STM_PRO, "%s", "Serial Timeout");
    }
    return ESP_FAIL;
}

esp_err_t readPage(const char *address, const char *data, uart_port_t uart_num)
{
    logI(TAG_STM_PRO, "%s", "Reading page");
    char param[] = {0xFF, 0x00};

    cmdRead(uart_num);

    loadAddress(address[0], address[1], address[2], address[3], uart_num);

    sendData(TAG_STM_PRO, param, sizeof(param), uart_num);
    int length = waitForSerialData(257, SERIAL_TIMEOUT, uart_num);
    if (length > 0)
    {
        uint8_t uart_data[length];
        const int rxBytes = uart_read_bytes(uart_num, uart_data, length, 1000 / portTICK_RATE_MS);

        if (rxBytes > 0 && uart_data[0] == 0x79)
        {
            logI(TAG_STM_PRO, "%s", "Success");
            if (!memcpy((void *)data, uart_data, 257))
            {
                return ESP_FAIL;
            }

            //ESP_LOG_BUFFER_HEXDUMP("READ MEMORY", data, rxBytes, ESP_LOG_DEBUG);
        }
        else
        {
            logE(TAG_STM_PRO, "%s", "Failure");
            return ESP_FAIL;
        }
    }
    else
    {
        logE(TAG_STM_PRO, "%s", "Serial Timeout");
        return ESP_FAIL;
    }

    return ESP_OK;
}

stm32flash::FlashStatus isSTMPresent(gpio_num_t reset_pin, uart_port_t uart_num) {
    logI(TAG_STM_PRO, "Checking STM32 presence...");
    
    // Reset (STM should already be set in BOOT0 mode)
    resetSTM(reset_pin);
    
    // Essayer de synchroniser plusieurs fois
    for(int i = 0; i < 3; i++) {
        if(cmdSync(uart_num) == 1) {
            if(cmdGet(uart_num) == 1) {
                logI(TAG_STM_PRO, "STM32 detected in bootloader mode!");
                return stm32flash::SUCCESS;
            }
        }
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
    
    logE(TAG_STM_PRO, "No STM32 detected or not in bootloader mode!");
    return stm32flash::ERROR_STM_NOT_FOUND;
}

stm32flash::FlashStatus setFlashMode(gpio_num_t reset_pin, gpio_num_t boot0_pin, bool enter_flash_mode) {
    // Configure les pins en GPIO
    if (gpio_set_direction(reset_pin, GPIO_MODE_OUTPUT) != ESP_OK ||
    gpio_set_direction(boot0_pin, GPIO_MODE_OUTPUT) != ESP_OK) {
        logE(TAG_STM_PRO, "Failed to initialize GPIO");
        return stm32flash::ERROR_GPIO_INIT;
    }
    
    // Séquence d'entrée/sortie du mode flash
    gpio_set_level(boot0_pin, enter_flash_mode ? HIGH : LOW);
    resetSTM(reset_pin);

    logI(TAG_STM_PRO, "STM32 %s flash mode", enter_flash_mode ? "entered" : "exited");
    return stm32flash::SUCCESS;
}