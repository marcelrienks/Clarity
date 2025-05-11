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
    Config
};

enum class Theme
{
    Light,
    Dark
};

struct Config
{
    Theme theme = Theme::Dark;
    std::string panel = std::string();
};