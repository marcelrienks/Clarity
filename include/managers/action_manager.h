#pragma once

#include "interfaces/i_action_manager.h"
#include "interfaces/i_action_service.h"
#include "interfaces/i_panel_service.h"
#include "managers/interrupt_manager.h"
#include "sensors/action_button_sensor.h"
#include "utilities/types.h"
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

/**
 * @class ActionManager
 * @brief Centralized action execution and button input management
 *
 * @details This class handles GPIO 32 button input detection, debouncing, and
 * timing logic to distinguish between short and long presses. It uses an
 * action-based approach where panels provide action objects that InputManager
 * executes when appropriate, supporting queuing during animations.
 *
 * @architecture Uses coordinated interrupt system for button state monitoring
 * @gpio_pin GPIO 32 with pull-down resistor (3.3V when pressed)
 * @timing Short press: 50ms-2000ms, Long press: 2000ms-5000ms, Timeout: >5000ms
 * @debouncing 50ms debounce window to prevent false triggers
 * @interrupt_driven Button state changes trigger coordinated interrupts
 */
class ActionManager : public IActionManager
{
  public:
    // Constructors and Destructors
    ActionManager(std::shared_ptr<ActionButtonSensor> buttonSensor, IPanelService *panelService);

    ActionManager(const ActionManager &) = delete;
    ActionManager &operator=(const ActionManager &) = delete;
    ~ActionManager() = default;

    // Core Functionality

    /**
     * @brief Initialize GPIO pin and input detection
     */
    void Init();


    /**
     * @brief Register a panel as the current action handler
     * @param service Pointer to panel implementing IActionService
     * @param panelName Name of the panel for action lookup
     */
    void RegisterPanel(IActionService *service, const char *panelName) override;

    /**
     * @brief Remove current panel registration
     */
    void ClearPanel() override;
    
    /**
     * @brief Set the panel service after construction (for circular dependency resolution)
     * @param panelService Panel service for UIState checking
     */
    void SetPanelService(IPanelService *panelService);

  private:
    // Static callback functions for interrupt system
    static bool EvaluateButtonChange(void* context);
    static void ExecuteButtonAction(void* context);
    
    // Internal processing methods
    void RegisterButtonInterrupts();
    void ProcessInputEvents();
    void HandlePressedState(unsigned long currentTime);
    bool ShouldTriggerLongPress(unsigned long currentTime);
    void ExecuteLongPressAction(unsigned long currentTime);

    // Button state tracking
    enum class ButtonState
    {
        IDLE,           // Button not pressed
        DEBOUNCE,       // Waiting for debounce period
        PRESSED,        // Button confirmed pressed, timing
        LONG_PRESS_SENT // Long press event already sent
    };

    // Timing constants (milliseconds)
    static constexpr unsigned long DEBOUNCE_TIME_MS = 50;
    static constexpr unsigned long SHORT_PRESS_MIN_MS = 50;
    static constexpr unsigned long LONG_PRESS_THRESHOLD_MS = 2000; // 2 seconds
    static constexpr unsigned long LONG_PRESS_MAX_MS = 5000;       // 5 seconds
    static constexpr unsigned long MAX_PRESS_TIME_MS = 5100;       // Slightly above 5s for timeout
    static constexpr unsigned long INPUT_TIMEOUT_MS = 3000;

    // Input processing methods
    void HandleButtonPress();
    void HandleButtonRelease();
    void CheckPressTimeout();
    bool IsButtonPressed() const;
    unsigned long GetCurrentTime() const;

    // Action processing methods
    void ProcessPendingActions();
    
    // Helper method to check if actions can be executed based on UIState
    bool CanExecuteActions() const;

    // Dependencies
    std::shared_ptr<ActionButtonSensor> buttonSensor_;
    IActionService *currentService_;
    IPanelService *panelService_;

    // State tracking
    ButtonState buttonState_;
    unsigned long pressStartTime_;
    unsigned long debounceStartTime_;
    bool lastButtonState_;
    bool initialized_;
    std::string currentPanelName_;

    // Phase 1: Pending action logic removed for simplicity
};