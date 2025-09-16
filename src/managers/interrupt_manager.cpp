#include "managers/interrupt_manager.h"
#include "managers/error_manager.h"
#include "handlers/trigger_handler.h"
#include "handlers/action_handler.h"
#include "utilities/system_definitions.h"
#include "utilities/constants.h"
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

    // Clear any existing handler storage
    handlers_.clear();

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

    // Register handlers in legacy system
    RegisterHandler(triggerHandler_);
    RegisterHandler(actionHandler_);

    // Register all system-level triggers and actions
    RegisterSystemInterrupts();

    initialized_ = true;
    lastEvaluationTime_ = millis();

    log_i("InterruptManager initialized with new Trigger/Action architecture");
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

    // Process Actions continuously for immediate responsiveness (button presses)
    if (actionHandler_) {
        actionHandler_->Process();
    }

    // Process Triggers only during UI idle to avoid interfering with animations
    if (IsUIIdle() && triggerHandler_) {
        triggerHandler_->Process();
    }

    // Update performance monitoring counters
    totalEvaluations_++;
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

    return actionHandler_->RegisterAction(action);
}

/**
 * @brief Update panel button handler functions for current panel
 * @details Called when switching panels to update button behavior
 */
void InterruptManager::UpdatePanelFunctions(void (*shortPressFunc)(void*), void (*longPressFunc)(void*), void* context)
{
    log_v("UpdatePanelFunctions() called");

    if (!actionHandler_) {
        log_e("Cannot update panel functions - ActionHandler not initialized. Button input will not work!");
        ErrorManager::Instance().ReportCriticalError("InterruptManager",
                                                     "Cannot update panel functions - button input will not work");
        return;
    }

    // Delegate to ActionHandler to update button behavior
    actionHandler_->UpdatePanelFunctions(shortPressFunc, longPressFunc, context);
    log_i("Updated panel functions in ActionHandler");
}

/**
 * @brief Register legacy interrupt handler (for backward compatibility)
 */
void InterruptManager::RegisterHandler(std::shared_ptr<IHandler> handler)
{
    if (!handler) {
        log_e("Cannot register null handler");
        ErrorManager::Instance().ReportError(ErrorLevel::ERROR, "InterruptManager",
                                            "Cannot register null handler");
        return;
    }

    handlers_.push_back(handler);
}

/**
 * @brief Unregister legacy interrupt handler
 */
void InterruptManager::UnregisterHandler(std::shared_ptr<IHandler> handler)
{
    auto it = std::find(handlers_.begin(), handlers_.end(), handler);
    if (it != handlers_.end()) {
        handlers_.erase(it);
    }
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
        count += actionHandler_->GetActionCount();
    }
    return count;
}

/**
 * @brief Retrieve performance statistics for interrupt processing
 */
void InterruptManager::GetInterruptStatistics(size_t& totalEvaluations, size_t& totalExecutions) const
{
    totalEvaluations = totalEvaluations_;
    totalExecutions = totalExecutions_;
}

/**
 * @brief Optimize memory usage - no-op with static array implementation
 * @details Static arrays are already memory-optimal, method exists for API compatibility
 */
void InterruptManager::OptimizeMemoryUsage()
{
    log_v("OptimizeMemoryUsage() called");

    // Memory is already optimized with static arrays
    // This method exists for API compatibility with dynamic implementations

    if (triggerHandler_) {
        // Static arrays are already optimized - no action needed
    }

    if (actionHandler_) {
        // Static arrays are already optimized - no action needed
    }
}

/**
 * @brief Compact interrupt array - no-op with static array implementation
 * @details Static arrays don't fragment and don't need compaction
 */
void InterruptManager::CompactInterruptArray()
{
    log_v("CompactInterruptArray() called");

    // Static arrays don't need compaction - no fragmentation occurs
}

/**
 * @brief Check if any triggers or actions are currently active or pending
 */
bool InterruptManager::HasActiveInterrupts() const
{
    if (triggerHandler_ && triggerHandler_->HasActiveTriggers()) {
        return true;
    }
    if (actionHandler_ && actionHandler_->HasPendingActions()) {
        return true;
    }
    return false;
}

/**
 * @brief Get interrupt count (alias for GetRegisteredInterruptCount)
 */
size_t InterruptManager::GetInterruptCount() const
{
    return GetRegisteredInterruptCount();
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
    auto systemTriggers = SystemDefinitions::GetSystemTriggers(
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
    auto systemActions = SystemDefinitions::GetSystemActions();
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

bool InterruptManager::IsUIIdle() const
{
    // PERFORMANCE OPTIMIZATION: Cache UI idle state to reduce LVGL queries
    // LVGL query is expensive (requires display lock + calculation)
    // 5ms cache timeout balances responsiveness vs performance
    // 10ms idle threshold prevents interrupting smooth animations

    // Cache UI idle state with timeout to reduce LVGL query frequency
    static bool cached_idle = false;
    static unsigned long last_check = 0;

    unsigned long current_time = millis();
    if (current_time - last_check >= 5) { // Check every 5ms max (was checking every main loop cycle)
        cached_idle = lv_disp_get_inactive_time(nullptr) > 10; // 10ms idle threshold
        last_check = current_time;
    }

    return cached_idle;
}