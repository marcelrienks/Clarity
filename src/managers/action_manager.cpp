#include "managers/action_manager.h"
#include "managers/error_manager.h"
#include "utilities/types.h"
#include "utilities/constants.h"
#include <Arduino.h>
#include <cstring>

#ifdef CLARITY_DEBUG
    #include "esp32-hal-log.h"
    #define LOG_TAG "ActionManager"
#else
    #define log_v(...)
    #define log_d(...)
    #define log_w(...)
    #define log_e(...)
#endif

ActionManager::ActionManager(std::shared_ptr<ActionButtonSensor> buttonSensor, IPanelService *panelService)
    : buttonSensor_(buttonSensor), currentService_(nullptr), panelService_(panelService), buttonState_(ButtonState::IDLE), pressStartTime_(0),
      debounceStartTime_(0), lastButtonState_(false), initialized_(false)
{
    log_v("ActionManager() constructor called");
}

void ActionManager::Init()
{
    log_v("Init() called");
    if (initialized_)
    {
        log_w("ActionManager already initialized");
        return;
    }

    if (!buttonSensor_)
    {
        log_e("Button sensor is null");
        ErrorManager::Instance().ReportCriticalError("ActionManager", "Cannot initialize - button sensor is null");
        return;
    }

    // Initialize the button sensor
    buttonSensor_->Init();

    // Initialize state
    lastButtonState_ = IsButtonPressed();
    buttonState_ = ButtonState::IDLE;

    log_i("Initial button state: %s", lastButtonState_ ? "PRESSED" : "RELEASED");

    initialized_ = true;
    log_i("ActionManager initialized");
}

void ActionManager::ProcessInputEvents()
{
    log_v("ProcessInputEvents() called");
    if (!initialized_)
    {
        return;
    }

    bool currentButtonState = IsButtonPressed();

    // Log only actual button state changes
    static bool lastLoggedState = false;
    static bool firstCall = true;
    if (firstCall || currentButtonState != lastLoggedState)
    {
        log_i("Button state changed: %s", currentButtonState ? "PRESSED" : "RELEASED");
        lastLoggedState = currentButtonState;
        firstCall = false;
    }
    unsigned long currentTime = GetCurrentTime();

    // Detect button state changes
    if (currentButtonState != lastButtonState_)
    {
        if (currentButtonState)
        {
            // Button pressed
            HandleButtonPress();
        }
        else
        {
            // Button released
            HandleButtonRelease();
        }
        lastButtonState_ = currentButtonState;
    }

    // Handle state-specific logic
    switch (buttonState_)
    {
        case ButtonState::DEBOUNCE:
            if (currentTime - debounceStartTime_ >= DEBOUNCE_TIME_MS)
            {
                if (IsButtonPressed())
                {
                    buttonState_ = ButtonState::PRESSED;
                    pressStartTime_ = currentTime;
                    log_d("Button press confirmed after debounce at %lu ms", currentTime);
                }
                else
                {
                    buttonState_ = ButtonState::IDLE;
                }
            }
            break;

        case ButtonState::PRESSED:
            HandlePressedState(currentTime);
            break;

        case ButtonState::LONG_PRESS_SENT:
            CheckPressTimeout();
            break;

        case ButtonState::IDLE:
        default:
            // No action needed
            break;
    }

    // Process any pending actions
}

void ActionManager::HandlePressedState(unsigned long currentTime)
{
    if (!ShouldTriggerLongPress(currentTime))
    {
        CheckPressTimeout();
        return;
    }
    
    ExecuteLongPressAction(currentTime);
    buttonState_ = ButtonState::LONG_PRESS_SENT;
    CheckPressTimeout();
}

