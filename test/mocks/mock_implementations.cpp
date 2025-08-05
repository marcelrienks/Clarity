/**
 * @file mock_implementations.cpp
 * @brief Mock implementations of Arduino framework functions for native testing
 * 
 * @details This file provides mock implementations of Arduino framework functions
 * that are needed by sensor classes but not available in native testing environment.
 * Functions are implemented to provide realistic behavior for testing.
 */

#include <cstdint>
#include <chrono>
#include <cstdio>

// Mock Arduino constants
#ifndef HIGH
#define HIGH 1
#define LOW 0
#endif

#ifndef INPUT
#define INPUT 0
#define OUTPUT 1
#define INPUT_PULLUP 2
#define INPUT_PULLDOWN 3
#endif

#ifndef RISING
#define RISING 1
#define FALLING 2
#define CHANGE 3
#endif

#ifndef ADC_11db
#define ADC_11db 3
#endif

// Global time tracking for millis() simulation
static auto start_time = std::chrono::steady_clock::now();

/**
 * @brief Mock implementation of Arduino millis() function
 * @return Milliseconds since program start
 */
extern "C" unsigned long millis() {
    auto now = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time);
    return static_cast<unsigned long>(duration.count());
}

/**
 * @brief Mock implementation of Arduino delay() function
 * @param ms Milliseconds to delay
 */
extern "C" void delay(unsigned long ms) {
    auto start = std::chrono::steady_clock::now();
    while (true) {
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start);
        if (elapsed.count() >= static_cast<long>(ms)) {
            break;
        }
    }
}

/**
 * @brief Mock implementation of analogReadResolution()
 * @param bits ADC resolution in bits (typically 12 for ESP32)
 */
extern "C" void analogReadResolution(int bits) {
    // Mock implementation - just ignore the call
    // Real implementation would configure ADC hardware
    (void)bits; // Suppress unused parameter warning
}

/**
 * @brief Mock implementation of analogSetAttenuation()
 * @param attenuation ADC attenuation setting
 */
extern "C" void analogSetAttenuation(int attenuation) {
    // Mock implementation - just ignore the call
    // Real implementation would set ADC voltage range
    (void)attenuation; // Suppress unused parameter warning
}

#include <cstdarg>

// Mock ESP32 logging functions
extern "C" int esp_log_write(int level, const char* tag, const char* format, ...) {
    // Simple implementation that writes to stdout
    va_list args;
    va_start(args, format);
    
    // Print log level prefix
    const char* level_str = "INFO";
    switch (level) {
        case 1: level_str = "ERROR"; break;
        case 2: level_str = "WARN"; break;
        case 3: level_str = "INFO"; break;
        case 4: level_str = "DEBUG"; break;
        case 5: level_str = "VERBOSE"; break;
    }
    
    printf("[%s] %s: ", level_str, tag);
    int ret = vprintf(format, args);
    printf("\n");
    
    va_end(args);
    return ret;
}

// Mock log level check function
extern "C" int esp_log_level_get(const char* tag) {
    (void)tag;
    return 5; // Return VERBOSE level for all tags in testing
}

// Define log macros that are used by sensor implementations
#define ESP_LOGE(tag, format, ...) esp_log_write(1, tag, format, ##__VA_ARGS__)
#define ESP_LOGW(tag, format, ...) esp_log_write(2, tag, format, ##__VA_ARGS__)
#define ESP_LOGI(tag, format, ...) esp_log_write(3, tag, format, ##__VA_ARGS__)
#define ESP_LOGD(tag, format, ...) esp_log_write(4, tag, format, ##__VA_ARGS__)
#define ESP_LOGV(tag, format, ...) esp_log_write(5, tag, format, ##__VA_ARGS__)

// Mock putchar for Unity output
extern "C" int putchar(int c) {
    return std::putchar(c);
}

// Mock fflush for Unity output
extern "C" int fflush(FILE* stream) {
    return std::fflush(stream);
}