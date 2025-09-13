#include "managers/interrupt_manager.h"
#include "managers/error_manager.h"
#include "handlers/trigger_handler.h"
#include "handlers/action_handler.h"
#include "utilities/system_definitions.h"
#include "utilities/constants.h"
#include "sensors/key_present_sensor.h"
#include "sensors/key_not_present_sensor.h"
#include "sensors/lock_sensor.h"
#include "sensors/lights_sensor.h"
#ifdef CLARITY_DEBUG
#include "sensors/debug_error_sensor.h"
#endif
#include <Arduino.h>
#include <cstring>
#include "esp32-hal-log.h"

// Singleton implementation
InterruptManager& InterruptManager::Instance()
{
    static InterruptManager instance;
    return instance;
}

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
        return;
    }

    // Store GPIO provider
    gpioProvider_ = gpioProvider;
    
    // Clear handler storage
    handlers_.clear();
    
    // Create new Trigger/Action architecture handlers only
    triggerHandler_ = std::make_shared<TriggerHandler>(gpioProvider);
    actionHandler_ = std::make_shared<ActionHandler>(gpioProvider);
    
    if (!triggerHandler_ || !actionHandler_)
    {
        log_e("Failed to create Trigger/Action handlers");
        ErrorManager::Instance().ReportError(ErrorLevel::ERROR, "InterruptManager", 
                                           "Failed to create Trigger/Action handlers");
        return;
    }
    
    // Register handlers
    RegisterHandler(triggerHandler_);
    RegisterHandler(actionHandler_);
    
    // Register system triggers and actions
    RegisterSystemInterrupts();
    
    initialized_ = true;
    lastEvaluationTime_ = millis();
    
    log_i("InterruptManager initialized with new Trigger/Action architecture");
}

void InterruptManager::RegisterSystemInterrupts()
{
    
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
    
    // Register all system triggers
    for (const auto& trigger : systemTriggers) {
        if (!RegisterTrigger(trigger)) {
            log_e("Failed to register system trigger: %s", trigger.id);
        }
    }
    
    // Get and register system actions
    auto systemActions = SystemDefinitions::GetSystemActions();
    for (const auto& action : systemActions) {
        if (!RegisterAction(action)) {
            log_e("Failed to register system action: %s", action.id);
        }
    }
    
    log_i("Registered %zu system triggers and %zu system actions", 
          systemTriggers.size(), systemActions.size());
}

void InterruptManager::Process()
{
    if (!initialized_) {
        return;
    }
    
    // Process Actions (continuous evaluation for responsiveness)
    if (actionHandler_) {
        // Removed verbose logging from hot path - ActionHandler called every main loop cycle";
        actionHandler_->Process();
    }
    
    // Process Triggers (only during UI idle)
    if (IsUIIdle() && triggerHandler_) {
        log_d("Processing TriggerHandler - UI is idle, polling GPIO sensors");
        triggerHandler_->Process();
        log_d("TriggerHandler processing complete");
    }
    else {
        // Removed verbose logging from hot path - UI busy check happens frequently
    }
    
    // Update performance counters
    totalEvaluations_++;
}

bool InterruptManager::RegisterTrigger(const Trigger& trigger)
{
    if (!triggerHandler_) {
        log_e("Cannot register trigger - TriggerHandler not initialized");
        return false;
    }
    
    return triggerHandler_->RegisterTrigger(trigger);
}

void InterruptManager::UnregisterTrigger(const char* id)
{
    if (triggerHandler_) {
        triggerHandler_->UnregisterTrigger(id);
    }
}

bool InterruptManager::RegisterAction(const Action& action)
{
    if (!actionHandler_) {
        log_e("Cannot register action - ActionHandler not initialized");
        return false;
    }
    
    return actionHandler_->RegisterAction(action);
}

void InterruptManager::UnregisterAction(const char* id)
{
    if (actionHandler_) {
        actionHandler_->UnregisterAction(id);
    }
}

