#pragma once // preventing duplicate definitions, alternative to the traditional include guards

#include "interfaces/i_panel.h"
#include "components/oem/oem_oil_pressure_component.h"
#include "components/oem/oem_oil_temperature_component.h"
#include "sensors/oil_pressure_sensor.h"
#include "sensors/oil_temperature_sensor.h"
#include "utilities/types.h"

#include <utilities/lv_tools.h>

/**
 * @class OemOilPanel
 * @brief Main oil monitoring dashboard panel
 * 
 * @details This panel serves as the primary monitoring interface for engine oil
 * systems. It coordinates two specialized gauge components for pressure and
 * temperature monitoring, positioned side-by-side for optimal visibility.
 * 
 * @presenter_role Coordinates OemOilPressureComponent and OemOilTemperatureComponent
 * @data_sources OilPressureSensor and OilTemperatureSensor with delta-based updates
 * @update_strategy Smart caching with animation-based smooth transitions
 * 
 * @ui_layout:
 * - Oil Pressure Gauge: Left side (LV_ALIGN_LEFT_MID) - 120x120px
 * - Oil Temperature Gauge: Right side (LV_ALIGN_RIGHT_MID) - 120x120px
 * - Both gauges feature danger zone indicators and smooth needle animations
 * 
 * @animation_system:
 * - Dual independent animations for pressure and temperature
 * - Prevents conflicts with separate animation objects
 * - Completion callbacks ensure proper synchronization
 * 
 * @performance_optimizations:
 * - Delta-based updates (skips unchanged values)
 * - Cached previous values for comparison
 * - Efficient animation state tracking
 * 
 * @context The components are currently set to 240x240 size in order to ensure
 * that they maintain a consistent appearance with OEM styling by being shown on either side of the screen.
 */
class OemOilPanel : public IPanel
{
public:
    // Constructors and Destructors
    OemOilPanel();
    ~OemOilPanel();

    // Core Functionality Methods
    static constexpr const char* name = PanelNames::Oil;
    void init() override;
    void load(std::function<void()> callback_function) override;
    void update(std::function<void()> callback_function = nullptr) override;

    // Static Data Members
    static constexpr int32_t _animation_duration = 1000;

private:
    // Core Functionality Methods
    void update_oil_pressure();
    void update_oil_temperature();
    
    // Value mapping methods
    /// @brief Map oil pressure sensor value to display scale
    /// @param sensor_value Raw sensor value (1-10 Bar)
    /// @return Mapped value for display (0-60, representing 0.0-6.0 Bar x10)
    int32_t map_pressure_value(int32_t sensor_value);
    
    /// @brief Map oil temperature sensor value to display scale
    /// @param sensor_value Raw sensor value (0-120°C)
    /// @return Mapped value for display
    int32_t map_temperature_value(int32_t sensor_value);

    // Static Callback Methods
    static void show_panel_completion_callback(lv_event_t *event);
    static void update_panel_completion_callback(lv_anim_t *animation);
    static void execute_pressure_animation_callback(void *target, int32_t value);
    static void execute_temperature_animation_callback(void *target, int32_t value);

    // Instance Data Members - UI Objects
    lv_obj_t *_screen; // All panels should always have their own screens

    // Instance Data Members - Components and Sensors
    std::shared_ptr<IComponent> _oem_oil_pressure_component;
    std::shared_ptr<IComponent> _oem_oil_temperature_component;
    std::shared_ptr<ISensor> _oem_oil_pressure_sensor;
    std::shared_ptr<ISensor> _oem_oil_temperature_sensor;

    // Instance Data Members - State Variables
    int32_t _current_oil_pressure_value;
    int32_t _current_oil_temperature_value;
    bool _is_pressure_animation_running = false;
    bool _is_temperature_animation_running = false;

    // Instance Data Members - Animation Objects
    lv_anim_t _pressure_animation;   // Instance-level animation objects (prevents memory leaks)
    lv_anim_t _temperature_animation;
};