bool ActionManager::ShouldTriggerLongPress(unsigned long currentTime)
{
    if (buttonState_ == ButtonState::LONG_PRESS_SENT) return false;
    
    unsigned long pressDuration = currentTime - pressStartTime_;
    log_d("Button timing check - pressDuration: %lu, threshold: %lu, max: %lu, currentTime: %lu", 
          pressDuration, LONG_PRESS_THRESHOLD_MS, LONG_PRESS_MAX_MS, currentTime);
    
    return pressDuration >= LONG_PRESS_THRESHOLD_MS && 
           pressDuration <= LONG_PRESS_MAX_MS;
}

void ActionManager::ExecuteLongPressAction(unsigned long currentTime)
{
    if (!currentService_) return;
    
    auto longPressFunc = currentService_->GetLongPressFunction();
    if (!longPressFunc) return;
    
    log_i("Long press detected after %lu ms (started at %lu ms)", 
          currentTime - pressStartTime_, pressStartTime_);
    
    if (CanExecuteActions())
    {
        log_d("Executing long press action immediately");
        longPressFunc(currentService_->GetPanelContext());
        return;
    }
    
    // Phase 1: Simplified - just execute immediately for now
    log_d("Executing long press action (Phase 1 simplified)");
    longPressFunc(currentService_->GetPanelContext());
}

void ActionManager::RegisterPanel(IActionService *service, const char *panelName)
{
    log_v("RegisterPanel() called");
    currentService_ = service;
    currentPanelName_ = panelName ? panelName : "";
    log_d("Panel registered for actions: %s %p", panelName, service);
}

void ActionManager::ClearPanel()
{
    log_v("ClearPanel() called");
    currentService_ = nullptr;
}

/// @brief Set the panel service after construction (for circular dependency resolution)
/// @param panelService Panel service for UIState checking
void ActionManager::SetPanelService(IPanelService *panelService)
{
    log_v("SetPanelService() called");
    panelService_ = panelService;
}

/// @brief Handle button press event - starts debounce timer
/// @details Transitions to DEBOUNCE state and records press start time
void ActionManager::HandleButtonPress()
{
    log_v("HandleButtonPress() called");
    unsigned long currentTime = GetCurrentTime();

    log_i("ActionManager: Button press detected at %lu ms", currentTime);

    buttonState_ = ButtonState::DEBOUNCE;
    debounceStartTime_ = currentTime;
}

/// @brief Handle button release event - determines press type and executes action
/// @details Calculates press duration to distinguish between short/long presses
/// and executes the appropriate action if within valid timing windows
void ActionManager::HandleButtonRelease()
{
    log_v("HandleButtonRelease() called");
    unsigned long currentTime = GetCurrentTime();

    log_d("Button release detected at %lu ms", currentTime);

    if (buttonState_ == ButtonState::PRESSED)
    {
        // Calculate press duration
        unsigned long pressDuration = currentTime - pressStartTime_;
        log_d("Button release duration: %lu ms (from %lu to %lu)", pressDuration, pressStartTime_, currentTime);

        if (pressDuration > LONG_PRESS_MAX_MS)
        {
            // Press was too long (>5 seconds), ignore it
            log_w("Button press too long (%lu ms), ignoring", pressDuration);
        }
        else if (pressDuration >= SHORT_PRESS_MIN_MS && pressDuration < LONG_PRESS_THRESHOLD_MS)
        {
            log_i("Short press detected");
            log_d("Short press analysis - duration: %lu, minShort: %lu, maxShort: %lu", 
                  pressDuration, SHORT_PRESS_MIN_MS, LONG_PRESS_THRESHOLD_MS);

            if (currentService_)
            {
                // Get function pointer from current panel
                auto shortPressFunc = currentService_->GetShortPressFunction();
                if (shortPressFunc)
                {
                    // Check if actions can be executed based on UIState
                    if (CanExecuteActions())
                    {
                        log_d("Executing short press action immediately");
                        shortPressFunc(currentService_->GetPanelContext());
                    }
                    else
                    {
                        // Phase 1: Simplified - just execute immediately for now
                        log_d("Executing short press action (Phase 1 simplified)");
                        shortPressFunc(currentService_->GetPanelContext());
                    }
                }
            }
        }
        else if (pressDuration >= LONG_PRESS_THRESHOLD_MS && pressDuration <= LONG_PRESS_MAX_MS)
        {
            // This is a long press that completed without being sent during press hold
            log_i("Long press detected on release");

            if (currentService_)
            {
                auto longPressFunc = currentService_->GetLongPressFunction();
                if (longPressFunc)
                {
                    if (CanExecuteActions())
                    {
                        log_d("Executing long press action on release");
                        longPressFunc(currentService_->GetPanelContext());
                    }
                    else
                    {
                        // Phase 1: Simplified - just execute immediately for now
                        log_d("Executing long press action on release (Phase 1 simplified)");
                        longPressFunc(currentService_->GetPanelContext());
                    }
                }
            }
        }
    }

    buttonState_ = ButtonState::IDLE;
}

