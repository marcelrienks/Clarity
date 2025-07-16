#pragma once

#include <Arduino.h>

// Define custom log level for testing (level 1.5 - between ERROR and WARN)
#ifndef ARDUHAL_LOG_LEVEL_TEST
#define ARDUHAL_LOG_LEVEL_TEST 1
#endif

// Custom test logging function that works at any debug level
#if ARDUHAL_LOG_LEVEL >= ARDUHAL_LOG_LEVEL_TEST
#define log_t(format, ...) log_printf(ARDUHAL_LOG_FORMAT(T, format), ##__VA_ARGS__)
#else
#define log_t(format, ...) do {} while(0)
#endif

// Alternative: Force test logs to always appear
#ifdef WOKWI_EMULATOR
  #undef log_t
  #define log_t(format, ...) Serial.printf("[TEST] " format "\n", ##__VA_ARGS__)
#endif