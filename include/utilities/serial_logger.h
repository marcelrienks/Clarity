#pragma once // preventing duplicate definitions, alternative to the traditional include guards

#include "utilities/ticker.h"

#include <Arduino.h>

class SerialLogger
{
public:
    static void init(unsigned long baud_rate = 115200);
    static void log(const std::string &message, bool force_print = false, unsigned long time_threshold = 5000);
    static void log_with_time(const std::string &message, bool force_print = false, unsigned long time_threshold = 5000);
    static void log_point(const std::string &point, const std::string &message, bool force_print = false, unsigned long time_threshold = 5000);
    static void log_value(const std::string &point, const std::string &variable_name, const std::string &value, bool force_print = false, unsigned long time_threshold = 5000);

private:
    static bool _is_initialized;
    static std::string _last_message;
    static uint32_t _duplicate_count;
    static unsigned long _last_log_time;
};