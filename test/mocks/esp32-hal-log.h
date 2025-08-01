#pragma once

// Mock ESP32 logging for unit testing
#ifdef UNIT_TESTING

// Mock logging macros
#define log_d(format, ...) do {} while(0)
#define log_i(format, ...) do {} while(0)
#define log_w(format, ...) do {} while(0)
#define log_e(format, ...) do {} while(0)

#endif // UNIT_TESTING