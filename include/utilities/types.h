#pragma once // preventing duplicate definitions, alternative to the traditional include guards

#include <variant>
#include <vector>
#include <string>

// Define the possible return types for the sensor readings
using Reading = std::variant<std::monostate, int32_t, double, std::string, bool>;

enum class LogLevel
{
    Verbose,
    Debug,
    Info,
    Warning,
    Error
};

enum class PanelType
{
    Splash,
    Sensor,
    Configuration
};

enum class Theme
{
    Night,
    Day
};

struct Config
{
    Theme theme = Theme::Night;
    std::string panel = std::string();
};