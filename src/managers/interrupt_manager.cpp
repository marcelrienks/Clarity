#include "managers/interrupt_manager.h"
#include "managers/error_manager.h"
#include "handlers/trigger_handler.h"
#include "handlers/action_handler.h"
#include "interfaces/i_panel_manager.h"
#include "definitions/interrupts.h"
#include "definitions/constants.h"
#include "definitions/enums.h"
#include "sensors/gpio_sensor.h"
#include <Arduino.h>
#include <cstring>
#include "esp32-hal-log.h"

// ========== Static Methods ==========

/**
 * @brief Singleton implementation using Meyer's singleton pattern
 * @details Thread-safe in C++11+ due to static local variable initialization
 */
InterruptManager& InterruptManager::Instance()
{
    static InterruptManager instance;
    return instance;
}

// ========== Public Interface Methods ==========

/**
 * @brief Initialize interrupt system with GPIO provider and create handlers
 * @details Sets up TriggerHandler and ActionHandler, registers system interrupts
 */
void InterruptManager::Init(IGpioProvider* gpioProvider)
{
    log_v("Init() called");
    if (initialized_)
    {
        log_w("InterruptManager already initialized");
        return;
    }

    if (!gpioProvider) {
        log_e("Cannot initialize InterruptManager - GPIO provider is null");
        ErrorManager::Instance().ReportCriticalError("InterruptManager",
                                                     "Cannot initialize - GPIO provider is null");
        return;
    }

    // Store GPIO provider reference
    gpioProvider_ = gpioProvider;


    // Create new Trigger/Action architecture handlers
    triggerHandler_ = std::make_shared<TriggerHandler>(gpioProvider);
    actionHandler_ = std::make_shared<ActionHandler>(gpioProvider);

    if (!triggerHandler_ || !actionHandler_)
    {
        log_e("Failed to create Trigger/Action handlers");
        ErrorManager::Instance().ReportError(ErrorLevel::ERROR, "InterruptManager",
                                           "Failed to create Trigger/Action handlers");
        return;
    }


    // Register all system-level triggers and actions
    RegisterSystemInterrupts();

    initialized_ = true;
    lastEvaluationTime_ = millis();

    log_i("InterruptManager initialized with new Trigger/Action architecture");
}

/**
 * @brief Process initial trigger states after system initialization
 * @details This should be called once after all triggers are registered but before
 * the initial panel is loaded to ensure proper system state at startup
 */
void InterruptManager::ProcessInitialTriggerStates()
{
    if (!initialized_) {
        log_e("Cannot process initial trigger states - InterruptManager not initialized");
        return;
    }

    if (!triggerHandler_) {
        log_e("Cannot process initial trigger states - TriggerHandler not initialized");
        return;
    }

    // Delegate to trigger handler to process initial states
    triggerHandler_->ProcessInitialTriggerStates();
}

/**
 * @brief Process all interrupts based on UI state and responsiveness requirements
 * @details Actions are processed continuously, triggers only during UI idle periods
 */
void InterruptManager::Process()
{
    if (!initialized_) {
        return;
    }

    // Process Action validation continuously for immediate responsiveness (button event detection)
    if (actionHandler_) {
        actionHandler_->ValidateActions();
    } else {
        log_e("InterruptManager::Process: ActionHandler is null - button input is non-functional!");
        ErrorManager::Instance().ReportCriticalError("InterruptManager",
                                                     "ActionHandler is null - button input system is broken");
    }

    // Process execution only during UI idle to avoid interfering with animations
    // Check application-level UI state instead of LVGL idle time
    // This ensures interrupts are processed between animations, not during them
    if (panelManager_ && panelManager_->GetUiState() == UIState::IDLE) {
        // Execute pending Actions first
        if (actionHandler_) {
            actionHandler_->ExecutePendingActions();
        } else {
            log_e("InterruptManager::Process: ActionHandler is null during execution phase!");
            ErrorManager::Instance().ReportCriticalError("InterruptManager",
                                                         "ActionHandler is null - cannot execute button actions");
        }

        // Then process Triggers
        if (triggerHandler_) {
            triggerHandler_->ProcessTriggers();
        } else {
            log_e("InterruptManager::Process: TriggerHandler is null - trigger system is non-functional!");
            ErrorManager::Instance().ReportCriticalError("InterruptManager",
                                                         "TriggerHandler is null - trigger system is broken");
        }
    }
}

