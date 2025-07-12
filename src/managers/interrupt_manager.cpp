#include "managers/interrupt_manager.h"
#include "managers/panel_manager.h"
#include <esp32-hal-log.h>
#include <algorithm>

// Static Methods

/// @brief Get the singleton instance of InterruptManager
/// @return Reference to the InterruptManager instance
InterruptManager &InterruptManager::get_instance()
{
    static InterruptManager instance;
    return instance;
}

// Core Functionality Methods

/// @brief Initialize the interrupt manager
/// @param panel_switch_callback Function to call when panel should be switched
void InterruptManager::init(std::function<void(const char *)> panel_switch_callback)
{
    log_d("...");

    // Don't clear global triggers - they're registered before init is called
    // _global_triggers.clear();
    _panel_triggers.clear();
    _previous_panel = "";
    _current_panel = "";
    _active_trigger = nullptr;
    _active_trigger_priority = -1;
    _panel_switch_callback = panel_switch_callback;

    log_d("InterruptManager initialized with %d global triggers", _global_triggers.size());
}

/// @brief Check all registered triggers and handle any activations
/// @return true if a trigger was activated and panel switch occurred
bool InterruptManager::check_triggers()
{
    log_d("...");

    // First check if we need to restore from an active trigger
    if (_active_trigger != nullptr)
    {
        log_d("Active trigger detected, checking restoration...");
        check_trigger_restoration();
        // Don't return early - continue to check for higher priority triggers
    }

    log_d("Checking %d global triggers and %d panel triggers (active trigger priority: %d)", 
          _global_triggers.size(), _panel_triggers.size(), 
          _active_trigger != nullptr ? _active_trigger_priority : -1);

    // Check global triggers first (higher priority)
    for (const auto &entry : _global_triggers)
    {
        log_d("Evaluating trigger: %s (priority: %d)", entry.id.c_str(), entry.priority);
        
        // Only evaluate triggers with higher priority than current active trigger
        if (_active_trigger != nullptr && entry.priority >= _active_trigger_priority)
        {
            log_d("Skipping lower/equal priority trigger: %s (priority %d >= active %d)", 
                  entry.id.c_str(), entry.priority, _active_trigger_priority);
            continue;
        }
        
        if (evaluate_trigger(entry.id, entry.trigger, entry.priority))
        {
            return true;
        }
    }

    // Then check panel-specific triggers
    for (const auto &entry : _panel_triggers)
    {
        log_d("Evaluating panel trigger: %s (priority: %d)", entry.id.c_str(), entry.priority);
        
        // Only evaluate triggers with higher priority than current active trigger
        if (_active_trigger != nullptr && entry.priority >= _active_trigger_priority)
        {
            log_d("Skipping lower/equal priority panel trigger: %s (priority %d >= active %d)", 
                  entry.id.c_str(), entry.priority, _active_trigger_priority);
            continue;
        }
        
        if (evaluate_trigger(entry.id, entry.trigger, entry.priority))
        {
            return true;
        }
    }

    // Restoration is now handled in check_trigger_restoration() when triggers clear
    // This prevents double restoration logic that causes infinite loops
    // if (_active_trigger == nullptr && _trigger_has_fired) {
    //     const char* restoration_panel = PanelManager::get_instance().get_restoration_panel();
    //     if (restoration_panel != nullptr && _current_panel != restoration_panel) {
    //         log_i("No triggers active, restoring panel: %s", restoration_panel);
    //         _panel_switch_callback(restoration_panel);
    //         return true;
    //     }
    // }

    return false;
}

/// @brief Register a global trigger that persists throughout application
/// @param trigger_id Unique identifier for the trigger
/// @param trigger Shared pointer to the trigger instance
void InterruptManager::register_global_trigger(const std::string &trigger_id, std::shared_ptr<ITrigger> trigger)
{
    log_d("...");

    trigger->init();
    
    // Priority is based on registration order (0 = highest priority)
    int priority = _global_triggers.size();
    _global_triggers.push_back({trigger_id, trigger, priority});

    log_d("Global trigger registered: %s (priority: %d, total: %d)", trigger_id.c_str(), priority, _global_triggers.size());
}

/// @brief Register a panel-specific trigger (removed when panel changes)
/// @param trigger_id Unique identifier for the trigger
/// @param trigger Shared pointer to the trigger instance
void InterruptManager::add_panel_trigger(const std::string &trigger_id, std::shared_ptr<ITrigger> trigger)
{
    log_d("...");

    trigger->init();
    
    // Panel trigger priority starts after global triggers
    int priority = _global_triggers.size() + _panel_triggers.size();
    _panel_triggers.push_back({trigger_id, trigger, priority});

    log_d("Panel trigger registered: %s (priority: %d)", trigger_id.c_str(), priority);
}

