#include "utilities/serial_logger.h"

//TODO: investigate log_e() and all other ESP32 logging, and potentially replace all below with that
// The ESP32 Arduino core provides several logging macros for different severity levels:

// log_e(format, ...) - Error (highest severity)
// log_w(format, ...) - Warning
// log_i(format, ...) - Information
// log_d(format, ...) - Debug
// log_v(format, ...) - Verbose (lowest severity)

// // In Arduino IDE:
// esp_log_level_set("*", ESP_LOG_VERBOSE);  // Show all log messages
// esp_log_level_set("*", ESP_LOG_ERROR);    // Show only error messages

// Define static class members here
bool SerialLogger::_is_initialized = false;
std::string SerialLogger::_last_message = "";
uint32_t SerialLogger::_duplicate_count = 0;
unsigned long SerialLogger::_last_log_time = 0;

/// @brief Initialize the serial logger
/// @param baud_rate The baud rate to use (default: 115200)
void SerialLogger::init(unsigned long baud_rate)
{
#ifdef CLARITY_DEBUG
  if (!_is_initialized)
  {
    Serial.begin(115200);
    _is_initialized = true;
  }
#endif
}

/// @brief Log a message with deduplication
/// @param message the message to log
/// @param force_print force printing even if it's a duplicate (default: false)
/// @param time_threshold minimum time between duplicate summaries in ms (default: 5000)
void SerialLogger::log(const std::string &message, bool force_print, unsigned long time_threshold)
{
  std::string msg_str = message.c_str();
  unsigned long current_time = millis();

  // If it's a new message or force print is enabled
  if (msg_str != _last_message || force_print)
  {
    // Print duplicate summary if we had duplicates
    if (_duplicate_count > 0)
    {
      Serial.print("Last message repeated ");
      Serial.print(_duplicate_count);
      Serial.println(" times");
      _duplicate_count = 0;
    }

    // Print the new message
    Serial.println(message.c_str());
    _last_message = msg_str;
    _last_log_time = current_time;
  }
  // If it's a duplicate
  else
  {
    _duplicate_count++;

    // Periodically print duplicate count for long-running duplicates
    if (current_time - _last_log_time >= time_threshold)
    {
      Serial.print("Message repeated ");
      Serial.print(_duplicate_count);
      Serial.println(" times so far...");
      _last_log_time = current_time;
      // Don't reset duplicate_count here, we'll include these in the final count
    }
  }
}

/// @brief Log a message with timestamp and deduplication
/// @param message the message to log
/// @param force_print force printing even if it's a duplicate (default: false)
/// @param time_threshold minimum time between duplicate summaries in ms (default: 5000)
void SerialLogger::log_with_time(const std::string &message, bool force_print, unsigned long time_threshold)
{
  char timestamp[16];
  snprintf(timestamp, sizeof(timestamp), "[%lu ms] ", millis());
  std::string full_message = timestamp + message;
  log(full_message, force_print, time_threshold);
}

 /// @brief Log a message with point and deduplication
 /// @param point The point in code
  /// @param message The message to log
  /// @param force_print Force printing even if it's a duplicate (default: false)
  /// @param time_threshold Minimum time between duplicate summaries in ms (default: 5000)
void SerialLogger::log_point(const std::string &point, const std::string &message, bool force_print, unsigned long time_threshold)
{
  std::string full_message = point + " -> " + message;
  log(full_message, force_print, time_threshold);
}

/// @brief Log a variable value with point and deduplication
/// @param point the point in code
/// @param variable_name the name of the variable
/// @param value the value of the variable
/// @param force_print force printing even if it's a duplicate (default: false)
/// @param time_threshold minimum time between duplicate summaries in ms (default: 5000)
void SerialLogger::log_value(const std::string &point, const std::string &variable_name, const std::string &value, bool force_print, unsigned long time_threshold)
{
  std::string full_message = point + " -> " + variable_name + " = " + value;
  log(full_message, force_print, time_threshold);
}