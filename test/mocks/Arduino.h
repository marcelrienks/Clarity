#pragma once

/**
 * @file Arduino.h
 * @brief Mock Arduino framework header for native testing
 * 
 * @details This file provides mock implementations of Arduino framework
 * functions and constants needed by sensor classes during native testing.
 */

#ifdef UNIT_TESTING

#include <cstdint>

// Arduino pin mode constants
#ifndef INPUT
#define INPUT 0
#define OUTPUT 1
#define INPUT_PULLUP 2
#define INPUT_PULLDOWN 3
#endif

// Arduino digital values
#ifndef HIGH
#define HIGH 1
#define LOW 0
#endif

// Arduino interrupt modes
#ifndef RISING
#define RISING 1
#define FALLING 2
#define CHANGE 3
#endif

// Arduino ADC constants
#ifndef ADC_11db
#define ADC_11db 3
#endif

// Arduino function declarations for native testing
extern "C" {
    unsigned long millis();
    void delay(unsigned long ms);
    void analogReadResolution(int bits);
    void analogSetAttenuation(int attenuation);
}

// Mock ESP32 logging - provide the macros that sensors use
#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG

// ESP32 log levels
typedef enum {
    ESP_LOG_NONE,
    ESP_LOG_ERROR,
    ESP_LOG_WARN,
    ESP_LOG_INFO,
    ESP_LOG_DEBUG,
    ESP_LOG_VERBOSE
} esp_log_level_t;

extern "C" int esp_log_write(esp_log_level_t level, const char* tag, const char* format, ...);
extern "C" int esp_log_level_get(const char* tag);

// ESP32 log macros (used by esp32-hal-log.h)
#define ESP_LOGE(tag, format, ...) esp_log_write(ESP_LOG_ERROR, tag, format, ##__VA_ARGS__)
#define ESP_LOGW(tag, format, ...) esp_log_write(ESP_LOG_WARN, tag, format, ##__VA_ARGS__)
#define ESP_LOGI(tag, format, ...) esp_log_write(ESP_LOG_INFO, tag, format, ##__VA_ARGS__)
#define ESP_LOGD(tag, format, ...) esp_log_write(ESP_LOG_DEBUG, tag, format, ##__VA_ARGS__)
#define ESP_LOGV(tag, format, ...) esp_log_write(ESP_LOG_VERBOSE, tag, format, ##__VA_ARGS__)

// Common Arduino log macros (used by esp32-hal-log.h)
#define log_e(format, ...) ESP_LOGE("", format, ##__VA_ARGS__)
#define log_w(format, ...) ESP_LOGW("", format, ##__VA_ARGS__)
#define log_i(format, ...) ESP_LOGI("", format, ##__VA_ARGS__)
#define log_d(format, ...) ESP_LOGD("", format, ##__VA_ARGS__)
#define log_v(format, ...) ESP_LOGV("", format, ##__VA_ARGS__)

#endif // UNIT_TESTING