/// @brief Remove a specific trigger by ID
/// @param trigger_id The trigger ID to remove
void InterruptManager::remove_trigger(const std::string &trigger_id)
{
    log_d("...");

    // Check global triggers first
    auto global_it = std::find_if(_global_triggers.begin(), _global_triggers.end(),
        [&trigger_id](const TriggerEntry& entry) { return entry.id == trigger_id; });
    
    if (global_it != _global_triggers.end())
    {
        // Clear active trigger if it's the one being removed
        if (_active_trigger == global_it->trigger)
        {
            _active_trigger = nullptr;
            _active_trigger_priority = -1;
        }
        _global_triggers.erase(global_it);
        log_d("Removed global trigger: %s", trigger_id.c_str());
        return;
    }

    // Check panel triggers
    auto panel_it = std::find_if(_panel_triggers.begin(), _panel_triggers.end(),
        [&trigger_id](const TriggerEntry& entry) { return entry.id == trigger_id; });
    
    if (panel_it != _panel_triggers.end())
    {
        // Clear active trigger if it's the one being removed
        if (_active_trigger == panel_it->trigger)
        {
            _active_trigger = nullptr;
            _active_trigger_priority = -1;
        }
        _panel_triggers.erase(panel_it);
        log_d("Removed panel trigger: %s", trigger_id.c_str());
        return;
    }
}

/// @brief Remove all panel-specific triggers (called when panel changes)
void InterruptManager::clear_panel_triggers()
{
    log_d("...");

    // Check if active trigger is in panel triggers being cleared
    if (_active_trigger != nullptr)
    {
        for (const auto &entry : _panel_triggers)
        {
            if (_active_trigger == entry.trigger)
            {
                _active_trigger = nullptr;
                _active_trigger_priority = -1;
                break;
            }
        }
    }

    size_t count = _panel_triggers.size();
    _panel_triggers.clear();

    log_d("Cleared %zu panel triggers", count);
}

/// @brief Set the current panel name (for tracking)
/// @param panel_name Current panel name
/// @param is_trigger_driven Whether this change is caused by trigger activation
void InterruptManager::set_current_panel(const std::string &panel_name, bool is_trigger_driven)
{
    log_d("...");

    // Only update previous panel if we're switching to a different panel
    // AND this is not a trigger-driven change (preserve original panel for restoration)
    if (_current_panel != panel_name && !_current_panel.empty() && !is_trigger_driven)
    {
        _previous_panel = _current_panel;
        log_d("Previous panel set to: %s", _previous_panel.c_str());
    }

    _current_panel = panel_name;
}

// Private Methods

/// @brief Check if trigger condition has cleared and handle restoration
void InterruptManager::check_trigger_restoration()
{
    // Check if trigger condition has cleared
    if (!_active_trigger->evaluate() && _active_trigger->should_restore())
    {
        // Get restoration panel from PanelManager instead of using _previous_panel
        const char* restoration_panel = PanelManager::get_instance().get_restoration_panel();
        
        // Clear triggered state BEFORE calling panel switch to prevent infinite loop
        _active_trigger = nullptr;
        _active_trigger_priority = -1;
        
        // Only restore if we have a valid restoration panel
        if (restoration_panel != nullptr)
        {
            log_i("Trigger condition cleared, restoring to: %s", restoration_panel);
            _panel_switch_callback(restoration_panel);
        }
        else
        {
            log_i("Trigger condition cleared, but no restoration panel available");
        }
    }
}

/// @brief Evaluate a single trigger and handle activation
/// @param trigger_id The trigger identifier
/// @param trigger The trigger to evaluate
/// @return true if trigger was activated
bool InterruptManager::evaluate_trigger(const std::string &trigger_id, std::shared_ptr<ITrigger> trigger, int priority)
{
    log_d("...");

    if (trigger->evaluate())
    {
        // Store the active trigger for tracking
        _active_trigger = trigger;
        _active_trigger_priority = priority;
        _trigger_has_fired = true; // Mark that a trigger has fired (enables restoration)

        log_d("Trigger %s activated with priority %d", trigger_id.c_str(), priority);
        _panel_switch_callback(trigger->get_target_panel());
        return true;
    }
    else if (_active_trigger == trigger)
    {
        // This trigger was active but is no longer firing - clear it
        log_d("Trigger %s is no longer active, clearing", trigger_id.c_str());
        _active_trigger = nullptr;
        _active_trigger_priority = -1;
    }

    return false;
}