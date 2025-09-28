#pragma once

#include "interfaces/i_gpio_provider.h"
#include "interfaces/i_configuration_manager.h"
#include "definitions/types.h"
#include "definitions/constants.h"
#include "definitions/configs.h"
#include <vector>
#include <memory>

// Forward declarations for sensors
class ButtonSensor;
class IActionHandler;

#include "esp32-hal-log.h"

/**
 * @class ActionHandler
 * @brief Handles event-based actions with ButtonSensor ownership and press duration detection
 * 
 * @details Processes button events with timing detection and continuous evaluation:
 * - Short press: 50ms - 2000ms duration
 * - Long press: 2000ms - 5000ms duration
 * - Continuous evaluation: Actions evaluated every main loop cycle
 * 
 * @architecture Owns ButtonSensor for exclusive button monitoring
 * @timing_detection Precise press duration measurement for action classification
 * @function_injection Receives current panel functions dynamically
 */
class ActionHandler
{
public:
    // ========== Constructors and Destructor ==========
    ActionHandler(IGpioProvider* gpioProvider);
    ~ActionHandler();
    
    // ========== Public Interface Methods ==========
    // Main action processing methods
    void ValidateActions();
    void ExecutePendingActions();

    // Panel management for direct method calls
    void SetCurrentPanel(IActionHandler* panel);
    void ClearCurrentPanel();

    // Sensor access for action context
    ButtonSensor* GetButtonSensor() const { return buttonSensor_.get(); }

    // Status and diagnostics
    bool HasPendingAction() const;
    void ClearPendingAction();

    // Configuration management
    static void RegisterConfigSchema(IConfigurationManager* configurationManager);
    void SetConfigurationManager(IConfigurationManager* configurationManager);

    // ========== Public Data Members ==========
    // Button state machine (moved to public for StateToString access)
    enum class ButtonState {
        IDLE,
        PRESSED,
        LONG_PRESS_TRIGGERED,
        RELEASED
    };

private:
    // ========== Private Methods ==========
    // Core action processing
    void ProcessButtonEvents();
    void SetPendingAction(ButtonAction actionType);

    // Button timing detection
    void UpdateButtonState();
    void StartButtonTiming();
    void StopButtonTiming();
    ButtonAction CalculateButtonAction(unsigned long press_duration);

    // Helper methods
    bool IsButtonPressed() const;
    const char* StateToString(ButtonState state) const;

    // Configuration helpers
    unsigned long GetDebounceMs() const;
    unsigned long GetLongPressMs() const;
    
    // ========== Private Data Members ==========
    // Configuration items for button timing
    inline static Config::ConfigItem debounceConfig_{ConfigConstants::Items::DEBOUNCE_MS, UIStrings::ConfigLabels::DEBOUNCE_MS,
                                                      ConfigConstants::Defaults::DEFAULT_DEBOUNCE_MS,
                                                      Config::ConfigMetadata(ConfigConstants::Options::DEBOUNCE_OPTIONS, "ms", Config::ConfigItemType::Selection)};
    inline static Config::ConfigItem longPressConfig_{ConfigConstants::Items::LONG_PRESS_MS, UIStrings::ConfigLabels::LONG_PRESS_MS,
                                                       ConfigConstants::Defaults::DEFAULT_LONG_PRESS_MS,
                                                       Config::ConfigMetadata(ConfigConstants::Options::LONG_PRESS_OPTIONS, "ms", Config::ConfigItemType::Selection)};

    // Single pending action (LIFO with size 1)
    ButtonAction pendingActionType_ = ButtonAction::NONE;
    bool hasPendingAction_ = false;

    // Button state
    ButtonState buttonState_ = ButtonState::IDLE;
    unsigned long buttonPressStartTime_ = 0;
    unsigned long buttonPressEndTime_ = 0;

    // Current panel for direct method calls
    class IActionHandler* currentPanel_ = nullptr;

    // Preference service for configuration access
    IConfigurationManager* configurationManager_ = nullptr;

    // Handler-owned sensor
    IGpioProvider* gpioProvider_;
    std::unique_ptr<ButtonSensor> buttonSensor_;
};