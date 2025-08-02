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

// ADC attenuation types are defined in Arduino.h

#endif // UNIT_TESTING