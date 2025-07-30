#pragma once

// Mock ESP32 logging for unit tests
// This replaces the actual ESP32 logging functions with no-op macros

#define log_d(format, ...) do {} while(0)
#define log_i(format, ...) do {} while(0)
#define log_w(format, ...) do {} while(0)
#define log_e(format, ...) do {} while(0)
#define log_v(format, ...) do {} while(0)

// Log levels
#define ESP_LOG_NONE    0
#define ESP_LOG_ERROR   1
#define ESP_LOG_WARN    2
#define ESP_LOG_INFO    3
#define ESP_LOG_DEBUG   4
#define ESP_LOG_VERBOSE 5