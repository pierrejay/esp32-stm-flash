#include <Arduino.h>
#include <STM32Flasher.h>

using namespace stm32flash;

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH); // Turn OFF LED

    Serial.begin(9600); // Serial (USB CDC on ESP32S3) for debugging

    // Configuration
    FlashConfig config = {
        .reset_pin = GPIO_NUM_5,
        .boot0_pin = GPIO_NUM_4,
        .uart_tx = GPIO_NUM_43,
        .uart_rx = GPIO_NUM_6,
        .uart_num = (uart_port_t)UART_NUM_1,
    };

    Serial.println("Starting STM32 flash...");

    // Flash the STM32
    FlashStatus status = flash(config, "blink1000.bin");
    if (status != SUCCESS) {
        Serial.printf("STM32 flash aborted with error: %s\n", toString(status));
    } else {
        Serial.println("STM32 flash completed!");
    }

    digitalWrite(LED_BUILTIN, LOW); // Turn ON LE
}

void loop() {

}
