#pragma once

#include "interfaces/i_input_service.h"
#include "interfaces/i_panel_service.h"
#include "sensors/input_button_sensor.h"
#include "utilities/types.h"
#include <memory>
#include <unordered_map>
#include <string>

/**
 * @class InputManager
 * @brief Centralized button input management with debouncing and timing logic
 * 
 * @details This class handles GPIO 34 button input detection, debouncing, and 
 * timing logic to distinguish between short and long presses. It provides a
 * clean interface for panels to register as input handlers.
 * 
 * @architecture Separate from trigger system - dedicated to button input
 * @gpio_pin GPIO 34 with rising edge detection (3.3V pull-up)
 * @timing Short press: 50ms-500ms, Long press: >500ms, Max: 3000ms
 * @debouncing 50ms debounce window to prevent false triggers
 */
class InputManager
{
public:
    // Constructors and Destructors
    explicit InputManager(std::shared_ptr<InputButtonSensor> buttonSensor);
    
    /**
     * @brief Register input actions for panels
     * @details Must be called after Init() to configure panel-specific actions
     */
    void RegisterInputActions();
    InputManager(const InputManager&) = delete;
    InputManager& operator=(const InputManager&) = delete;
    ~InputManager() = default;

    // Core Functionality
    
    /**
     * @brief Initialize GPIO pin and input detection
     * @param panelService Service for panel switching requests
     */
    void Init(IPanelService* panelService);

    /**
     * @brief Process button input events (call regularly from main loop)
     * @details Handles debouncing, timing, and event generation
     */
    void ProcessInputEvents();

    /**
     * @brief Register a panel as the current input service
     * @param service Pointer to panel implementing IInputService
     * @param panelName Name of the panel for action lookup
     */
    void SetInputService(IInputService* service, const char* panelName);
    
    /**
     * @brief Request navigation to another panel
     * @param targetPanel Name of the panel to navigate to
     * @details Called by panels when they need to trigger navigation
     */
    void RequestPanelSwitch(const char* targetPanel);

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
    static constexpr unsigned long LONG_PRESS_THRESHOLD_MS = 500;
    static constexpr unsigned long MAX_PRESS_TIME_MS = 3000;

    // Input processing methods
    void HandleButtonPress();
    void HandleButtonRelease();
    void CheckPressTimeout();
    bool IsButtonPressed() const;
    unsigned long GetCurrentTime() const;

    // Input action structure
    struct InputAction {
        const char* targetPanel;  // Panel to load
        bool enabled;            // Whether action is enabled
    };
    
    // Dependencies
    std::shared_ptr<InputButtonSensor> buttonSensor_;
    IInputService* currentService_;
    IPanelService* panelService_;
    
    // Action mappings (panel name -> action)
    std::unordered_map<std::string, InputAction> shortPressActions_;
    std::unordered_map<std::string, InputAction> longPressActions_;

    // State tracking
    ButtonState buttonState_;
    unsigned long pressStartTime_;
    unsigned long debounceStartTime_;
    bool lastButtonState_;
    bool initialized_;
    std::string currentPanelName_;
};