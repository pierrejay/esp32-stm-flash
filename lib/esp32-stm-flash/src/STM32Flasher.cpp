#include "STM32Flasher.h"
#include "stm_flash.h"
#include "stm_pro_mode.h"

namespace stm32flash {

STM32Flasher::STM32Flasher(const FlashConfig& config) : 
    config_(config) {}

FlashStatus STM32Flasher::flash(const char* filename) {
    if (!config_.isValid()) {
        return ERROR_CONFIG_INVALID;
    }

    // Everything is handled in flashSTM
    return flashSTM(
        filename,
        config_.reset_pin,
        config_.boot0_pin,
        config_.uart_tx,
        config_.uart_rx,
        config_.uart_num
    );
}

} // namespace stm32flash
