#include "handlers/action_handler.h"
#include "managers/error_manager.h"
#include "sensors/button_sensor.h"
#include "hardware/gpio_pins.h"
#include <Arduino.h>
#include <algorithm>
#include <cstring>

#include "esp32-hal-log.h"

ActionHandler::ActionHandler(IGpioProvider* gpioProvider) 
    : gpioProvider_(gpioProvider),
      actionCount_(0),
      buttonPressed_(false),
      buttonPreviouslyPressed_(false),
      buttonPressStartTime_(0),
      buttonPressEndTime_(0)
{
    log_v("ActionHandler() constructor called");
    
    // Create and initialize ButtonSensor owned by this handler
    if (gpioProvider_) {
        log_d("Creating ActionHandler-owned ButtonSensor");
        
        buttonSensor_ = std::make_unique<ButtonSensor>(gpioProvider_);
        buttonSensor_->Init();
        
        log_i("ActionHandler created and initialized ButtonSensor");
    }
    else {
        log_w("ActionHandler created without GPIO provider - ButtonSensor not initialized");
    }
}

ActionHandler::~ActionHandler() {
    log_v("ActionHandler destructor called");
}

void ActionHandler::Process() {
    // Actions are evaluated every main loop cycle (continuous evaluation)
    UpdateButtonState();
    ProcessButtonEvents();
    EvaluateActions();
    
    // Execute queued actions (this will be called during UI IDLE by InterruptManager)
    ExecuteQueuedActions();
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
    
    log_i("Registered action '%s' (press type: %d)", 
          action.id, static_cast<int>(action.pressType));
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
    for (size_t i = 0; i < actionCount_; i++) {
        EvaluateIndividualAction(actions_[i]);
    }
}

void ActionHandler::EvaluateIndividualAction(Action& action) {
    if (ShouldTriggerAction(action)) {
        action.hasTriggered = true;
        log_d("Action '%s' queued", action.id);
    }
}

bool ActionHandler::ShouldTriggerAction(const Action& action) {
    // Check if we have a button event that matches this action's press type
    ButtonAction detectedAction = DetectButtonAction();
    
    if (detectedAction == ButtonAction::NONE) {
        return false;
    }
    
    // Match detected button action to action press type
    if (action.pressType == ActionPress::SHORT && detectedAction == ButtonAction::SHORT_PRESS) {
        return !action.hasTriggered;  // Only trigger once per press event
    }
    else if (action.pressType == ActionPress::LONG && detectedAction == ButtonAction::LONG_PRESS) {
        return !action.hasTriggered;  // Only trigger once per press event
    }
    
    return false;
}

void ActionHandler::ExecuteQueuedActions() {
    // Process all queued actions using the documented Execute method
    for (size_t i = 0; i < actionCount_; i++) {
        Action& action = actions_[i];
        if (action.hasTriggered) {
            // Use injected panel functions if available, otherwise use action's own function
            if (action.pressType == ActionPress::SHORT && currentShortPressFunc_) {
                log_d("Executing injected short press function for action '%s'", action.id);
                currentShortPressFunc_();
                action.hasTriggered = false;  // Clear manually since we're using injected function
            }
            else if (action.pressType == ActionPress::LONG && currentLongPressFunc_) {
                log_d("Executing injected long press function for action '%s'", action.id);
                currentLongPressFunc_();
                action.hasTriggered = false;  // Clear manually since we're using injected function
            }
            else {
                // Use the documented Execute method
                action.Execute();  // This will clear hasTriggered automatically
            }
        }
    }
}

void ActionHandler::ExecuteAction(const Action& action) {
    // Legacy method - keeping for backward compatibility
    // Check if we should use panel function or action's own function
    if (action.pressType == ActionPress::SHORT && currentShortPressFunc_) {
        log_d("Executing injected short press function for action '%s'", action.id);
        currentShortPressFunc_();
    }
    else if (action.pressType == ActionPress::LONG && currentLongPressFunc_) {
        log_d("Executing injected long press function for action '%s'", action.id);
        currentLongPressFunc_();
    }
    else if (action.executeFunc) {
        log_d("Executing action function for action '%s'", action.id);
        action.executeFunc();
    }
    else {
        log_w("No function to execute for action '%s'", action.id);
    }
}