/// @brief Check if button press has exceeded maximum allowed duration
/// @details Resets state to IDLE if press exceeds MAX_PRESS_TIME_MS (5.1 seconds)
void ActionManager::CheckPressTimeout()
{
    log_v("CheckPressTimeout() called");
    unsigned long currentTime = GetCurrentTime();

    if (currentTime - pressStartTime_ >= MAX_PRESS_TIME_MS)
    {
        log_w("Button press timeout reached, resetting to idle");
        buttonState_ = ButtonState::IDLE;
    }
}

/// @brief Check if button is currently pressed
/// @return true if button is pressed (GPIO HIGH), false otherwise
/// @details Uses button sensor to read GPIO 32 state with pull-down configuration
bool ActionManager::IsButtonPressed() const
{
    log_v("IsButtonPressed() called");
    if (!buttonSensor_ || !initialized_)
    {
        return false;
    }

    // Use the button sensor to check if button is pressed
    bool pressed = buttonSensor_->IsButtonPressed();

    // Enhanced logging to debug button press detection
    static bool lastLoggedState = false;
    static bool firstCall = true;
    static unsigned long logCount = 0;

    logCount++;

    // Log every 10000 calls or on state changes only
    if (firstCall || pressed != lastLoggedState || (logCount % 10000 == 0))
    {
        log_d("ActionManager: Button check %lu - GPIO 32 state: %s", logCount,
              pressed ? "HIGH (PRESSED)" : "LOW (released)");
        lastLoggedState = pressed;
        firstCall = false;
    }

    return pressed;
}

/// @brief Get current system time in milliseconds
/// @return Current time from millis()
/// @details Wrapper for millis() to enable mocking in unit tests
unsigned long ActionManager::GetCurrentTime() const
{
    log_v("GetCurrentTime() called");
    // Use millis() equivalent - this will need to be mocked for testing
    return millis();
}



// Action Processing Methods

/// @brief Process any pending actions if UIState allows
/// @details Phase 1: Simplified - no action queuing for now
void ActionManager::ProcessPendingActions()
{
    log_v("ProcessPendingActions() called");
    // Phase 1: No action queuing - all actions execute immediately
}

/// @brief Check if actions can be executed based on current UIState
/// @return true if UIState is IDLE, false otherwise
/// @details Actions are only executed when UI is not busy with animations or updates
bool ActionManager::CanExecuteActions() const
{
    log_v("CanExecuteActions() called");
    if (!panelService_)
    {
        log_w("PanelService not available, defaulting to allow actions");
        return true; // Fallback to allow actions if panel service is not available
    }
    
    UIState currentState = panelService_->GetUiState();
    log_d("Checking UI state: %s, canExecute: %s", UIStateToString(currentState), 
          currentState == UIState::IDLE ? "true" : "false");
    
    // Actions can only be executed when UI is IDLE
    // UPDATING/LOADING/LVGL_BUSY should queue actions for later processing
    return currentState == UIState::IDLE;
}
