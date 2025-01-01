#ifndef _STM_FLASH_H
#define _STM_FLASH_H

#include "stm_pro_mode.h"

/**
 * @brief Write the code into the flash memory of STM32Fxx
 * 
 * The data from the .bin file is written into the flash memory 
 * of the client, block-by-block 
 * 
 * @param flash_file File pointer of the .bin file to be flashed
 *   
 * @return ESP_OK - success, ESP_FAIL - failed
 */
stm32flash::FlashStatus writeTask(FILE *flash_file, gpio_num_t reset_pin, gpio_num_t boot0_pin, uart_port_t uart_num);

/**
 * @brief Read the flash memory of the STM32Fxx, for verification
 * 
 * It reads the flash memory of the STM32 block-by-block and 
 * checks it with the data from the file (with pointer passed)
 * 
 * @param flash_file File pointer of the .bin file to be verified against
 *   
 * @return ESP_OK - success, ESP_FAIL - failed
 */
stm32flash::FlashStatus readTask(FILE *flash_file, uart_port_t uart_num);

/**
 * @brief Flash the .bin file passed, to STM32Fxx, with read verification
 * 
 * @param file_name name of the .bin to be flashed
 *   
 * @return ESP_OK - success, ESP_FAIL - failed
 */
stm32flash::FlashStatus flashSTM(const char *file_name, gpio_num_t reset_pin, gpio_num_t boot0_pin, uart_port_t uart_num);

#endif