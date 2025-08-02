#pragma once

// Mock ESP32 logging for unit testing
#ifdef UNIT_TESTING

#include <cstdio>

// Mock logging macros
#define log_v(format, ...) printf("[V] " format "\n", ##__VA_ARGS__)
#define log_d(format, ...) printf("[D] " format "\n", ##__VA_ARGS__)
#define log_i(format, ...) printf("[I] " format "\n", ##__VA_ARGS__)
#define log_w(format, ...) printf("[W] " format "\n", ##__VA_ARGS__)
#define log_e(format, ...) printf("[E] " format "\n", ##__VA_ARGS__)

// ADC attenuation types (ESP32 specific)
typedef enum {
    ADC_ATTEN_DB_0   = 0,
    ADC_ATTEN_DB_2_5 = 1, 
    ADC_ATTEN_DB_6   = 2,
    ADC_ATTEN_DB_11  = 3
} adc_attenuation_t;

#endif // UNIT_TESTING