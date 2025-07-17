#pragma once

// System/Library Includes
#include <memory>

// Project Includes
#include "interfaces/i_trigger.h"
#include "sensors/lights_sensor.h"
#include "utilities/types.h"

/**
 * @class LightsTrigger
 * @brief Trigger for monitoring lights switch and applying system-wide theme changes
 * 
 * @details This trigger monitors GPIO 32 (DIP switch 4) and applies theme changes
 * to the entire system when the lights switch state changes. Unlike other triggers that
 * switch panels, this trigger modifies the visual theme of all existing panels.
 * 
 * @trigger_conditions:
 * - GPIO 32 HIGH (switch ON): Lights on - Apply night theme system-wide
 * - GPIO 32 LOW (switch OFF): Lights off - Apply day theme system-wide
 * 
 * @behavior:
 * - Monitors lights switch state continuously
 * - Applies theme changes immediately when switch changes
 * - Does not change active panel, only visual theme
 * - Persists theme state across all panels
 * 
 * @theme_application:
 * - Night theme: Dark background with red accents (lights on)
 * - Day theme: Light background with white accents (lights off)
 * - Applied to all UI components system-wide
 * 
 * @integration:
 * - Uses LightsSensor for GPIO 32 monitoring
 * - Interfaces with StyleManager for theme application
 * - Registered with InterruptManager for continuous monitoring
 * 
 * @implementation_notes:
 * - Does not change panels (get_target_panel returns nullptr)
 * - Does not support restoration (should_restore returns false)
 * - Triggers on state change, not specific states
 * - Applies theme changes directly in evaluate() method
 * 
 * @context: This trigger provides automatic theme switching based on
 * vehicle lights state, allowing the display to match the dashboard
 * lighting without affecting panel functionality.
 */
class LightsTrigger : public ITrigger
{
public:
    // Constructors and Destructors
    LightsTrigger(bool enable_restoration = false);

    // Core Functionality Methods
    void init() override;
    bool evaluate() override;
    const char* get_target_panel() const override;
    const char* get_id() const override;
    bool should_restore() const override;

private:
    // Private Data Members
    bool _last_lights_state;                 ///< Previous lights state for change detection
    LightsSensor _lights_sensor;             ///< Lights sensor for GPIO 32 reading
    static constexpr const char* TRIGGER_ID = "lights_trigger"; ///< Unique trigger identifier
};