#ifndef STM32_FLASHER_H
#define STM32_FLASHER_H

#include <Arduino.h>
#include "driver/uart.h" 
#include "driver/gpio.h"

namespace stm32flash {

/**
 * @brief Configuration structure for the flasher
 */
struct FlashConfig {
    
    // Control pins
    gpio_num_t reset_pin = GPIO_NUM_NC;
    gpio_num_t boot0_pin = GPIO_NUM_NC;

    // UART configuration (ESP32 side)
    gpio_num_t uart_tx = GPIO_NUM_NC; // ESP32 TX <-> STM32 RX
    gpio_num_t uart_rx = GPIO_NUM_NC; // ESP32 RX <-> STM32 TX
    uart_port_t uart_num = UART_NUM_MAX;

    bool isValid() const {
        return (uart_tx != GPIO_NUM_NC &&
                uart_rx != GPIO_NUM_NC &&
                reset_pin != GPIO_NUM_NC &&
                boot0_pin != GPIO_NUM_NC &&
                uart_tx != uart_rx && 
                reset_pin != boot0_pin &&
                uart_num != UART_NUM_MAX);
    }
};


/**
 * @brief Enumeration of flash operation status
 */
enum FlashStatus {
    SUCCESS = 0,
    
    // Initialization errors
    ERROR_CONFIG_INVALID,
    ERROR_UART_INIT,
    ERROR_GPIO_INIT,
    ERROR_SPIFFS_INIT,
    
    // STM32 communication errors
    ERROR_STM_NOT_FOUND,
    ERROR_STM_SYNC_FAILED,
    ERROR_STM_GET_COMMANDS_FAILED,
    ERROR_STM_GET_VERSION_FAILED,
    ERROR_STM_GET_ID_FAILED,
    
    // Flash errors
    ERROR_FILE_NOT_FOUND,
    ERROR_FILE_EMPTY,
    ERROR_FILE_TOO_LARGE,
    ERROR_CANNOT_OPEN_FILE,
    ERROR_ERASE_FAILED,
    ERROR_EXT_ERASE_FAILED,
    ERROR_WRITE_FAILED,
    ERROR_READ_FAILED,
    
    // Other errors
    ERROR_UNKNOWN
};

/**
 * @brief Convert a FlashStatus to a string
 * @param status FlashStatus to convert
 * @return String representation of the status
 */
static constexpr const char* toString(FlashStatus status) {
    switch(status) {
        case SUCCESS:              return "success";
            
        // Initialization errors
        case ERROR_CONFIG_INVALID: return "invalid_configuration";
        case ERROR_UART_INIT:      return "uart_initialization_failed";
        case ERROR_GPIO_INIT:      return "gpio_initialization_failed";
        case ERROR_SPIFFS_INIT:    return "spiffs_initialization_failed";
        
        // STM32 communication errors
        case ERROR_STM_NOT_FOUND:         return "stm32_not_detected";
        case ERROR_STM_SYNC_FAILED:       return "failed_to_synchronize_with_stm32";
        case ERROR_STM_GET_COMMANDS_FAILED: return "failed_to_get_commands_from_stm32";
        case ERROR_STM_GET_ID_FAILED:     return "failed_to_get_stm32_chip_id";
        case ERROR_STM_GET_VERSION_FAILED: return "failed_to_get_bootloader_version";
        
        // Flash errors
        case ERROR_FILE_NOT_FOUND:  return "file_not_found";
        case ERROR_FILE_EMPTY:      return "file_empty";
        case ERROR_FILE_TOO_LARGE:  return "file_too_large_for_flash_memory";
        case ERROR_CANNOT_OPEN_FILE: return "cannot_open_file";
        case ERROR_ERASE_FAILED:    return "flash_erase_failed";
        case ERROR_EXT_ERASE_FAILED: return "flash_extended_erase_failed";
        case ERROR_WRITE_FAILED:    return "flash_write_failed";
        case ERROR_READ_FAILED:     return "flash_read_failed";
        
        // Other errors
        case ERROR_UNKNOWN:
        default:                    return "unknown_error";
        }
}

class STM32Flasher {
public:

    /**
     * @brief Constructor with custom configuration
     * @param config Flasher configuration
     */
    explicit STM32Flasher(const FlashConfig& config);

    /**
     * @brief Flash STM32 with binary file
     * @param filename Name of the binary file to flash
     * @return FlashStatus indicating success or specific error
     */
    FlashStatus flash(const char* filename);

    // Disable copy constructor and assignment operator
    STM32Flasher(const STM32Flasher&) = delete;
    STM32Flasher& operator=(const STM32Flasher&) = delete;

private:
    FlashConfig config_;         // Configuration
};

} // namespace stm32flash

#endif // STM32_FLASHER_H