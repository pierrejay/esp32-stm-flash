#ifndef _STM_PRO_MODE_H
#define _STM_PRO_MODE_H

#include "STM32Flasher.h"  // Pour avoir accès à FlashStatus

#include "logger.h"

#include <stdio.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>

#include <sys/unistd.h>
#include <sys/stat.h>
#include <sys/param.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "driver/uart.h"
#include "driver/gpio.h"

#include "esp_err.h"
#include "esp_vfs.h"
#include "esp_system.h"
#include "esp_spiffs.h"

#include "esp_event.h"
#include "esp_wifi.h"
#include "esp_http_server.h"

#include "nvs_flash.h"

//Macro for error checking
#define IS_ESP_OK(x) if ((x) != ESP_OK) break;

#define TXD_PIN (GPIO_NUM_43) 
#define RXD_PIN (GPIO_NUM_6) 
#define UART_BAUD_RATE 115200
#define UART_BUF_SIZE 1024
#define UART_CONTROLLER UART_NUM_1

#define RESET_PIN (GPIO_NUM_5) 
#define BOOT0_PIN (GPIO_NUM_4) 
// Already defined in Arduino.h
// #define HIGH 1
// #define LOW 0

#define ACK 0x79
#define SERIAL_TIMEOUT 5000

#define FILE_PATH_MAX 128
#define BASE_PATH "/spiffs/"
#define MAX_FLASH_SIZE 32768 // 32KB

//Initialize UART functionalities
stm32flash::FlashStatus initFlashUART(uart_port_t uart_num, gpio_num_t tx, gpio_num_t rx);

//Initialize SPIFFS functionalities
stm32flash::FlashStatus initSPIFFS(void);

//Increment the memory address for the next write operation
void incrementLoadAddress(char *loadAddr);

//Get in sync with STM32Fxx
int cmdSync(uart_port_t uart_num);

//Get the version and the allowed commands supported by the current version of the bootloader
int cmdGet(uart_port_t uart_num);

//Get the bootloader version and the Read Protection status of the Flash memory
int cmdVersion(uart_port_t uart_num);

//Get the chip ID
int cmdId(uart_port_t uart_num);

//Erase from one to all the Flash memory pages
int cmdErase(uart_port_t uart_num);

//Erases from one to all the Flash memory pages using 2-byte addressing mode
int cmdExtErase(uart_port_t uart_num);

//Setup STM32Fxx for the 'flashing' process
stm32flash::FlashStatus setupSTM(gpio_num_t reset_pin, uart_port_t uart_num);

//Write data to flash memory address
int cmdWrite(uart_port_t uart_num);

//Read data from flash memory address
int cmdRead(uart_port_t uart_num);

//UART send data to STM32Fxx & wait for response
int sendBytes(const char *bytes, int count, int resp, uart_port_t uart_num);

//UART send data byte-by-byte to STM32Fxx
int sendData(const char *logName, const char *data, const int count, uart_port_t uart_num);

//Wait for response from STM32Fxx
int waitForSerialData(int dataCount, int timeout, uart_port_t uart_num);

//Send the STM32Fxx the memory address, to be written
int loadAddress(const char adrMS, const char adrMI, const char adrLI, const char adrLS, uart_port_t uart_num);

//UART write the flash memory address of the STM32Fxx with blocks of data 
esp_err_t flashPage(const char *address, const char *data, uart_port_t uart_num);

//UART read the flash memory address of the STM32Fxx and verify with the given block of data 
esp_err_t readPage(const char *address, const char *data, uart_port_t uart_num);

//Check if STM32 is present and in bootloader mode
stm32flash::FlashStatus isSTMPresent(gpio_num_t reset_pin, uart_port_t uart_num);

//Nouvelle fonction pour gérer l'état du STM32
stm32flash::FlashStatus setFlashMode(gpio_num_t reset_pin, gpio_num_t boot0_pin, bool enter_flash_mode);

#endif