#pragma once

#include "interfaces/i_interrupt_service.h"
#include "interfaces/i_action_manager.h"
#include "interfaces/i_action_service.h"
#include "sensors/action_button_sensor.h"
#include "utilities/types.h"
#include <memory>
#include <unordered_map>
#include <string>
#include <functional>

/**
 * @class ActionManager
 * @brief Centralized action execution and button input management
 * 
 * @details This class handles GPIO 32 button input detection, debouncing, and 
 * timing logic to distinguish between short and long presses. It uses an
 * action-based approach where panels provide action objects that InputManager
 * executes when appropriate, supporting queuing during animations.
 * 
 * @architecture Implements IInterruptService for unified interrupt handling
 * @gpio_pin GPIO 32 with pull-down resistor (3.3V when pressed)
 * @timing Short press: 50ms-2000ms, Long press: 2000ms-5000ms, Timeout: >5000ms
 * @debouncing 50ms debounce window to prevent false triggers
 * @priority 50 (lower than triggers, higher than background tasks)
 */
class ActionManager : public IInterruptService, public IActionManager
{
public:
    // Constructors and Destructors
    ActionManager(std::shared_ptr<ActionButtonSensor> buttonSensor);
    
    /**
     * @brief Set callback for panel switch requests from actions
     * @param callback Function to call when an action requests a panel switch
     */
    void SetPanelSwitchCallback(std::function<void(const char*)> callback) override;
    
    ActionManager(const ActionManager&) = delete;
    ActionManager& operator=(const ActionManager&) = delete;
    ~ActionManager() = default;

    // Core Functionality
    
    /**
     * @brief Initialize GPIO pin and input detection
     */
    void Init();

    // IInterruptService Interface Implementation
    
    /**
     * @brief Check for pending interrupts and process them (IInterruptService interface)
     * @details Called by InterruptManager during idle time
     */
    void CheckInterrupts() override;

    /**
     * @brief Check if there are pending input interrupts (IInterruptService interface)
     * @details Quick check without processing for optimization
     * @return true if input events are pending
     */
    bool HasPendingInterrupts() const override;

    /**
     * @brief Get interrupt priority level (IInterruptService interface)
     * @details Input priority is 50 (lower than triggers=100)
     * @return Priority value of 50
     */
    int GetPriority() const override { return 50; }

    // Legacy Methods (for backward compatibility during transition)
    
    /**
     * @brief Process button input events (call regularly from main loop)
     * @details Handles debouncing, timing, and event generation
     * @deprecated Use CheckInterrupts() via InterruptManager instead
     */
    void ProcessInputEvents();

    /**
     * @brief Register a panel as the current input service
     * @param service Pointer to panel implementing IActionService
     * @param panelName Name of the panel for action lookup
     */
    void SetInputService(IActionService* service, const char* panelName) override;

    /**
     * @brief Remove current input service
     */
    void ClearInputService() override;
    
    /**
     * @brief Request panel switch operation (IActionManager interface)
     * @param targetPanel Name of the panel to switch to
     */
    void RequestPanelSwitch(const char* targetPanel) override;

private:
    // Button state tracking
    enum class ButtonState {
        IDLE,           // Button not pressed
        DEBOUNCE,       // Waiting for debounce period
        PRESSED,        // Button confirmed pressed, timing
        LONG_PRESS_SENT // Long press event already sent
    };

    // Timing constants (milliseconds)
    static constexpr unsigned long DEBOUNCE_TIME_MS = 50;
    static constexpr unsigned long SHORT_PRESS_MIN_MS = 50;
    static constexpr unsigned long LONG_PRESS_THRESHOLD_MS = 2000;  // 2 seconds
    static constexpr unsigned long LONG_PRESS_MAX_MS = 5000;       // 5 seconds
    static constexpr unsigned long MAX_PRESS_TIME_MS = 5100;      // Slightly above 5s for timeout
    static constexpr unsigned long INPUT_TIMEOUT_MS = 3000;

    // Pending action structure  
    struct PendingAction {
        Action action;
        unsigned long timestamp = 0;
        
        bool HasAction() const { return action.IsValid() || action.IsPanelSwitch(); }
        void Clear() { 
            action = Action(); 
            timestamp = 0; 
        }
    };

    // Input processing methods
    void HandleButtonPress();
    void HandleButtonRelease();
    void CheckPressTimeout();
    bool IsButtonPressed() const;
    unsigned long GetCurrentTime() const;
    
    // Action processing methods
    void ProcessPendingActions();

    // Dependencies
    std::shared_ptr<ActionButtonSensor> buttonSensor_;
    IActionService* currentService_;
    std::function<void(const char*)> panelSwitchCallback_;

    // Static instance for global access
    static ActionManager* instance_;

    // State tracking
    ButtonState buttonState_;
    unsigned long pressStartTime_;
    unsigned long debounceStartTime_;
    bool lastButtonState_;
    bool initialized_;
    std::string currentPanelName_;
    PendingAction pendingAction_;
};