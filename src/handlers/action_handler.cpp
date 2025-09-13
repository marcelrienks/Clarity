#include "handlers/action_handler.h"
#include "managers/error_manager.h"
#include "sensors/button_sensor.h"
#include "hardware/gpio_pins.h"
#include <Arduino.h>
#include <algorithm>
#include <cstring>

#include "esp32-hal-log.h"
#include "utilities/logging.h"  // For log_t()

ActionHandler::ActionHandler(IGpioProvider* gpioProvider)
    : gpioProvider_(gpioProvider),
      actionCount_(0),
      buttonState_(ButtonState::IDLE),
      buttonPressStartTime_(0),
      buttonPressEndTime_(0)
{
    log_d("ActionHandler() constructor called");
    
    // Create and initialize ButtonSensor owned by this handler
    if (gpioProvider_) {
        
        buttonSensor_ = std::make_unique<ButtonSensor>(gpioProvider_);
        buttonSensor_->Init();
        
        log_i("ActionHandler created and initialized ButtonSensor");
    }
    else {
        log_w("ActionHandler created without GPIO provider - ButtonSensor not initialized");
    }
}

ActionHandler::~ActionHandler() {
    log_d("ActionHandler destructor called");
}

void ActionHandler::Process() {
    static unsigned long lastProcessTime = 0;
    unsigned long currentTime = millis();
    
    // Removed Process() logging for critical path optimization - Step 10
    
    // Actions are evaluated every main loop cycle (continuous evaluation)
    UpdateButtonState();
    ProcessButtonEvents();
    EvaluateActions();
    
    // Execute pending actions (this will be called during UI IDLE by InterruptManager)
    ExecutePendingActions();
}

bool ActionHandler::RegisterAction(const Action& action) {
    if (actionCount_ >= MAX_ACTIONS) {
        log_e("Cannot register action '%s' - maximum actions reached (%d)", 
              action.id, MAX_ACTIONS);
        return false;
    }
    
    // Check for duplicate ID
    if (FindAction(action.id) != nullptr) {
        log_w("Action with ID '%s' already registered", action.id);
        return false;
    }
    
    // Copy action to storage
    actions_[actionCount_] = action;
    actions_[actionCount_].hasTriggered = false;  // Always start untriggered
    actionCount_++;
    
    log_i("Registered action '%s' (press type: %d) - total actions now: %zu", 
          action.id, static_cast<int>(action.pressType), actionCount_);
    return true;
}

void ActionHandler::UnregisterAction(const char* id) {
    Action* action = FindAction(id);
    if (!action) {
        log_w("Cannot unregister action '%s' - not found", id);
        return;
    }
    
    // Find index and shift array
    size_t index = action - actions_;
    for (size_t i = index; i < actionCount_ - 1; i++) {
        actions_[i] = actions_[i + 1];
    }
    actionCount_--;
    
    log_i("Unregistered action '%s'", id);
}

void ActionHandler::EvaluateActions() {
    // Process each action for trigger conditions
    
    // Check if there's a valid button action to evaluate
    ButtonAction detectedAction = DetectButtonAction();
    
    // Also check for long press during button hold (without waiting for release)
    if (detectedAction == ButtonAction::NONE) {
        detectedAction = DetectLongPressDuringHold();
    }
    
    bool anyActionTriggered = false;
    
    for (size_t i = 0; i < actionCount_; i++) {
        // Evaluating action
        
        if (EvaluateIndividualActionWithDetectedAction(actions_[i], detectedAction)) {
            anyActionTriggered = true;
        }
    }
    
    // Clear timing data after all actions have been evaluated to prevent duplicate triggers
    if (detectedAction != ButtonAction::NONE && anyActionTriggered) {
        buttonPressStartTime_ = 0;
        buttonPressEndTime_ = 0;
    }
}

void ActionHandler::EvaluateIndividualAction(Action& action) {
    if (ShouldTriggerAction(action)) {
        action.hasTriggered = true;
    }
}

bool ActionHandler::EvaluateIndividualActionWithDetectedAction(Action& action, ButtonAction detectedAction) {
    if (ShouldTriggerActionWithDetectedAction(action, detectedAction)) {
        action.hasTriggered = true;
        return true;
    }
    return false;
}

bool ActionHandler::ShouldTriggerAction(const Action& action) {
    // Check if we have a button event that matches this action's press type
    ButtonAction detectedAction = DetectButtonAction();
    return ShouldTriggerActionWithDetectedAction(action, detectedAction);
}