void ActionHandler::UpdateButtonState() {
    buttonPreviouslyPressed_ = buttonPressed_;
    buttonPressed_ = IsButtonPressed();
    
    // Detect button press start
    if (!buttonPreviouslyPressed_ && buttonPressed_) {
        StartButtonTiming();
    }
    // Detect button press end
    else if (buttonPreviouslyPressed_ && !buttonPressed_) {
        StopButtonTiming();
    }
}

void ActionHandler::ProcessButtonEvents() {
    // This method processes any queued button events
    // The timing logic is handled in UpdateButtonState() and DetectButtonAction()
    // Reset triggered flags when button is released
    if (buttonPreviouslyPressed_ && !buttonPressed_) {
        for (size_t i = 0; i < actionCount_; i++) {
            actions_[i].hasTriggered = false;
        }
    }
}

ButtonAction ActionHandler::DetectButtonAction() {
    // Only detect action when button is released after a press
    if (!buttonPreviouslyPressed_ || buttonPressed_ || buttonPressEndTime_ == 0) {
        return ButtonAction::NONE;
    }
    
    unsigned long pressDuration = buttonPressEndTime_ - buttonPressStartTime_;
    ButtonAction action = CalculateButtonAction(pressDuration);
    
    // Clear the timing after detection to prevent multiple triggers
    if (action != ButtonAction::NONE) {
        buttonPressStartTime_ = 0;
        buttonPressEndTime_ = 0;
    }
    
    return action;
}

void ActionHandler::StartButtonTiming() {
    buttonPressStartTime_ = millis();
    buttonPressEndTime_ = 0;
    log_d("Button press started at %lu ms", buttonPressStartTime_);
}

void ActionHandler::StopButtonTiming() {
    buttonPressEndTime_ = millis();
    unsigned long duration = buttonPressEndTime_ - buttonPressStartTime_;
    log_d("Button press ended at %lu ms, duration: %lu ms", buttonPressEndTime_, duration);
}

ButtonAction ActionHandler::CalculateButtonAction(unsigned long pressDuration) {
    if (pressDuration < MIN_PRESS_DURATION_MS) {
        log_d("Button press too short: %lu ms (min: %lu ms)", pressDuration, MIN_PRESS_DURATION_MS);
        return ButtonAction::NONE;
    }
    else if (pressDuration <= SHORT_PRESS_MAX_MS) {
        log_d("Short press detected: %lu ms", pressDuration);
        return ButtonAction::SHORT_PRESS;
    }
    else if (pressDuration >= LONG_PRESS_MIN_MS && pressDuration <= LONG_PRESS_MAX_MS) {
        log_d("Long press detected: %lu ms", pressDuration);
        return ButtonAction::LONG_PRESS;
    }
    else {
        log_d("Button press too long: %lu ms (max: %lu ms)", pressDuration, LONG_PRESS_MAX_MS);
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
        return std::get<bool>(reading);
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
void ActionHandler::UpdatePanelFunctions(void (*shortPressFunc)(), void (*longPressFunc)()) {
    currentShortPressFunc_ = shortPressFunc;
    currentLongPressFunc_ = longPressFunc;
    log_d("Panel functions updated - short: %s, long: %s",
          shortPressFunc ? "set" : "null", longPressFunc ? "set" : "null");
}

void ActionHandler::ClearPanelFunctions() {
    currentShortPressFunc_ = nullptr;
    currentLongPressFunc_ = nullptr;
    log_d("Panel functions cleared");
}

// Legacy compatibility methods
void ActionHandler::RegisterInterrupt(struct Interrupt* interrupt) {
    log_w("ActionHandler::RegisterInterrupt() called - legacy compatibility only");
    // Could convert Interrupt to Action format if needed
}

void ActionHandler::UnregisterInterrupt(const char* id) {
    log_w("ActionHandler::UnregisterInterrupt() called - legacy compatibility only");
    UnregisterAction(id);
}

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
    log_i("  Button state: pressed=%s, previouslyPressed=%s", 
          buttonPressed_ ? "yes" : "no", buttonPreviouslyPressed_ ? "yes" : "no");
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