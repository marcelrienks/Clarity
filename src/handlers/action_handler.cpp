#include "handlers/action_handler.h"
#include "interfaces/i_action_service.h"
#include "interfaces/i_preference_service.h"
#include "managers/error_manager.h"
#include "sensors/button_sensor.h"
#include "hardware/gpio_pins.h"
#include "definitions/constants.h"
#include <Arduino.h>
#include <algorithm>
#include <cstring>

#include "esp32-hal-log.h"
#include "utilities/logging.h"
#include "managers/configuration_manager.h"

// Self-registration at program startup
static bool action_handler_config_registered = []() {
    ConfigurationManager::AddSchema(ActionHandler::RegisterConfigSchema);
    return true;
}();

// ========== Constructors and Destructor ==========

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
      pendingActionType_(ButtonAction::NONE),
      hasPendingAction_(false),
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
        log_e("ActionHandler created without GPIO provider - ButtonSensor not initialized. Application will not function correctly!");
        ErrorManager::Instance().ReportCriticalError("ActionHandler",
                                                     "Created without GPIO provider - button input will not function");
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

// ========== Public Interface Methods ==========

/**
 * @brief Main processing loop for action detection and execution
 *
 * Called every main loop cycle to provide continuous button monitoring.
 * Updates button state machine, processes button events, detects button
 * actions, and executes any pending action.
 * Designed for real-time responsiveness in automotive applications.
 */
void ActionHandler::Process() {
    // Update button state machine
    UpdateButtonState();

    // Process button events and detect action type
    ProcessButtonEvents();

    // Execute any pending action immediately
    if (hasPendingAction_) {
        ExecutePendingAction();
    }
}

// REMOVED: RegisterAction method - no longer using action registry
// Actions are now handled directly through panel methods

/**
 * @brief Sets the pending action, replacing any existing one
 * @param actionType The type of button action to set as pending
 *
 * Implements LIFO behavior with size 1 - only the most recent action
 * is kept. This ensures we always execute the latest user input.
 */
void ActionHandler::SetPendingAction(ButtonAction actionType) {
    pendingActionType_ = actionType;
    hasPendingAction_ = true;

    log_i("Set pending action: %s",
          actionType == ButtonAction::SHORT_PRESS ? UIStrings::ButtonActionStrings::SHORT_PRESS :
          actionType == ButtonAction::LONG_PRESS ? UIStrings::ButtonActionStrings::LONG_PRESS : UIStrings::ButtonActionStrings::NONE);
}

/**
 * @brief Clears the pending action after execution
 */
void ActionHandler::ClearPendingAction() {
    pendingActionType_ = ButtonAction::NONE;
    hasPendingAction_ = false;
}

/**
 * @brief Checks if there's a pending action to execute
 * @return true if an action is pending
 */
bool ActionHandler::HasPendingAction() const {
    return hasPendingAction_;
}

/**
 * @brief Executes the single pending action
 *
 * Executes the pending action through the current panel's methods.
 * Only one action is pending at a time (LIFO with size 1), ensuring
 * we always execute the most recent user input. Clears the pending
 * action after execution.
 */
void ActionHandler::ExecutePendingAction() {
    if (!hasPendingAction_ || !currentPanel_) {
        return;
    }

    log_i("ExecutePendingAction: Executing %s press",
          pendingActionType_ == ButtonAction::SHORT_PRESS ? UIStrings::ButtonActionStrings::SHORT : UIStrings::ButtonActionStrings::LONG);

    // Execute through panel methods
    if (pendingActionType_ == ButtonAction::SHORT_PRESS) {
        currentPanel_->HandleShortPress();
    } else if (pendingActionType_ == ButtonAction::LONG_PRESS) {
        currentPanel_->HandleLongPress();
    }

    // Clear pending action after execution
    ClearPendingAction();
}

/**
 * @brief Evaluates all registered actions against current button state
 *
 * Core action detection logic that examines button events and determines
 * which registered actions should be triggered. Handles both completed
 * press events and long press detection during button hold. Prevents
 * duplicate triggering by clearing timing data after evaluation.
 */