bool ActionHandler::ShouldTriggerActionWithDetectedAction(const Action& action, ButtonAction detectedAction) {
    if (detectedAction == ButtonAction::NONE) {
        return false;
    }
    
    // Action evaluation completed
    
    // Match detected button action to action press type
    if (action.pressType == ActionPress::SHORT && detectedAction == ButtonAction::SHORT_PRESS) {
        bool shouldTrigger = !action.hasTriggered;
        if (shouldTrigger) log_t("Button action detected: SHORT_PRESS");
        return shouldTrigger;  // Only trigger once per press event
    }
    else if (action.pressType == ActionPress::LONG && detectedAction == ButtonAction::LONG_PRESS) {
        bool shouldTrigger = !action.hasTriggered;
        if (shouldTrigger) log_t("Button action detected: LONG_PRESS");
        return shouldTrigger;  // Only trigger once per press event
    }
    
    return false;
}

void ActionHandler::ExecutePendingActions() {
    // Process all pending actions using the documented Execute method
    for (size_t i = 0; i < actionCount_; i++) {
        Action& action = actions_[i];
        if (action.hasTriggered) {
            log_i("ExecutePendingActions: Executing triggered action '%s' (type: %s)", 
                  action.id, action.pressType == ActionPress::SHORT ? "SHORT" : "LONG");
            
            // Use injected panel functions if available, otherwise use action's own function
            if (action.pressType == ActionPress::SHORT && currentShortPressFunc_) {
                log_i("Executing injected SHORT press function for action '%s'", action.id);
                currentShortPressFunc_(currentPanelContext_);
                action.hasTriggered = false;  // Clear manually since we're using injected function
            }
            else if (action.pressType == ActionPress::LONG && currentLongPressFunc_) {
                log_i("Executing injected LONG press function for action '%s'", action.id);
                currentLongPressFunc_(currentPanelContext_);
                action.hasTriggered = false;  // Clear manually since we're using injected function
            }
            else {
                log_i("Using action's own execute function for '%s'", action.id);
                // Use the documented Execute method
                action.Execute();  // This will clear hasTriggered automatically
            }
        }
    }
}

void ActionHandler::ExecuteAction(const Action& action) {
    // Execute action based on press type
    // Check if we should use panel function or action's own function
    if (action.pressType == ActionPress::SHORT && currentShortPressFunc_) {
        currentShortPressFunc_(currentPanelContext_);
    }
    else if (action.pressType == ActionPress::LONG && currentLongPressFunc_) {
        currentLongPressFunc_(currentPanelContext_);
    }
    else if (action.executeFunc) {
        action.executeFunc();
    }
    else {
        log_w("No function to execute for action '%s'", action.id);
    }
}

void ActionHandler::UpdateButtonState() {
    bool currentPressed = IsButtonPressed();
    unsigned long currentTime = millis();

    static unsigned long lastDebugTime = 0;
    ButtonState previousState = buttonState_;

    // State machine transitions
    switch (buttonState_) {
        case ButtonState::IDLE:
            if (currentPressed) {
                buttonState_ = ButtonState::PRESSED;
                StartButtonTiming();
                log_t("Button press started");
            }
            break;

        case ButtonState::PRESSED:
            if (!currentPressed) {
                buttonState_ = ButtonState::RELEASED;
                StopButtonTiming();
                log_t("Button press ended");
            } else {
                // Check for long press during hold
                unsigned long pressDuration = currentTime - buttonPressStartTime_;
                if (pressDuration >= LONG_PRESS_MIN_MS) {
                    buttonState_ = ButtonState::LONG_PRESS_TRIGGERED;
                    log_t("Long press triggered during hold");
                }
            }
            break;

        case ButtonState::LONG_PRESS_TRIGGERED:
            if (!currentPressed) {
                buttonState_ = ButtonState::IDLE; // Skip RELEASED state for long press
                buttonPressEndTime_ = currentTime;
            }
            break;

        case ButtonState::RELEASED:
            buttonState_ = ButtonState::IDLE;
            break;
    }

    // Debug every 2 seconds or on state changes (reduced from 1 second)
    if ((currentTime - lastDebugTime > 2000) || (buttonState_ != previousState)) {
        log_i("Button state: %s, raw_gpio=%s, sensor_valid=%s",
              StateToString(buttonState_),
              currentPressed ? "HIGH" : "LOW",
              buttonSensor_ ? "YES" : "NO");
        lastDebugTime = currentTime;
    }
}

const char* ActionHandler::StateToString(ButtonState state) const {
    switch (state) {
        case ButtonState::IDLE: return "IDLE";
        case ButtonState::PRESSED: return "PRESSED";
        case ButtonState::LONG_PRESS_TRIGGERED: return "LONG_TRIGGERED";
        case ButtonState::RELEASED: return "RELEASED";
        default: return "UNKNOWN";
    }
}

void ActionHandler::ProcessButtonEvents() {
    // Reset triggered flags when button transitions to idle
    if (buttonState_ == ButtonState::IDLE) {
        for (size_t i = 0; i < actionCount_; i++) {
            actions_[i].hasTriggered = false;
        }
    }
}

