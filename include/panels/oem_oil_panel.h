#pragma once // preventing duplicate definitions, alternative to the traditional include guards

#include "interfaces/i_panel.h"
#include "interfaces/i_input_service.h"
#include "interfaces/i_gpio_provider.h"
#include "interfaces/i_display_provider.h"
#include "interfaces/i_style_service.h"
#include "components/oem/oem_oil_pressure_component.h"
#include "components/oem/oem_oil_temperature_component.h"
#include "sensors/oil_pressure_sensor.h"
#include "sensors/oil_temperature_sensor.h"
#include "utilities/types.h"
#include "managers/trigger_manager.h"

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
class OemOilPanel : public IPanel, public IInputService
{
public:
    // Constructors and Destructors
    OemOilPanel(IGpioProvider *gpio, IDisplayProvider *display, IStyleService *styleService);
    ~OemOilPanel();

    // Core Functionality Methods
    static constexpr const char* NAME = PanelNames::OIL;
    void Init() override;
    void Load(std::function<void()> callbackFunction) override;
    void Update(std::function<void()> callbackFunction) override;
    
    // IInputService Interface Implementation
    void OnShortPress() override;
    void OnLongPress() override;
    
    // IPanel override to provide input service
    IInputService* GetInputService() override { return this; }

    // Static Data Members
    static constexpr int32_t _animation_duration = 750;

private:
    // Core Functionality Methods
    void UpdateOilPressure();
    void UpdateOilTemperature();
    
    // Value mapping methods
    /// @brief Map oil pressure sensor value to display scale
    /// @param sensor_value Raw sensor value (1-10 Bar)
    /// @return Mapped value for display (0-60, representing 0.0-6.0 Bar x10)
    int32_t MapPressureValue(int32_t sensorValue);
    
    /// @brief Map oil temperature sensor value to display scale
    /// @param sensor_value Raw sensor value (0-120°C)
    /// @return Mapped value for display
    int32_t MapTemperatureValue(int32_t sensorValue);

    // Static Callback Methods
    static void ShowPanelCompletionCallback(lv_event_t *event);
    static void UpdatePanelCompletionCallback(lv_anim_t *animation);
    static void ExecutePressureAnimationCallback(void *target, int32_t value);
    static void ExecuteTemperatureAnimationCallback(void *target, int32_t value);

    // Instance Data Members - Dependencies
    IGpioProvider *gpioProvider_;
    IDisplayProvider *displayProvider_;
    IStyleService *styleService_;

    // Instance Data Members - UI Objects
    // screen_ is inherited from IPanel base class

    // Instance Data Members - Components and Sensors
    std::shared_ptr<IComponent> oemOilPressureComponent_;
    std::shared_ptr<IComponent> oemOilTemperatureComponent_;
    std::shared_ptr<ISensor> oemOilPressureSensor_;
    std::shared_ptr<ISensor> oemOilTemperatureSensor_;

    // Instance Data Members - State Variables
    int32_t currentOilPressureValue_;
    int32_t currentOilTemperatureValue_;
    bool isPressureAnimationRunning_ = false;
    bool isTemperatureAnimationRunning_ = false;
    bool forceComponentRefresh_ = false;  // Force component refresh regardless of value changes
    String lastTheme_;  // Track last theme to force refresh when theme changes

    // Instance Data Members - Animation Objects
    lv_anim_t pressureAnimation_;   // Instance-level animation objects (prevents memory leaks)
    lv_anim_t temperatureAnimation_;
    
};