#include <Arduino.h>
#include <STM32Flasher.h>

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH);

    Serial.begin(9600); // Serial (USB CDC on ESP32S3) for debugging

    // Flasher configuration
    stm32flash::FlashConfig config = {
        .reset_pin = GPIO_NUM_5,
        .boot0_pin = GPIO_NUM_4,
        .uart_tx = GPIO_NUM_43,
        .uart_rx = GPIO_NUM_6,
        .uart_num = (uart_port_t)UART_NUM_1,
    };

    // Create the flasher with the config
    stm32flash::STM32Flasher flasher(config);

    Serial.println("Starting STM32 flash...");

    // Flash the STM32
    stm32flash::FlashStatus status = flasher.flash("blink1000.bin");
    if (status != stm32flash::SUCCESS) {
        Serial.printf("STM32 flash aborted with error: %s\n", stm32flash::toString(status));
    } else {
        Serial.println("STM32 flash completed!");
    }
}

void loop() {
    digitalWrite(LED_BUILTIN, LOW);
    delay(1000);
}
