#pragma once

/**
 * @file logging.h
 * @brief Extended logging utilities for the Clarity automotive gauge system
 * 
 * @details This header provides additional logging macros that extend the 
 * ESP32's built-in logging system (log_v, log_d, log_i, log_w, log_e).
 * 
 * Additional log levels:
 * - log_t: Timing/performance logs with [T] prefix
 * 
 * @usage:
 * log_t("Operation completed in %d ms", duration);
 * 
 * @note Follows project logging standards from docs/patterns.md
 */

// Include ESP32 logging system
#if defined(ARDUINO_ARCH_ESP32)
    #include <esp32-hal-log.h>
#else
    // Fallback for non-ESP32 builds (testing/simulation)
    #include <stdio.h>
    #define log_i(format, ...) printf("[I] " format "\n", ##__VA_ARGS__)
#endif

//=============================================================================
// TIMING/PERFORMANCE LOGGING
// Special log level for timing, performance metrics, and optimization tracking
//=============================================================================

/**
 * @brief Timing/performance log with [T] prefix
 * 
 * @details Special logging level for timing measurements, performance metrics,
 * and optimization tracking. In TEST_LOGS_ONLY mode, only these logs are shown.
 * In normal mode, acts like log_i() with [T] prefix.
 * 
 * @usage_examples:
 * - Panel load timing: log_t("Panel load completed in %d ms", duration);
 * - Sensor read timing: log_t("Sensor read cycle: %d ms", cycle_time);
 * - Memory usage: log_t("Free heap: %d bytes", ESP.getFreeHeap());
 * - Animation performance: log_t("Frame render time: %d ms", frame_time);
 * 
 * @note Can be filtered in serial output by searching for "[T]" prefix
 * @note In TEST_LOGS_ONLY mode, this is the ONLY output shown
 * @note Output format: [T] message (not [I][T] message)
 */
#ifdef TEST_LOGS_ONLY
    // In test mode, log_t is the only output
    #define log_t(format, ...) printf("[T] " format "\n", ##__VA_ARGS__)
#else
    // In normal mode, log_t outputs with [T] prefix
    #define log_t(format, ...) printf("[T] " format "\n", ##__VA_ARGS__)
#endif

// Test-only logging mode - suppress all logs except log_t
#ifdef TEST_LOGS_ONLY
    #undef log_e
    #undef log_w
    #undef log_i
    #undef log_d
    #undef log_v
    #define log_e(format, ...) ((void)0)
    #define log_w(format, ...) ((void)0)
    #define log_i(format, ...) ((void)0)
    #define log_d(format, ...) ((void)0)
    #define log_v(format, ...) ((void)0)
#endif

// Option 2: With ESP32-style timestamp (uncomment to use instead)
// #define log_t(format, ...) printf("[%7u][T] " format "\n", (unsigned)millis(), ##__VA_ARGS__)

//=============================================================================
// USAGE EXAMPLES
//=============================================================================

/*
 * Basic usage:
 *   log_t("Panel loaded successfully");
 *   log_t("Operation completed in %d ms", duration);
 *   log_t("Free heap: %d bytes", ESP.getFreeHeap());
 * 
 * Output format:
 *   [T] Panel loaded successfully
 *   [T] Operation completed in 150 ms
 *   [T] Free heap: 180432 bytes
 *
 * With timestamp option enabled:
 *   [   5432][T] Panel loaded successfully  
 *   [   5582][T] Operation completed in 150 ms
 */