// void ActionHandler::EvaluateActions() {
//     // Process each action for trigger conditions
//     
//     // Check if there's a valid button action to evaluate
//     ButtonAction detectedAction = DetectButtonAction();
//     
//     // Also check for long press during button hold (without waiting for release)
//     if (detectedAction == ButtonAction::NONE) {
//         detectedAction = DetectLongPressDuringHold();
//     }
//     
//     bool anyActionTriggered = false;
//     
//     for (size_t i = 0; i < actionCount_; i++) {
//         // Evaluating action
//         
//         if (EvaluateIndividualActionWithDetectedAction(actions_[i], detectedAction)) {
//             anyActionTriggered = true;
//         }
//     }
//     
//     // Clear timing data after all actions have been evaluated to prevent duplicate triggers
//     if (detectedAction != ButtonAction::NONE && anyActionTriggered) {
//         buttonPressStartTime_ = 0;
//         buttonPressEndTime_ = 0;
//     }
// }
// 
// /**
//  * @brief Evaluates single action for trigger condition
//  * @param action Action to evaluate for triggering
//  *
//  * Legacy method that evaluates individual action using current button state.
//  * Marks action as triggered if conditions are met. Superseded by the more
//  * efficient EvaluateIndividualActionWithDetectedAction method.
//  */
// void ActionHandler::EvaluateIndividualAction(Action& action) {
//     if (ShouldTriggerAction(action)) {
//         action.hasTriggered = true;
//     }
// }
// 
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
// bool ActionHandler::EvaluateIndividualActionWithDetectedAction(Action& action, ButtonAction detectedAction) {
//     if (ShouldTriggerActionWithDetectedAction(action, detectedAction)) {
//         action.hasTriggered = true;
//         return true;
//     }
//     return false;
// }
// 
/**
 * @brief Determines if action should be triggered based on current button state
 * @param action Action to check for trigger conditions
 * @return true if action should be triggered
 *
 * Legacy trigger evaluation that detects current button action and delegates
 * to the optimized ShouldTriggerActionWithDetectedAction method.
 */
// bool ActionHandler::ShouldTriggerAction(const Action& action) {
//     // Check if we have a button event that matches this action's press type
//     ButtonAction detectedAction = DetectButtonAction();
//     return ShouldTriggerActionWithDetectedAction(action, detectedAction);
// }

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
// bool ActionHandler::ShouldTriggerActionWithDetectedAction(const Action& action, ButtonAction detectedAction) {
//     if (detectedAction == ButtonAction::NONE) {
//         return false;
//     }
//     
//     // Action evaluation completed
//     
//     // Match detected button action to action press type
//     if (action.pressType == ActionPress::SHORT && detectedAction == ButtonAction::SHORT_PRESS) {
//         bool shouldTrigger = !action.hasTriggered;
//         if (shouldTrigger) log_t("Button action detected: SHORT_PRESS");
//         return shouldTrigger;  // Only trigger once per press event
//     }
//     else if (action.pressType == ActionPress::LONG && detectedAction == ButtonAction::LONG_PRESS) {
//         bool shouldTrigger = !action.hasTriggered;
//         if (shouldTrigger) log_t("Button action detected: LONG_PRESS");
//         return shouldTrigger;  // Only trigger once per press event
//     }
//
//     return false;
// }

/**
 * @brief Executes all actions that have been triggered
 *
 * Processes all registered actions that are marked as triggered, executing
 * either injected panel functions or the action's own execution function.
 * Called during UI idle periods to ensure smooth user experience. Clears
 * trigger flags after execution to prevent duplicate execution.
 */
