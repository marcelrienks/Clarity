#include "handlers/action_handler.h"
#include "managers/error_manager.h"
#include "sensors/button_sensor.h"
#include "hardware/gpio_pins.h"
#include <Arduino.h>
#include <algorithm>
#include <cstring>

#include "esp32-hal-log.h"
#include "utilities/logging.h"

/**
 * @brief Constructs ActionHandler with GPIO provider and initializes button sensor
 * @param gpioProvider GPIO provider for hardware button access
 *
 * Creates ActionHandler instance that owns and manages a ButtonSensor for detecting
 * button press events. Initializes internal state machine and timing variables
 * for precise button action detection.
 */
ActionHandler::ActionHandler(IGpioProvider* gpioProvider)
    : gpioProvider_(gpioProvider),
      actionCount_(0),
      buttonState_(ButtonState::IDLE),
      buttonPressStartTime_(0),
      buttonPressEndTime_(0)
{
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

/**
 * @brief Destructor cleans up ActionHandler resources
 *
 * ButtonSensor is automatically cleaned up through unique_ptr. No manual
 * cleanup required for GPIO resources as they're managed by the provider.
 */
ActionHandler::~ActionHandler() {
}

/**
 * @brief Main processing loop for action detection and execution
 *
 * Called every main loop cycle to provide continuous button monitoring.
 * Updates button state machine, processes button events, evaluates registered
 * actions against detected button events, and executes any triggered actions.
 * Designed for real-time responsiveness in automotive applications.
 */
void ActionHandler::Process() {
    static unsigned long lastProcessTime = 0;
    unsigned long currentTime = millis();
    
    
    // Actions are evaluated every main loop cycle (continuous evaluation)
    UpdateButtonState();
    ProcessButtonEvents();
    EvaluateActions();
    
    // Execute pending actions (this will be called during UI IDLE by InterruptManager)
    ExecutePendingActions();
}

/**
 * @brief Registers a button action for monitoring and execution
 * @param action Action configuration including ID, press type, and function
 * @return true if registration successful, false if duplicate ID or array full
 *
 * Adds action to internal registry for monitoring. Each action specifies
 * a button press type (short/long) and execution function. Prevents duplicate
 * IDs and enforces maximum action limit for memory safety.
 */
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

/**
 * @brief Removes a registered action from monitoring
 * @param id Unique identifier of action to remove
 *
 * Finds and removes action from registry, compacting the array to maintain
 * sequential storage. Used during panel transitions to clean up previous
 * panel's button behaviors and register new ones.
 */
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

/**
 * @brief Evaluates all registered actions against current button state
 *
 * Core action detection logic that examines button events and determines
 * which registered actions should be triggered. Handles both completed
 * press events and long press detection during button hold. Prevents
 * duplicate triggering by clearing timing data after evaluation.
 */
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

/**
 * @brief Evaluates single action for trigger condition
 * @param action Action to evaluate for triggering
 *
 * Legacy method that evaluates individual action using current button state.
 * Marks action as triggered if conditions are met. Superseded by the more
 * efficient EvaluateIndividualActionWithDetectedAction method.
 */
void ActionHandler::EvaluateIndividualAction(Action& action) {
    if (ShouldTriggerAction(action)) {
        action.hasTriggered = true;
    }
}

/**
 * @brief Evaluates single action against specific detected button action
 * @param action Action to evaluate for triggering
 * @param detectedAction The button action that was detected
 * @return true if action was triggered, false otherwise
 *
 * Optimized evaluation method that uses pre-detected button action rather
 * than re-detecting. Improves performance and ensures consistency across
 * all action evaluations in the same processing cycle.
 */
bool ActionHandler::EvaluateIndividualActionWithDetectedAction(Action& action, ButtonAction detectedAction) {
    if (ShouldTriggerActionWithDetectedAction(action, detectedAction)) {
        action.hasTriggered = true;
        return true;
    }
    return false;
}

/**
 * @brief Determines if action should be triggered based on current button state
 * @param action Action to check for trigger conditions
 * @return true if action should be triggered
 *
 * Legacy trigger evaluation that detects current button action and delegates
 * to the optimized ShouldTriggerActionWithDetectedAction method.
 */
bool ActionHandler::ShouldTriggerAction(const Action& action) {
    // Check if we have a button event that matches this action's press type
    ButtonAction detectedAction = DetectButtonAction();
    return ShouldTriggerActionWithDetectedAction(action, detectedAction);
}

/**
 * @brief Determines if action should trigger for specific detected button action
 * @param action Action to evaluate
 * @param detectedAction Button action that was detected
 * @return true if action should be triggered
 *
 * Core trigger logic that matches action press type with detected button action.
 * Ensures actions only trigger once per button event and provides appropriate
 * logging for automotive diagnostics.
 */
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

/**
 * @brief Executes all actions that have been triggered
 *
 * Processes all registered actions that are marked as triggered, executing
 * either injected panel functions or the action's own execution function.
 * Called during UI idle periods to ensure smooth user experience. Clears
 * trigger flags after execution to prevent duplicate execution.
 */
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

/**
 * @brief Executes a specific action immediately
 * @param action Action to execute
 *
 * Direct action execution that uses either injected panel functions or
 * the action's own execution function. Provides immediate execution without
 * waiting for the normal pending action processing cycle.
 */
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

/**
 * @brief Updates button state machine based on current GPIO reading
 *
 * Core state machine that tracks button press lifecycle from IDLE through
 * PRESSED to RELEASED or LONG_PRESS_TRIGGERED. Handles timing capture for
 * press duration calculation and provides debug logging for automotive
 * diagnostics. Essential for reliable button event detection.
 */
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

/**
 * @brief Converts button state enum to human-readable string
 * @param state Button state to convert
 * @return String representation of the state
 *
 * Utility function for debug logging and diagnostic output. Provides
 * clear state names for troubleshooting button behavior issues.
 */
const char* ActionHandler::StateToString(ButtonState state) const {
    switch (state) {
        case ButtonState::IDLE: return "IDLE";
        case ButtonState::PRESSED: return "PRESSED";
        case ButtonState::LONG_PRESS_TRIGGERED: return "LONG_TRIGGERED";
        case ButtonState::RELEASED: return "RELEASED";
        default: return "UNKNOWN";
    }
}

/**
 * @brief Processes button state transitions and manages action flags
 *
 * Handles state-dependent event processing, particularly resetting triggered
 * flags when button returns to IDLE state. Ensures actions can be triggered
 * again for subsequent button presses.
 */
void ActionHandler::ProcessButtonEvents() {
    // Reset triggered flags when button transitions to idle
    if (buttonState_ == ButtonState::IDLE) {
        for (size_t i = 0; i < actionCount_; i++) {
            actions_[i].hasTriggered = false;
        }
    }
}

/**
 * @brief Detects completed button action based on press duration
 * @return ButtonAction type (SHORT_PRESS, LONG_PRESS, or NONE)
 *
 * Analyzes completed button press (RELEASED state) to determine action type
 * based on press duration timing. Only triggers when button has been released
 * to ensure accurate duration measurement for automotive reliability.
 */
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

/**
 * @brief Detects long press action while button is still held
 * @return ButtonAction::LONG_PRESS if detected, otherwise NONE
 *
 * Provides immediate long press detection without waiting for button release.
 * Critical for responsive user interface in automotive applications where
 * immediate feedback is expected for long press actions.
 */
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

/**
 * @brief Captures button press start time for duration calculation
 *
 * Records timestamp when button press begins, resetting end time to ensure
 * clean timing state. Essential for accurate press duration measurement
 * used in short/long press classification.
 */
void ActionHandler::StartButtonTiming() {
    buttonPressStartTime_ = millis();
    buttonPressEndTime_ = 0;
}

/**
 * @brief Captures button press end time for duration calculation
 *
 * Records timestamp when button press ends, enabling calculation of total
 * press duration. Used in conjunction with start time to classify button
 * actions as short or long presses.
 */
void ActionHandler::StopButtonTiming() {
    buttonPressEndTime_ = millis();
    unsigned long duration = buttonPressEndTime_ - buttonPressStartTime_;
}

/**
 * @brief Calculates button action type from press duration
 * @param pressDuration Duration of button press in milliseconds
 * @return ButtonAction classification based on timing thresholds
 *
 * Core classification logic that converts press duration into action type.
 * Uses automotive-appropriate timing thresholds: 50-2000ms for short press,
 * 2000ms+ for long press. Handles edge cases and provides diagnostic logging.
 */
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

/**
 * @brief Checks current button state from hardware sensor
 * @return true if button is currently pressed, false otherwise
 *
 * Reads current button state through ButtonSensor, handling different
 * reading types and providing diagnostic logging for troubleshooting.
 * Essential for real-time button state monitoring in the state machine.
 */
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

/**
 * @brief Finds registered action by unique identifier
 * @param id Unique identifier of action to find
 * @return Pointer to action if found, nullptr otherwise
 *
 * Linear search through registered actions array to locate action by ID.
 * Used for action management operations like unregistration and duplicate
 * prevention during registration.
 */
Action* ActionHandler::FindAction(const char* id) {
    for (size_t i = 0; i < actionCount_; i++) {
        if (strcmp(actions_[i].id, id) == 0) {
            return &actions_[i];
        }
    }
    return nullptr;
}

// Function injection system
/**
 * @brief Updates injected panel functions for button actions
 * @param shortPressFunc Function to call for short press actions
 * @param longPressFunc Function to call for long press actions
 * @param context Panel context pointer passed to functions
 *
 * Function injection system that allows panels to override default action
 * execution. Enables dynamic button behavior changes during panel transitions
 * while maintaining consistent action registration.
 */
void ActionHandler::UpdatePanelFunctions(void (*shortPressFunc)(void*), void (*longPressFunc)(void*), void* context) {
    currentShortPressFunc_ = shortPressFunc;
    currentLongPressFunc_ = longPressFunc;
    currentPanelContext_ = context;
    // Panel functions updated
}

/**
 * @brief Clears injected panel functions, reverting to action defaults
 *
 * Resets function injection system to use actions' own execution functions.
 * Called during panel cleanup to ensure clean state for next panel.
 * Prevents accidental execution of previous panel's functions.
 */
void ActionHandler::ClearPanelFunctions() {
    currentShortPressFunc_ = nullptr;
    currentLongPressFunc_ = nullptr;
    currentPanelContext_ = nullptr;
}

// Handler interface implementation complete

// Status and diagnostics
/**
 * @brief Gets current number of registered actions
 * @return Number of actions currently registered
 *
 * Provides count of registered actions for diagnostic purposes and
 * capacity management. Useful for debugging action registration issues.
 */
size_t ActionHandler::GetActionCount() const {
    return actionCount_;
}

/**
 * @brief Checks if there are actions waiting to be executed
 * @return true if button actions are detected and pending execution
 *
 * Determines if current button state indicates pending action execution.
 * Used by InterruptManager to coordinate action execution during UI idle
 * periods for optimal user experience.
 */
bool ActionHandler::HasPendingActions() const {
    // Check if any action is waiting to be triggered
    ButtonAction currentAction = const_cast<ActionHandler*>(this)->DetectButtonAction();
    return currentAction != ButtonAction::NONE;
}

/**
 * @brief Prints comprehensive diagnostic information about action handler state
 *
 * Outputs detailed status including registered actions, button state, timing
 * information, and function injection status. Essential for debugging button
 * behavior issues and verifying proper action handler operation.
 */
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