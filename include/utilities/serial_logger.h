#pragma once // preventing duplicate definitions, alternative to the traditional include guards

#include "utilities/ticker.h"
#include "utilities/types.h"

#include <esp32-hal-log.h>
#include <stdarg.h>

#define VLOG(format, ...) log_v("[%s][%s] " format, __FILE__, __FUNCTION__, ##__VA_ARGS__)
#define DLOG(format, ...) log_d("[%s][%s] " format, __FILE__, __FUNCTION__, ##__VA_ARGS__)
#define ILOG(format, ...) log_i("[%s][%s] " format, __FILE__, __FUNCTION__, ##__VA_ARGS__)
#define WLOG(format, ...) log_w("[%s][%s] " format, __FILE__, __FUNCTION__, ##__VA_ARGS__)
#define ELOG(format, ...) log_e("[%s][%s][%s] " format, __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__)

class SerialLogger
{
public:
    static void init(unsigned long baud_rate = 115200);
    static void log(LogLevel level, const char *format, ...);
    static void log_at_level(LogLevel level, const char *format, ...);

private:
    static bool _is_initialized;
    static std::string _last_message;
    static uint32_t _duplicate_count;
    static unsigned long _last_log_time;
};