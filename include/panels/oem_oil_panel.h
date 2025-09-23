#pragma once

#include "components/oem/oem_oil_pressure_component.h"
#include "components/oem/oem_oil_temperature_component.h"
#include "interfaces/i_action_handler.h"
#include "interfaces/i_display_provider.h"
#include "interfaces/i_gpio_provider.h"
#include "interfaces/i_panel.h"
#include "interfaces/i_panel_manager.h"
#include "interfaces/i_configuration_manager.h"
#include "interfaces/i_style_manager.h"
#include "sensors/oil_pressure_sensor.h"
#include "sensors/oil_temperature_sensor.h"
#include "definitions/types.h"
#include <Arduino.h>

// Forward declarations
// (Direct component usage - no factory needed)

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
    // ========== Constructors and Destructor ==========
    OemOilPanel(IGpioProvider *gpio, IDisplayProvider *display, IStyleManager *styleService);
    ~OemOilPanel();

    // ========== Public Interface Methods ==========
    static constexpr const char *NAME = PanelNames::OIL;
    void Init() override;
    void Load() override;
    void Update() override;

    // Manager injection method
    void SetManagers(IPanelManager *panelService, IStyleManager *styleService);

    void SetPreferenceService(IConfigurationManager *preferenceService);
    
    void ApplyCurrentSensorSettings();

    // IActionService Interface Implementation (inherited through IPanel)
    // Old function pointer methods removed - using direct HandleShortPress/HandleLongPress
    
    // Public action handlers
    void HandleShortPress() override;
    void HandleLongPress() override;

    // ========== Public Data Members ==========
    static constexpr int32_t _animation_duration = 750;

  private:
    // ========== Private Methods ==========
    void UpdateOilPressure(bool forceRefresh = false);
    void UpdateOilTemperature(bool forceRefresh = false);

    // Value mapping methods
    int32_t MapPressureValue(int32_t sensorValue);
    
    // Helper methods for pressure mapping
    int32_t MapPressureByUnit(int32_t sensorValue, const std::string& unit);
    int32_t MapPSIPressure(int32_t sensorValue);
    int32_t MapkPaPressure(int32_t sensorValue);
    int32_t MapBarPressure(int32_t sensorValue);
    int32_t ClampValue(int32_t value, int32_t minVal, int32_t maxVal);

    int32_t MapTemperatureValue(int32_t sensorValue);

    // ========== Static Methods ==========
    static void ShowPanelCompletionCallback(lv_event_t *event);
    static void UpdatePanelCompletionCallback(lv_anim_t *animation);
    static void ExecutePressureAnimationCallback(void *target, int32_t value);
    static void ExecuteTemperatureAnimationCallback(void *target, int32_t value);

    // ========== Private Data Members ==========
    IGpioProvider *gpioProvider_;
    IDisplayProvider *displayProvider_;
    IStyleManager *styleService_;
    IPanelManager *panelService_;
    IConfigurationManager *preferenceService_ = nullptr;
    // Instance Data Members - UI Objects
    lv_obj_t* screen_ = nullptr;

    // Instance Data Members - Components and Sensors (static allocation)
    OemOilPressureComponent oemOilPressureComponent_;
    OemOilTemperatureComponent oemOilTemperatureComponent_;
    std::shared_ptr<ISensor> oemOilPressureSensor_;
    std::shared_ptr<ISensor> oemOilTemperatureSensor_;
    bool componentsInitialized_ = false;

    // Instance Data Members - State Variables
    int32_t currentOilPressureValue_;
    int32_t currentOilTemperatureValue_;
    String lastTheme_;                   // Track last theme to force refresh when theme changes

    // Cache settings to avoid redundant updates
    int lastUpdateRate_ = -1;            // Track last applied update rate
    String lastPressureUnit_;            // Track last applied pressure unit
    String lastTempUnit_;                // Track last applied temperature unit

    // Cached sensor references to avoid repeated static_pointer_cast
    std::shared_ptr<OilPressureSensor> cachedPressureSensor_;
    std::shared_ptr<OilTemperatureSensor> cachedTemperatureSensor_;

    // Simplified animation state (single enum instead of multiple booleans)
    enum class AnimationState {
        IDLE,
        PRESSURE_RUNNING,
        TEMPERATURE_RUNNING,
        BOTH_RUNNING
    };
    AnimationState animationState_ = AnimationState::IDLE;

    // Instance Data Members - Animation Objects
    lv_anim_t pressureAnimation_; // Instance-level animation objects (prevents memory leaks)
    lv_anim_t temperatureAnimation_;
};