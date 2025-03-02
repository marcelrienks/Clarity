#ifndef SERIAL_LOGGER_H
#define SERIAL_LOGGER_H

#include "utilities/ticker.h"

#include <Arduino.h>

class SerialLogger
{
private:
    static bool _initialized;
    static std::string _last_message;
    static uint32_t _duplicate_count;
    static unsigned long _last_log_time;

public:
    static void init(unsigned long baud_rate = 115200);
    static void log(const String &message, bool force_print = false, unsigned long time_threshold = 5000);
    static void log_with_time(const String &message, bool force_print = false, unsigned long time_threshold = 5000);
    static void log_point(const String &point, const String &message, bool force_print = false, unsigned long time_threshold = 5000);
    static void log_value(const String &point, const String &variable_name, const String &value, bool force_print = false, unsigned long time_threshold = 5000);
    static void flush();
};

#endif // SERIAL_LOGGER_H