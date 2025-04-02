#include "utilities/serial_logger.h"

// format:
// %d or %i: Signed integer
// %u: Unsigned integer
// %f: Float/double
// %s: String (null-terminated)
// %c: Character
// %x or %X: Hexadecimal (lowercase or uppercase)
// %p: Pointer address
// %%: Literal percent sign

// Define static class members here
bool _is_initialized = false;
std::string _last_message = "";
uint32_t _duplicate_count = 0;
unsigned long _last_log_time = 0;

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

void SerialLogger::log(LogLevel level, const char *format, ...)
{
  va_list args;
  va_start(args, format);
  unsigned long current_time = millis();

  // If it's a new message or force print is enabled
  if (format != _last_message)
  {
    // Print duplicate summary if we had duplicates
    if (_duplicate_count > 0 && level == LogLevel::Verbose)
    {
      VLOG("Last message repeated %i times", _duplicate_count);
      _duplicate_count = 0;
    }

    SerialLogger::log_at_level(level, format, args);
    _last_message = format;
    _last_log_time = current_time;
  }

  else // If it's a duplicate
  {
    _duplicate_count++;

    // Periodically print duplicate count for long-running duplicates
    if (current_time - _last_log_time >= 5000)
    {
      VLOG("Message repeated %i times so far...", _duplicate_count);
      _last_log_time = current_time;
    }
  }

  va_end(args);
}

void SerialLogger::log_at_level(LogLevel level, const char *format, ...)
{
  va_list args;
  va_start(args, format);

  switch (level)
  {
  case LogLevel::Verbose:
    VLOG(format, args);
    break;

  case LogLevel::Debug:
    DLOG(format, args);
    break;

  case LogLevel::Info:
    ILOG(format, args);
    break;

  case LogLevel::Warning:
    WLOG(format, args);
    break;

  case LogLevel::Error:
    ELOG(format, args);
    break;

  default:
    log_i("Unknown log level: %d, Message: %s", (int)level, format);
    break;
  }

  va_end(args);
}