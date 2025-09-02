#pragma once

#include "interfaces/i_handler.h"
#include "interfaces/i_gpio_provider.h"
#include "utilities/types.h"
#include "utilities/constants.h"
#include <vector>
#include <memory>

// Forward declarations for sensors
class ButtonSensor;

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
class ActionHandler : public IHandler
{
public:
    ActionHandler(IGpioProvider* gpioProvider);
    ~ActionHandler();
    
    // IHandler interface - new interrupt system only
    void Process() override;
    
    // New Action system interface
    bool RegisterAction(const Action& action);
    void UnregisterAction(const char* id);
    void EvaluateActions();  // Called every main loop cycle
    
    // Function injection system for dynamic panel functions
    void UpdatePanelFunctions(void (*shortPressFunc)(), void (*longPressFunc)());
    void ClearPanelFunctions();
    
    // Button event processing
    void ProcessButtonEvents();
    ButtonAction DetectButtonAction();
    
    // Sensor access for action context
    ButtonSensor* GetButtonSensor() const { return buttonSensor_.get(); }
    
    // Status and diagnostics
    size_t GetActionCount() const;
    bool HasPendingActions() const;
    void PrintActionStatus() const;
    
private:
    // Core action processing
    void EvaluateIndividualAction(Action& action);
    void ExecuteQueuedActions();
    void ExecuteAction(const Action& action);  // Legacy compatibility
    bool ShouldTriggerAction(const Action& action);
    
    // Button timing detection
    void UpdateButtonState();
    void StartButtonTiming();
    void StopButtonTiming();
    ButtonAction CalculateButtonAction(unsigned long pressDuration);
    
    // Helper methods
    Action* FindAction(const char* id);
    bool IsButtonPressed() const;
    
    // Timing constants
    static constexpr unsigned long MIN_PRESS_DURATION_MS = 50;
    static constexpr unsigned long SHORT_PRESS_MAX_MS = 2000;
    static constexpr unsigned long LONG_PRESS_MIN_MS = 2000;
    static constexpr unsigned long LONG_PRESS_MAX_MS = 5000;
    
    static constexpr size_t MAX_ACTIONS = 8;
    
    // Action storage
    Action actions_[MAX_ACTIONS];
    size_t actionCount_ = 0;
    
    // Button timing state
    bool buttonPressed_ = false;
    bool buttonPreviouslyPressed_ = false;
    unsigned long buttonPressStartTime_ = 0;
    unsigned long buttonPressEndTime_ = 0;
    
    // Function injection for dynamic panel functions
    void (*currentShortPressFunc_)() = nullptr;
    void (*currentLongPressFunc_)() = nullptr;
    
    // Handler-owned sensor
    IGpioProvider* gpioProvider_;
    std::unique_ptr<ButtonSensor> buttonSensor_;
};