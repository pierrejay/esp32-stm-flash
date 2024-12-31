#include <Arduino.h>
#include "../lib/stm_flash/stm_flash.h"

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);

  Serial.begin(9600); // USB for debugging

  // Initialisation
  initFlashUART();
  // initGPIO();  // Supprimé car géré dans flashSTM()
  initSPIFFS();
  
  // Blink LED 
  for (int i = 0; i < 10; i++) {
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    delay(200);
  }
}

void loop() {
  Serial.println("Starting STM32 flash...");

  esp_err_t err = flashSTM("blink.bin");  // Cette fonction gère maintenant tout le processus
  if (err != ESP_OK) {
    Serial.printf("STM32 flash aborted with error");
  } else {
    Serial.println("STM32 flash completed!");
  }
  // endConn() est maintenant appelé automatiquement dans flashSTM()

  while(true) {
    digitalWrite(LED_BUILTIN, LOW);
  }
}
