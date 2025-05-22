#pragma once // preventing duplicate definitions, alternative to the traditional include guards

#include <variant>
#include <vector>
#include <string>

// Define the possible return types for the sensor readings
using Reading = std::variant<std::monostate, int32_t, double, std::string, bool>;

enum class LogLevels
{
    Verbose,
    Debug,
    Info,
    Warning,
    Error
};

enum class Themes
{
    Night,
    Day
};

struct PanelNames {
    static constexpr const char *Splash = "SplashPanel";
    static constexpr const char *Demo = "DemoPanel";
    static constexpr const char *Oil = "OilPanel";
};

struct Configs
{
    Themes theme = Themes::Night;
    const char *panel_name = PanelNames::Demo;
};