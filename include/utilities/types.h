#pragma once // preventing duplicate definitions, alternative to the traditional include guards

#include <variant>
#include <string>

// Define the possible return types for the sensor readings
using Reading = std::variant<std::monostate, int32_t, double, std::string, bool>;

enum class PanelType
{
    Splash,
    Sensor,
    Config
};

enum class PanelIteration
{
    Infinite,
    Disabled,
    Once
};

struct PanelConfig {
    std::string panel_name;
    PanelIteration iteration;
};