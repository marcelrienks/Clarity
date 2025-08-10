#pragma once

#include "interfaces/i_interrupt.h"
#include "interfaces/i_input_service.h"
#include "interfaces/i_input_action.h"
#include "sensors/input_button_sensor.h"
#include "utilities/types.h"
#include <memory>
#include <unordered_map>
#include <string>
#include <functional>

/**
 * @class InputManager
 * @brief Centralized button input management with action-based workflow
 * 
 * @details This class handles GPIO 32 button input detection, debouncing, and 
 * timing logic to distinguish between short and long presses. It uses an
 * action-based approach where panels provide action objects that InputManager
 * executes when appropriate, supporting queuing during animations.
 * 
 * @architecture Implements IInterrupt for unified interrupt handling
 * @gpio_pin GPIO 32 with pull-down resistor (3.3V when pressed)
 * @timing Short press: 50ms-2000ms, Long press: 2000ms-5000ms, Timeout: >5000ms
 * @debouncing 50ms debounce window to prevent false triggers
 * @priority 50 (lower than triggers, higher than background tasks)
 */
class InputManager : public IInterrupt
{
public:
    // Constructors and Destructors
    InputManager(std::shared_ptr<InputButtonSensor> buttonSensor);
    
    /**
     * @brief Set callback for panel switch requests from actions
     * @param callback Function to call when an action requests a panel switch
     */
    void SetPanelSwitchCallback(std::function<void(const char*)> callback);
    
    InputManager(const InputManager&) = delete;
    InputManager& operator=(const InputManager&) = delete;
    ~InputManager() = default;

    // Core Functionality
    
    /**
     * @brief Initialize GPIO pin and input detection
     */
    void Init();

    // IInterrupt Interface Implementation
    
    /**
     * @brief Check for pending interrupts and process them (IInterrupt interface)
     * @details Called by InterruptManager during idle time
     */
    void CheckInterrupts() override;

    /**
     * @brief Check if there are pending input interrupts (IInterrupt interface)
     * @details Quick check without processing for optimization
     * @return true if input events are pending
     */
    bool HasPendingInterrupts() const override;

    /**
     * @brief Get interrupt priority level (IInterrupt interface)
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
     * @param service Pointer to panel implementing IInputService
     * @param panelName Name of the panel for action lookup
     */
    void SetInputService(IInputService* service, const char* panelName);

    /**
     * @brief Remove current input service
     */
    void ClearInputService();


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
        std::unique_ptr<IInputAction> action = nullptr;
        unsigned long timestamp = 0;
        
        bool HasAction() const { return action != nullptr; }
        void Clear() { 
            action.reset(); 
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
    std::shared_ptr<InputButtonSensor> buttonSensor_;
    IInputService* currentService_;
    std::function<void(const char*)> panelSwitchCallback_;


    // State tracking
    ButtonState buttonState_;
    unsigned long pressStartTime_;
    unsigned long debounceStartTime_;
    bool lastButtonState_;
    bool initialized_;
    std::string currentPanelName_;
    PendingAction pendingAction_;
};