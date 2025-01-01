# ESP32-STM-Flash Library Documentation

## Context

In many applications, an ESP32 (main controller with WiFi/BLE capabilities) needs to work alongside an STM32 microcontroller dedicated to specific tasks (motor control, sensor acquisition, etc.). While the ESP32 can be easily programmed over-the-air or via USB, programming the STM32 traditionally requires an ST-Link programmer and access to specific debug pins (SWDIO/SWCLK).

This library enables in-situ programming of the STM32 directly from the ESP32 thanks to the STMicro USART bootloader protocol, using only UART lines and a couple of control signals (`BOOT0`, `RESET`). This approach eliminates the need for:
- External programmer hardware
- Debug pins exposure on the PCB
- Complex toolchain setup

The STM32 firmware (as a .bin file) is stored in the ESP32's flash memory and can be programmed at will. Once programmed, the same UART lines can be used for normal communication between both MCUs.

This approach has been successfully tested with the low-end `STM32F030` and `STM32G030` microcontroller series.

## Design 

The library is based on a C++ reimplementation of [OTA_update_STM32_using_ESP32](https://github.com/ESP32-Musings/OTA_update_STM32_using_ESP32) by ESP32-Musings. While the core protocol implementation remains similar, several improvements have been made:

- Complete C++ encapsulation with a simple public API
- Comprehensive error handling with specific error codes
- Streamlined execution flow
- Enhanced status reporting
- Pin configuration sequence optimized for shared BOOT0/UART pins

The main class `STM32Flasher` provides a simple interface:
- Configure pins and UART parameters via `FlashConfig` structure
- Single `flash()` method that handles the entire process
- Status reporting via `FlashStatus` error codes

The original logging system has been preserved and enhanced to provide detailed operation feedback.

## Setup

### Basic wiring
```
STM32 nRESET <-> GPIO ESP32
 STM32 BOOT0 <-> GPIO ESP32
    STM32 TX <-> RX ESP32
    STM32 RX <-> TX ESP32
```

### Pin Optimization Trick

A key optimization in this library is the ability to share `BOOT0` with one of the UART pins, reducing the total pin count from 4 to 3. This is possible because:

- `BOOT0` is only sampled by the STM32 during reset so we don't care about its level during the flashing process
- `BOOT0` pin has an internal ~45kΩ pull-up resistor so it will not pollute UART levels
- UART lines have well-defined idle states:
   - UART line is HIGH when idle
   - Stop bits are HIGH
   - Built-in pull-up resistors on ESP32 UART pins

The library handles the pin mode switching internally:
1. Before entering flash mode, drive the `BOOT0` pin to HIGH to ensure the STM32 is in bootloader mode
2. Enable the UART driver just before starting the flash process which will override `BOOT0` level
3. When leaving flash mode, reset both control pins and set `BOOT0` to LOW to ensure the STM32 will boot from flash

Be careful especially when using a dev board as some have "hidden" pull-up or series resistor on some pins that might interfere with the `BOOT0` level. For example, the default TX pin of the Seeeduino XIAO ESP32S3 cannot be shared with `BOOT0`, you will have to manually put `BOOT0` to GND after flash completion to boot the STM32 from flash.


## Usage

### Implementation

This repo is a PlatformIO project that can be used as a template for your own projects. Library files are located in `lib/esp32-stm-flash/src/`. 

```cpp
// 1. Define the configuration structure with your pin setup
// (uart_rx and uart_tx refer to ESP32 side)
stm32flash::FlashConfig config = {
    .reset_pin = GPIO_NUM_5,
    .boot0_pin = GPIO_NUM_4, // Can be mapped to RX or TX pin
    .uart_tx = GPIO_NUM_43,
    .uart_rx = GPIO_NUM_6,
    .uart_num = (uart_port_t)UART_NUM_1
};

// 2. Create the flasher object with the config
stm32flash::STM32Flasher flasher(config);

// 3. Start the flashing process
stm32flash::FlashStatus status = flasher.flash("firmware.bin");

// 4. Handle the status
// (refer to STM32Flasher.h for the full list of error codes)
if (status != stm32flash::SUCCESS) {
    Serial.printf("Flashing failed: %s\n", stm32flash::toString(status));
}

// 5. All pins are reset, you can now use the UART lines for normal communication with the STM32 (the pin setup needs to be redefined after flash).
```
Note: the `STM32Flasher` constructor returns a `FlashStatus` that can be catched after initialization to be sure the config is valid. In case of error, the constructor will return `ERROR_CONFIG_INVALID`.

### UART Configuration

The library handles the UART configuration internally. For users used to the Arduino syntax, the UART number corresponds to the Serial port:
- `UART_NUM_1` corresponds to `Serial1`
- `UART_NUM_2` corresponds to `Serial2`

The library configures the UART according to the ST protocol requirements:
- 115200 baud rate
- 8 data bits
- Even parity
- 1 stop bit

### Binary File Management

The STM32 binary must be stored in ESP32's flash memory. Using PlatformIO:
1. Place the .bin file in the `data/` directory
2. Use `Upload Filesystem Image` to write it to SPIFFS
3. The library will read it using the provided filename

Alternatively, you can implement your own file storage method - the library only requires a valid filename pointing to a binary in the mounted filesystem.

## Credits

This library is a C++ adaptation of [OTA_update_STM32_using_ESP32](https://github.com/ESP32-Musings/OTA_update_STM32_using_ESP32), enhanced with stronger error handling, pin optimization features and a more robust execution flow.

## License

MIT License.