// void ActionHandler::ExecutePendingActions() {
//     // Process all pending actions using the documented Execute method
//     for (size_t i = 0; i < actionCount_; i++) {
//         Action& action = actions_[i];
//         if (action.hasTriggered) {
//             log_i("ExecutePendingActions: Executing triggered action '%s' (type: %s)", 
//                   action.id, action.pressType == ActionPress::SHORT ? "SHORT" : "LONG");
//             
//             // Use current panel methods if available, otherwise use action's own function
//             if (action.pressType == ActionPress::SHORT && currentPanel_) {
//                 log_i("Executing panel SHORT press method for action '%s'", action.id);
//                 currentPanel_->HandleShortPress();
//                 action.hasTriggered = false;  // Clear manually since we're using panel method
//             }
//             else if (action.pressType == ActionPress::LONG && currentPanel_) {
//                 log_i("Executing panel LONG press method for action '%s'", action.id);
//                 currentPanel_->HandleLongPress();
//                 action.hasTriggered = false;  // Clear manually since we're using panel method
//             }
//             else {
//                 log_i("Using action's own execute function for '%s'", action.id);
//                 // Use the documented Execute method
//                 action.Execute();  // This will clear hasTriggered automatically
//             }
//         }
//     }
// }
// 
// /**
//  * @brief Executes a specific action immediately
//  * @param action Action to execute
//  *
//  * Direct action execution that uses either injected panel functions or
//  * the action's own execution function. Provides immediate execution without
//  * waiting for the normal pending action processing cycle.
//  */
// void ActionHandler::ExecuteAction(const Action& action) {
//     // Execute action based on press type
//     // Check if we should use panel function or action's own function
//     if (action.pressType == ActionPress::SHORT && currentShortPressFunc_) {
//         currentShortPressFunc_(currentPanelContext_);
//     }
//     else if (action.pressType == ActionPress::LONG && currentLongPressFunc_) {
//         currentLongPressFunc_(currentPanelContext_);
//     }
//     else if (action.executeFunc) {
//         action.executeFunc();
//     }
//     else {
//         log_w("No function to execute for action '%s'", action.id);
//     }
// }
// 
// /**
//  * @brief Updates button state machine based on current GPIO reading
//  *
//  * Core state machine that tracks button press lifecycle from IDLE through
//  * PRESSED to RELEASED or LONG_PRESS_TRIGGERED. Handles timing capture for
//  * press duration calculation and provides debug logging for automotive
//  * diagnostics. Essential for reliable button event detection.
// */
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
                unsigned long longPressThreshold = GetLongPressMs();
                log_v("Checking for long press: duration=%lu ms, threshold=%lu ms", pressDuration, longPressThreshold);
                if (pressDuration >= longPressThreshold) {
                    buttonState_ = ButtonState::LONG_PRESS_TRIGGERED;
                    log_t("Long press triggered during hold");
                    // Immediately set the pending action when long press is detected
                    SetPendingAction(ButtonAction::LONG_PRESS);
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
 * @brief Processes button state changes and sets pending action
 *
 * Detects button actions and stores them as pending. Only the most recent
 * action is kept (LIFO with size 1). Called every loop cycle to ensure
 * responsive button handling.
 */
void ActionHandler::ProcessButtonEvents() {
    // ButtonSensor now handles all action detection including long press during hold
    ButtonAction detectedAction = DetectButtonAction();

    // Set pending action (replaces any existing pending action)
    if (detectedAction != ButtonAction::NONE) {
        SetPendingAction(detectedAction);
    }
}

/**
 * @brief Detects completed button action using ButtonSensor detection
 * @return ButtonAction type (SHORT_PRESS, LONG_PRESS, or NONE)
 *
 * Uses the ButtonSensor's built-in action detection and consumption to avoid
 * duplicate action processing. This ensures actions are only processed once
 * and prevents double-triggering when long press is detected during hold.
 */
ButtonAction ActionHandler::DetectButtonAction() {
    if (!buttonSensor_) {
        return ButtonAction::NONE;
    }

    // Use the button sensor's action detection and consume the action
    ButtonAction action = buttonSensor_->GetAndConsumeAction();

    if (action != ButtonAction::NONE) {
        log_i("DetectButtonAction: ButtonSensor reported action: %s",
              action == ButtonAction::SHORT_PRESS ? UIStrings::ButtonActionStrings::SHORT_PRESS : UIStrings::ButtonActionStrings::LONG_PRESS);
    }

    return action;
}

/**
 * @brief Detects long press action while button is still held
 * @return ButtonAction::LONG_PRESS if detected, otherwise NONE
 *
 * Provides immediate long press detection without waiting for button release.
 * Critical for responsive user interface in automotive applications where
 * immediate feedback is expected for long press actions.
 * Also consumes any pending action from ButtonSensor to prevent double processing.
 */
ButtonAction ActionHandler::DetectLongPressDuringHold() {
    // Check for long press during button hold (state machine handles this better now)
    if (buttonState_ == ButtonState::LONG_PRESS_TRIGGERED) {
        unsigned long currentTime = millis();
        unsigned long pressDuration = currentTime - buttonPressStartTime_;
        log_i("DetectLongPressDuringHold: Long press detected during hold at %lu ms", pressDuration);

        // Consume any pending action from ButtonSensor to prevent double processing
        if (buttonSensor_) {
            ButtonAction sensorAction = buttonSensor_->GetAndConsumeAction();
            if (sensorAction != ButtonAction::NONE) {
                log_t("DetectLongPressDuringHold: Consumed ButtonSensor action to prevent double processing");
            }
        }

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
    unsigned long debounceMs = GetDebounceMs();
    unsigned long longPressMs = GetLongPressMs();

    if (pressDuration < debounceMs) {
        return ButtonAction::NONE;
    }
    else if (pressDuration < longPressMs) {
        return ButtonAction::SHORT_PRESS;
    }
    else {
        // Any press >= longPressMs is a long press when released
        return ButtonAction::LONG_PRESS;
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
// Action* ActionHandler::FindAction(const char* id) {
//     for (size_t i = 0; i < actionCount_; i++) {
//         if (strcmp(actions_[i].id, id) == 0) {
//             return &actions_[i];
//         }
//     }
//     return nullptr;
// }

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
void ActionHandler::SetCurrentPanel(IActionService* panel) {
    currentPanel_ = panel;
    log_i("Set current panel: %p", (void*)panel);
}

/**
 * @brief Clears injected panel functions, reverting to action defaults
 *
 * Resets function injection system to use actions' own execution functions.
 * Called during panel cleanup to ensure clean state for next panel.
 * Prevents accidental execution of previous panel's functions.
 */
void ActionHandler::ClearCurrentPanel() {
    currentPanel_ = nullptr;
    log_i("Cleared current panel");
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
// size_t ActionHandler::GetActionCount() const {
//     return actionCount_;
// }

/**
 * @brief Checks if there are actions waiting to be executed
 * @return true if button actions are detected and pending execution
 *
 * Determines if current button state indicates pending action execution.
 * Used by InterruptManager to coordinate action execution during UI idle
 * periods for optimal user experience.
 */
// bool ActionHandler::HasPendingActions() const {
//     // Check if any action is waiting to be triggered
//     ButtonAction currentAction = const_cast<ActionHandler*>(this)->DetectButtonAction();
//     return currentAction != ButtonAction::NONE;
// }

/**
 * @brief Registers button timing configuration schema with preference service
 * @param preferenceService The preference service to register with
 *
 * Registers configurable button press timing values including:
 * - Debounce timing: 300-700ms (default 500ms)
 * - Long press threshold: 1000-2000ms (default 1500ms)
 */
void ActionHandler::RegisterConfigSchema(IPreferenceService* preferenceService)
{
    if (!preferenceService) return;

    // Check if already registered to prevent duplicates
    if (preferenceService->IsSchemaRegistered(ConfigConstants::Sections::BUTTON_SENSOR)) {
        log_d("ActionHandler schema already registered");
        return;
    }

    using namespace Config;

    ConfigSection section(ConfigConstants::Sections::BUTTON_SENSOR, ConfigConstants::Sections::BUTTON_SENSOR, ConfigConstants::SectionNames::BUTTON_SENSOR);

    section.AddItem(debounceConfig_);
    section.AddItem(longPressConfig_);

    preferenceService->RegisterConfigSection(section);
    log_i("ActionHandler configuration schema registered (static)");
}

/**
 * @brief Sets preference service for configuration access
 * @param preferenceService The preference service to use for configuration
 */
void ActionHandler::SetPreferenceService(IPreferenceService* preferenceService)
{
    preferenceService_ = preferenceService;
}

/**
 * @brief Gets configured debounce time in milliseconds
 * @return Debounce time in ms, or default if config unavailable
 */
unsigned long ActionHandler::GetDebounceMs() const
{
    if (!preferenceService_) {
        return ConfigConstants::Defaults::DEFAULT_DEBOUNCE_MS;
    }

    auto value = preferenceService_->QueryConfig<int>(ConfigConstants::Keys::BUTTON_DEBOUNCE_MS);
    return value ? static_cast<unsigned long>(*value) : ConfigConstants::Defaults::DEFAULT_DEBOUNCE_MS;
}

/**
 * @brief Gets configured long press threshold in milliseconds
 * @return Long press threshold in ms, or default if config unavailable
 */
unsigned long ActionHandler::GetLongPressMs() const
{
    if (!preferenceService_) {
        log_w("No preference service, using default long press threshold: %d ms", ConfigConstants::Defaults::DEFAULT_LONG_PRESS_MS);
        return ConfigConstants::Defaults::DEFAULT_LONG_PRESS_MS;
    }

    auto value = preferenceService_->QueryConfig<int>(ConfigConstants::Keys::BUTTON_LONG_PRESS_MS);
    unsigned long result = value ? static_cast<unsigned long>(*value) : ConfigConstants::Defaults::DEFAULT_LONG_PRESS_MS;
    log_v("GetLongPressMs returning: %lu ms (config key: %s)", result, ConfigConstants::Keys::BUTTON_LONG_PRESS_MS);
    return result;
}