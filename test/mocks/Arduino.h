#pragma once

// Mock Arduino definitions for unit testing
#ifdef UNIT_TESTING

#include <stdint.h>
#include <cstddef>

// Basic Arduino constants
#define HIGH 1
#define LOW  0

#define INPUT             0x0
#define OUTPUT            0x1
#define INPUT_PULLUP      0x2
#define INPUT_PULLDOWN    0x3

// ADC pin constants
#define A0  0

// ADC attenuation (ESP32 specific)
typedef enum {
    ADC_0db,
    ADC_2_5db,
    ADC_6db,
    ADC_11db
} adc_attenuation_t;

// Mock Arduino functions
inline uint32_t millis() { return 0; }
inline void delay(uint32_t ms) {}
inline void analogReadResolution(uint8_t bits) {}
inline void analogSetAttenuation(adc_attenuation_t attenuation) {}
inline int digitalRead(uint8_t pin) { return LOW; }
inline int analogRead(uint8_t pin) { return 0; }
inline void pinMode(uint8_t pin, uint8_t mode) {}

// ESP32 return types
typedef int esp_err_t;
#define ESP_OK 0

#endif // UNIT_TESTING