void InterruptManager::UpdatePanelFunctions(void (*shortPressFunc)(void*), void (*longPressFunc)(void*), void* context)
{
    log_v("UpdatePanelFunctions() called");
    
    if (!actionHandler_) {
        log_w("Cannot update panel functions - ActionHandler not initialized");
        return;
    }
    
    // Update action handler with new panel functions
    actionHandler_->UpdatePanelFunctions(shortPressFunc, longPressFunc, context);
    log_i("Updated panel functions in ActionHandler");
}

void InterruptManager::RegisterHandler(std::shared_ptr<IHandler> handler)
{
    if (!handler) {
        log_e("Cannot register null handler");
        return;
    }
    
    handlers_.push_back(handler);
}

void InterruptManager::UnregisterHandler(std::shared_ptr<IHandler> handler)
{
    auto it = std::find(handlers_.begin(), handlers_.end(), handler);
    if (it != handlers_.end()) {
        handlers_.erase(it);
    }
}

bool InterruptManager::IsUIIdle() const
{
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

void InterruptManager::CheckRestoration()
{
    log_v("CheckRestoration() called");
    
    if (!triggerHandler_) {
        log_w("Cannot check restoration - TriggerHandler not initialized");
        return;
    }
    
    // Check if any non-overridable triggers are still active
    bool hasActiveTrigger = triggerHandler_->HasActiveTriggers();
    
    if (!hasActiveTrigger) {
        // Let the PanelManager handle restoration logic
        // This method is mainly for coordination
    }
}

bool InterruptManager::CheckAndExecuteHighestPriorityTrigger()
{
    log_v("CheckAndExecuteHighestPriorityTrigger() called");
    
    if (!triggerHandler_) {
        log_w("Cannot check triggers - TriggerHandler not initialized");
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

void InterruptManager::CheckAndExecuteActiveStyleTriggers()
{
    log_v("CheckAndExecuteActiveStyleTriggers() called");
    
    if (!triggerHandler_) {
        log_w("Cannot check style triggers - TriggerHandler not initialized");
        return;
    }
    
    // Find the highest priority active STYLE type trigger
    Trigger* styleTrigger = triggerHandler_->FindHighestPrioritySameType(TriggerType::STYLE);
    
    if (styleTrigger && styleTrigger->isActive && styleTrigger->activateFunc) {
        log_i("Found active STYLE trigger '%s' (priority %d) - executing", 
              styleTrigger->id, static_cast<int>(styleTrigger->priority));
        styleTrigger->activateFunc();
    } else {
    }
}

void InterruptManager::PrintSystemStatus() const
{
    log_i("=== InterruptManager Status ===");
    log_i("Initialized: %s", initialized_ ? "true" : "false");
    log_i("Total Evaluations: %zu", totalEvaluations_);
    log_i("Total Executions: %zu", totalExecutions_);
    log_i("Registered Handlers: %zu", handlers_.size());
    
    if (triggerHandler_) {
        log_i("TriggerHandler registered triggers: %zu", triggerHandler_->GetTriggerCount());
    }
    
    if (actionHandler_) {
        log_i("ActionHandler registered actions: %zu", actionHandler_->GetActionCount());
    }
}

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

void InterruptManager::GetInterruptStatistics(size_t& totalEvaluations, size_t& totalExecutions) const
{
    totalEvaluations = totalEvaluations_;
    totalExecutions = totalExecutions_;
}

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

size_t InterruptManager::GetInterruptCount() const
{
    return GetRegisteredInterruptCount();
}

void InterruptManager::OptimizeMemoryUsage()
{
    log_v("OptimizeMemoryUsage() called");
    
    // Memory is already optimized with static arrays
    // This method exists for API compatibility
    
    if (triggerHandler_) {
        // Static arrays, no optimization needed
    }
    
    if (actionHandler_) {
        // Static arrays, no optimization needed
    }
    
}

void InterruptManager::CompactInterruptArray()
{
    log_v("CompactInterruptArray() called");
    
    // Static arrays don't need compaction
}