ButtonAction ActionHandler::DetectButtonAction() {
    // Only detect action when button is in RELEASED state
    if (buttonState_ != ButtonState::RELEASED || buttonPressEndTime_ == 0) {
        return ButtonAction::NONE;
    }

    unsigned long pressDuration = buttonPressEndTime_ - buttonPressStartTime_;
    ButtonAction action = CalculateButtonAction(pressDuration);

    log_i("DetectButtonAction: Duration=%lu ms, Action=%s", pressDuration,
          action == ButtonAction::SHORT_PRESS ? "SHORT_PRESS" :
          action == ButtonAction::LONG_PRESS ? "LONG_PRESS" : "NONE");

    return action;
}

ButtonAction ActionHandler::DetectLongPressDuringHold() {
    // Check for long press during button hold (state machine handles this better now)
    if (buttonState_ == ButtonState::LONG_PRESS_TRIGGERED) {
        unsigned long currentTime = millis();
        unsigned long pressDuration = currentTime - buttonPressStartTime_;
        log_i("DetectLongPressDuringHold: Long press detected during hold at %lu ms", pressDuration);
        return ButtonAction::LONG_PRESS;
    }

    return ButtonAction::NONE;
}

void ActionHandler::StartButtonTiming() {
    buttonPressStartTime_ = millis();
    buttonPressEndTime_ = 0;
}

void ActionHandler::StopButtonTiming() {
    buttonPressEndTime_ = millis();
    unsigned long duration = buttonPressEndTime_ - buttonPressStartTime_;
}

ButtonAction ActionHandler::CalculateButtonAction(unsigned long pressDuration) {
    if (pressDuration < MIN_PRESS_DURATION_MS) {
        return ButtonAction::NONE;
    }
    else if (pressDuration <= SHORT_PRESS_MAX_MS) {
        return ButtonAction::SHORT_PRESS;
    }
    else if (pressDuration >= LONG_PRESS_MIN_MS) {
        // Any press >= 1500ms is a long press when released
        return ButtonAction::LONG_PRESS;
    }
    else {
        // This should never be reached since we cover all cases above
        log_w("Unexpected button press duration: %lu ms", pressDuration);
        return ButtonAction::NONE;
    }
}

bool ActionHandler::IsButtonPressed() const {
    if (!buttonSensor_) {
        return false;
    }
    
    // Get button state from sensor - assuming bool reading
    Reading reading = buttonSensor_->GetReading();
    if (std::holds_alternative<bool>(reading)) {
        bool pressed = std::get<bool>(reading);
        return pressed;
    }
    
    // Check what type the reading actually is
    if (std::holds_alternative<int32_t>(reading)) {
        log_w("IsButtonPressed: Got int32_t reading instead of bool: %d", std::get<int32_t>(reading));
    } else if (std::holds_alternative<std::monostate>(reading)) {
        log_w("IsButtonPressed: Got monostate reading (uninitialized)");
    } else {
        log_w("IsButtonPressed: Got unexpected reading type");
    }
    
    return false;
}

Action* ActionHandler::FindAction(const char* id) {
    for (size_t i = 0; i < actionCount_; i++) {
        if (strcmp(actions_[i].id, id) == 0) {
            return &actions_[i];
        }
    }
    return nullptr;
}

// Function injection system
void ActionHandler::UpdatePanelFunctions(void (*shortPressFunc)(void*), void (*longPressFunc)(void*), void* context) {
    currentShortPressFunc_ = shortPressFunc;
    currentLongPressFunc_ = longPressFunc;
    currentPanelContext_ = context;
    // Panel functions updated
}

void ActionHandler::ClearPanelFunctions() {
    currentShortPressFunc_ = nullptr;
    currentLongPressFunc_ = nullptr;
    currentPanelContext_ = nullptr;
}

// Handler interface implementation complete

// Status and diagnostics
size_t ActionHandler::GetActionCount() const {
    return actionCount_;
}

bool ActionHandler::HasPendingActions() const {
    // Check if any action is waiting to be triggered
    ButtonAction currentAction = const_cast<ActionHandler*>(this)->DetectButtonAction();
    return currentAction != ButtonAction::NONE;
}

void ActionHandler::PrintActionStatus() const {
    log_i("ActionHandler Status:");
    log_i("  Total actions: %d", actionCount_);
    log_i("  Button state: %s", StateToString(buttonState_));
    log_i("  Timing: start=%lu, end=%lu", buttonPressStartTime_, buttonPressEndTime_);
    log_i("  Functions: short=%s, long=%s",
          currentShortPressFunc_ ? "set" : "null", currentLongPressFunc_ ? "set" : "null");

    for (size_t i = 0; i < actionCount_; i++) {
        const Action& action = actions_[i];
        log_i("  Action[%d]: id='%s', pressType=%d, triggered=%s",
              i, action.id, static_cast<int>(action.pressType),
              action.hasTriggered ? "yes" : "no");
    }
}