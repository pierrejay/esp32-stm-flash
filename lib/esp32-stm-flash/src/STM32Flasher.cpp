#include "STM32Flasher.h"
#include "stm_flash.h"
#include "stm_pro_mode.h"

namespace stm32flash {

STM32Flasher::STM32Flasher(const FlashConfig& config) : 
    config_(config) {}

STM32Flasher::~STM32Flasher() {
    endConn(config_.reset_pin, config_.boot0_pin);
}

FlashStatus STM32Flasher::flash(const char* filename) {
    if (!config_.isValid()) {
        return ERROR_CONFIG_INVALID;
    }

    // 1. UART en premier (comme dans le main.cpp original)
    FlashStatus status = initFlashUART(
        config_.uart_num,
        config_.uart_tx, 
        config_.uart_rx
    );
    if (status != SUCCESS) return status;

    // 2. SPIFFS ensuite (comme dans le main.cpp original)
    status = initSPIFFS();
    if (status != SUCCESS) return status;

    // 3. Le reste est géré par flashSTM (GPIO, isSTMPresent, etc.)
    status = flashSTM(filename, config_.reset_pin, config_.boot0_pin, config_.uart_num);
    if (status != SUCCESS) return status;

    return SUCCESS;
}

} // namespace stm32flash