/**
 * @brief Register trigger condition with TriggerHandler
 * @details Delegates to TriggerHandler for actual registration and validation
 */
bool InterruptManager::RegisterTrigger(const Trigger& trigger)
{
    if (!triggerHandler_) {
        log_e("Cannot register trigger - TriggerHandler not initialized");
        ErrorManager::Instance().ReportError(ErrorLevel::ERROR, "InterruptManager",
                                            "Cannot register trigger - TriggerHandler not initialized");
        return false;
    }

    return triggerHandler_->RegisterTrigger(trigger);
}

/**
 * @brief Register action with ActionHandler
 * @details Delegates to ActionHandler for actual registration and validation
 */
bool InterruptManager::RegisterAction(const Action& action)
{
    if (!actionHandler_) {
        log_e("Cannot register action - ActionHandler not initialized");
        ErrorManager::Instance().ReportError(ErrorLevel::ERROR, "InterruptManager",
                                            "Cannot register action - ActionHandler not initialized");
        return false;
    }

    // Action registration no longer needed - panels handle actions directly
    log_w("RegisterAction called but no longer supported - use panel methods directly");
    return true;
}

/**
 * @brief Update panel button handler functions for current panel
 * @details Called when switching panels to update button behavior
 */
void InterruptManager::SetCurrentPanel(IActionHandler* panel)
{
    log_v("SetCurrentPanel() called");

    if (!actionHandler_) {
        log_e("Cannot set current panel - ActionHandler not initialized. Button input will not work!");
        ErrorManager::Instance().ReportCriticalError("InterruptManager",
                                                     "Cannot set current panel - button input will not work");
        return;
    }

    // Delegate to ActionHandler to set current panel
    actionHandler_->SetCurrentPanel(panel);
    log_i("Set current panel in ActionHandler");
}

/**
 * @brief Sets preference service for configuration access
 * @param preferenceService The preference service to use for configuration
 */
void InterruptManager::SetConfigurationManager(IConfigurationManager* configurationManager)
{
    log_v("SetConfigurationManager() called");

    if (!actionHandler_) {
        log_e("Cannot set preference service - ActionHandler not initialized");
        return;
    }

    // Delegate to ActionHandler to set preference service
    actionHandler_->SetConfigurationManager(configurationManager);
    log_i("Set preference service in ActionHandler");
}

/**
 * @brief Sets panel manager for UI state access
 * @param panelManager The panel manager to use for UI state checks
 */
void InterruptManager::SetPanelManager(IPanelManager* panelManager)
{
    log_v("SetPanelManager() called");
    panelManager_ = panelManager;
    log_i("Set panel manager for UI state access");
}


/**
 * @brief Get total count of registered interrupts (triggers + actions)
 */
size_t InterruptManager::GetRegisteredInterruptCount() const
{
    size_t count = 0;
    if (triggerHandler_) {
        count += triggerHandler_->GetTriggerCount();
    }
    if (actionHandler_) {
        // GetActionCount no longer available - using single pending action
        count += actionHandler_->HasPendingAction() ? 1 : 0;
    }
    return count;
}

/**
 * @brief Check if any triggers or actions are currently active or pending
 */
bool InterruptManager::HasActiveInterrupts() const
{
    if (triggerHandler_ && triggerHandler_->HasActiveTriggers()) {
        return true;
    }
    if (actionHandler_ && actionHandler_->HasPendingAction()) {
        return true;
    }
    return false;
}

/**
 * @brief Check if panel restoration is needed after interrupt deactivation
 * @details Coordinates with PanelManager for seamless panel restoration
 */
