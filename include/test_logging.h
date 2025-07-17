#pragma once

#include <Arduino.h>

// Custom test logging that always outputs regardless of CORE_DEBUG_LEVEL
// Use log_t() for test assertions and checkpoints that integration tests wait for

#ifdef WOKWI_EMULATOR
  // Always log test messages in Wokwi emulator (for integration tests)
  #define log_t(format, ...) do { \
    Serial.print("\n[TEST] "); \
    Serial.printf(format, ##__VA_ARGS__); \
    Serial.print("\n"); \
    Serial.flush(); \
    delay(1); \
  } while(0)
#else
  // In normal builds, test logs are disabled
  #define log_t(format, ...) do {} while(0)
#endif

// Alternative: Use a dedicated test logger function
inline void test_log(const char* message) {
#ifdef WOKWI_EMULATOR
  Serial.printf("[TEST] %s\n", message);
#endif
}