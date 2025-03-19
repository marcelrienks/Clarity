#pragma once // preventing duplicate definitions, alternative to the traditional include guards

#include <variant>
#include <string>

// Define the possible return types for the sensor readings
using Reading = std::variant<std::monostate, int32_t, double, std::string, bool>;

//TODO: is there value in a seperate file, or within th eheader they relate to

//TODO: is there value in this?
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