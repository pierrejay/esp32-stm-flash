#ifndef _LOGGER_H
#define _LOGGER_H

#include <Arduino.h>

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "esp_log.h"

namespace stm32flash {
namespace internal {

#define LOG_BUFFER_SIZE 512
#define LOG_FILE_PATH "/spiffs/log.txt"

// Macros pour faciliter l'utilisation du logger
#define logE(tag, format, ...) logger(ESP_LOG_ERROR, tag, __LINE__, __FUNCTION__, format, ##__VA_ARGS__)
#define logW(tag, format, ...) logger(ESP_LOG_WARN, tag, __LINE__, __FUNCTION__, format, ##__VA_ARGS__)
#define logI(tag, format, ...) logger(ESP_LOG_INFO, tag, __LINE__, __FUNCTION__, format, ##__VA_ARGS__)
#define logD(tag, format, ...) logger(ESP_LOG_DEBUG, tag, __LINE__, __FUNCTION__, format, ##__VA_ARGS__)
#define logV(tag, format, ...) logger(ESP_LOG_VERBOSE, tag, __LINE__, __FUNCTION__, format, ##__VA_ARGS__)

void logger(esp_log_level_t level, const char *TAG, int line, const char *func, const char *fmt, ...);
bool setLogToFile(void);
bool isLoggingToFileEnabled(void);

} // namespace internal
} // namespace stm32flash   

#endif