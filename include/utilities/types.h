#pragma once // preventing duplicate definitions, alternative to the traditional include guards

#include <variant>
#include <vector>
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

struct PanelConfig
{
    std::string name = std::string();
    PanelIteration iteration = PanelIteration::Once;
};

enum Theme
{
    Light,
    Dark
};

struct Config
{
    Theme theme = Theme::Dark;
    std::vector<PanelConfig> panels;
};