#include "utilities/serial_logger.h"

/// @brief Logs a message that is supplied verbatim
/// @param message The message to be logged
void SerialLogger::init()
{
#ifdef CLARITY_DEBUG
  Serial.begin(115200);
#endif
}

/// @brief Logs a message and prefixes it with the point in code that is supplied. e.g. "DemoSensor::get_reading() -> Busy getting reading"
void SerialLogger::log_message(const String &message)
{
#ifdef CLARITY_DEBUG
  char log[150];
  char elapsed[5];

  sprintf(elapsed, "%04d", Ticker::get_elapsed_millis() % 10000);
  snprintf(log, sizeof(log), "%sms:  %.90s", elapsed, message.c_str());  // Limits message to 90 chars

  Serial.println(log);
#endif
}

/// @brief Logs a message and prefixes it with the point in code that is supplied. e.g. "DemoSensor::get_reading() -> Busy getting reading"
/// @param point
/// @param message
void SerialLogger::log_point(const String &point, const String &message)
{
#ifdef CLARITY_DEBUG
  char log[150];
  char elapsed[5];

  sprintf(elapsed, "%04d", Ticker::get_elapsed_millis() % 10000);
  snprintf(log, sizeof(log), "%sms:  %s -> %.90s", elapsed, point.c_str(), message.c_str());  // Limits message to 90 chars

  Serial.println(log);
#endif
}

/// @brief Logs a variable and prefixes it with the point in code that is supplied, and post fixes it with the value supplied. e.g. "DemoSensor::get_reading() -> currentReading is = 42"
/// @param point
/// @param variable_name
/// @param value
void SerialLogger::log_value(const String &point, const String &variable_name, const String &value)
{
#ifdef CLARITY_DEBUG
  char log[150];
  char elapsed[5];

  sprintf(elapsed, "%04d", Ticker::get_elapsed_millis() % 10000);
  snprintf(log, sizeof(log), "%sms:  %s -> %s is = %.90s", elapsed, point.c_str(), variable_name.c_str(), value.c_str());  // Limits message to 90 chars

  Serial.println(log);
#endif
}

/// @brief Logs an exception and prefixes it with the point in code that is supplied. e.g. "DemoSensor::get_reading() -> EXCEPTION - Busy getting reading"
/// @param point
/// @param message
void SerialLogger::log_exception(const String &point, const String &exception)
{
#ifdef CLARITY_DEBUG
  char log[150];
  char elapsed[5];

  sprintf(elapsed, "%04d", Ticker::get_elapsed_millis() % 10000);
  snprintf(log, sizeof(log), "%sms:  %s -> EXCEPTION - %.90s", elapsed, point.c_str(), exception.c_str());  // Limits message to 90 chars

  Serial.println(log);
#endif
}