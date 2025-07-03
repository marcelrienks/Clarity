#pragma once // preventing duplicate definitions, alternative to the traditional include guards

#include <variant>
#include <vector>
#include <string>
#include <lvgl.h>

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

enum class OilSensorTypes
{
    Pressure,
    Temperature
};

struct PanelNames {
    static constexpr const char *Splash = "SplashPanel";
    static constexpr const char *Oil = "OemOilPanel";
};

struct JsonDocNames {
    static constexpr const char *panel_name = "panel_name";
};

struct ComponentLocation
{
    lv_coord_t x = 0;
    lv_coord_t y = 0;
    lv_align_t align = LV_ALIGN_CENTER;
    lv_coord_t x_offset = 0;
    lv_coord_t y_offset = 0;
    lv_coord_t width = LV_SIZE_CONTENT;
    lv_coord_t height = LV_SIZE_CONTENT;
    
    ComponentLocation() = default;
    ComponentLocation(lv_coord_t x, lv_coord_t y) : x(x), y(y) {}
    ComponentLocation(lv_align_t align, lv_coord_t x_offset = 0, lv_coord_t y_offset = 0) 
        : align(align), x_offset(x_offset), y_offset(y_offset) {}
};

struct Configs
{
    std::string panel_name = PanelNames::Oil;
};