void InterruptManager::CheckRestoration()
{
    if (!triggerHandler_) {
        return;
    }

    // Check if any non-overridable triggers are still active
    bool hasActiveTrigger = triggerHandler_->HasActiveTriggers();
    if (!hasActiveTrigger) {
        // Let the PanelManager handle restoration logic
        // This method provides coordination point for restoration
    }
}

/**
 * @brief Find and execute the highest priority PANEL trigger
 * @details Scans for active panel triggers and executes the highest priority one
 */
bool InterruptManager::CheckAndExecuteHighestPriorityTrigger()
{
    if (!triggerHandler_) {
        return false;
    }

    // Find the highest priority active PANEL type trigger
    Trigger* highestTrigger = triggerHandler_->FindHighestPrioritySameType(TriggerType::PANEL);

    if (highestTrigger && highestTrigger->isActive && highestTrigger->activateFunc) {
        log_i("Found active trigger '%s' (priority %d) - executing",
              highestTrigger->id, static_cast<int>(highestTrigger->priority));
        highestTrigger->activateFunc();
        return true;
    }

    return false;
}

/**
 * @brief Find and execute the highest priority STYLE trigger
 * @details Handles theme and styling triggers separately from panel triggers
 */
void InterruptManager::CheckAndExecuteActiveStyleTriggers()
{
    if (!triggerHandler_) {
        return;
    }

    // Find the highest priority active STYLE type trigger
    Trigger* styleTrigger = triggerHandler_->FindHighestPrioritySameType(TriggerType::STYLE);

    if (styleTrigger && styleTrigger->isActive && styleTrigger->activateFunc) {
        log_i("Found active STYLE trigger '%s' (priority %d) - executing",
              styleTrigger->id, static_cast<int>(styleTrigger->priority));
        styleTrigger->activateFunc();
    }
}

// ========== Private Methods ==========

/**
 * @brief Register all system-level triggers and actions
 * @details Creates sensors through TriggerHandler and registers standard system interrupts
 */
void InterruptManager::RegisterSystemInterrupts()
{
    // SAFE CASTING: All GPIO sensors inherit from BaseSensor through multiple inheritance
    // TriggerHandler owns these sensors exclusively - no shared ownership
    // static_cast is safe here as type hierarchy guarantees BaseSensor interface
    // This pattern allows system triggers to access sensors through base interface

    // Get system triggers with handler-owned sensors
    // Note: static_cast is safe here as all sensors inherit from BaseSensor
    auto systemTriggers = Interrupts::GetSystemTriggers(
        static_cast<BaseSensor*>(triggerHandler_->GetKeyPresentSensor()),
        static_cast<BaseSensor*>(triggerHandler_->GetKeyNotPresentSensor()),
        static_cast<BaseSensor*>(triggerHandler_->GetLockSensor()),
        static_cast<BaseSensor*>(triggerHandler_->GetLightsSensor()),
#ifdef CLARITY_DEBUG
        static_cast<BaseSensor*>(triggerHandler_->GetDebugErrorSensor())
#else
        nullptr  // No debug sensor in release builds
#endif
    );

    // Register all system triggers with error handling
    for (const auto& trigger : systemTriggers) {
        if (!RegisterTrigger(trigger)) {
            log_e("Failed to register system trigger: %s", trigger.id);
            ErrorManager::Instance().ReportError(ErrorLevel::ERROR, "InterruptManager",
                                                "Failed to register system trigger");
        }
    }

    // Get and register system actions
    auto systemActions = Interrupts::GetSystemActions();
    for (const auto& action : systemActions) {
        if (!RegisterAction(action)) {
            log_e("Failed to register system action: %s", action.id);
            ErrorManager::Instance().ReportError(ErrorLevel::ERROR, "InterruptManager",
                                                "Failed to register system action");
        }
    }

    log_i("Registered %zu system triggers and %zu system actions",
          systemTriggers.size(), systemActions.size());
}

