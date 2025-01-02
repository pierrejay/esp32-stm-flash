#include "STM32Flasher.h"
#include "stm_flash.h"
#include "stm_pro_mode.h"

namespace stm32flash {

FlashStatus flash(const FlashConfig& config, const char* filename) {
    if (!config.isValid()) {
        return ERROR_CONFIG_INVALID;
    }

    // Everything is handled in flashSTM
    return internal::flashSTM(
        filename,
        config.reset_pin,
        config.boot0_pin,
        config.uart_tx,
        config.uart_rx,
        config.uart_num
    );
}

} // namespace stm32flash
