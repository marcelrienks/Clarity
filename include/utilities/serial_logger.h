#ifndef SERIAL_LOGGER_H
#define SERIAL_LOGGER_H

#include "utilities/ticker.h"

#include <Arduino.h>

class SerialLogger
{
public:
    static void init();
    static void log_message(const String &logMessage);
    static void log_point(const String &point, const String &message);
    static void log_value(const String &point, const String &variable_name, const String &value);
    static void log_exception(const String &point, const String &exception);
};

#endif // SERIAL_LOGGER_H