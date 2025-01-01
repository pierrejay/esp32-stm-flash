#include <Arduino.h>
#include <STM32Flasher.h>

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);

    Serial.begin(9600); // USB for debugging
    
    // Configuration du flasher
    stm32flash::FlashConfig config = {
        .reset_pin = GPIO_NUM_5,
        .boot0_pin = GPIO_NUM_43,
        .uart_tx = GPIO_NUM_43,
        .uart_rx = GPIO_NUM_6,
        .uart_num = (uart_port_t)UART_NUM_1,
    };

    // Création du flasher avec la config
    stm32flash::STM32Flasher flasher(config);

    // Blink LED pendant l'init
    for (int i = 0; i < 10; i++) {
        digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
        delay(200);
    }

    Serial.println("Starting STM32 flash...");

    // Flash du STM32
    stm32flash::FlashStatus status = flasher.flash("blink50.bin");
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
