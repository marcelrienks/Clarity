#include "managers/interrupt_manager.h"
#include <esp32-hal-log.h>

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

    _global_triggers.clear();
    _panel_triggers.clear();
    _previous_panel = "";
    _current_panel = "";
    _active_trigger = nullptr;
    _panel_switch_callback = panel_switch_callback;

    log_d("InterruptManager initialized");
}

/// @brief Check all registered triggers and handle any activations
/// @return true if a trigger was activated and panel switch occurred
bool InterruptManager::check_triggers()
{
    log_d("...");

    // First check if we need to restore from an active trigger
    if (_active_trigger != nullptr)
    {
        check_trigger_restoration();
    }

    // Check global triggers first (higher priority)
    for (auto &[trigger_id, trigger] : _global_triggers)
    {
        if (evaluate_trigger(trigger_id, trigger))
        {
            return true;
        }
    }

    // Then check panel-specific triggers
    for (auto &[trigger_id, trigger] : _panel_triggers)
    {
        if (evaluate_trigger(trigger_id, trigger))
        {
            return true;
        }
    }

    return false;
}

/// @brief Register a global trigger that persists throughout application
/// @param trigger_id Unique identifier for the trigger
/// @param trigger Shared pointer to the trigger instance
void InterruptManager::register_global_trigger(const std::string &trigger_id, std::shared_ptr<ITrigger> trigger)
{
    log_d("...");

    trigger->init();
    _global_triggers[trigger_id] = trigger;

    log_d("Global trigger registered: %s", trigger_id.c_str());
}

/// @brief Register a panel-specific trigger (removed when panel changes)
/// @param trigger_id Unique identifier for the trigger
/// @param trigger Shared pointer to the trigger instance
void InterruptManager::add_panel_trigger(const std::string &trigger_id, std::shared_ptr<ITrigger> trigger)
{
    log_d("...");

    trigger->init();
    _panel_triggers[trigger_id] = trigger;

    log_d("Panel trigger registered: %s", trigger_id.c_str());
}

/// @brief Remove a specific trigger by ID
/// @param trigger_id The trigger ID to remove
void InterruptManager::remove_trigger(const std::string &trigger_id)
{
    log_d("...");

    // Check global triggers first
    auto global_it = _global_triggers.find(trigger_id);
    if (global_it != _global_triggers.end())
    {
        // Clear active trigger if it's the one being removed
        if (_active_trigger == global_it->second)
        {
            _active_trigger = nullptr;
        }
        _global_triggers.erase(global_it);
        log_d("Removed global trigger: %s", trigger_id.c_str());
        return;
    }

    // Check panel triggers
    auto panel_it = _panel_triggers.find(trigger_id);
    if (panel_it != _panel_triggers.end())
    {
        // Clear active trigger if it's the one being removed
        if (_active_trigger == panel_it->second)
        {
            _active_trigger = nullptr;
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
        for (const auto &[trigger_id, trigger] : _panel_triggers)
        {
            if (_active_trigger == trigger)
            {
                _active_trigger = nullptr;
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
void InterruptManager::set_current_panel(const std::string &panel_name)
{
    log_d("...");

    // Only update previous panel if we're switching to a different panel
    if (_current_panel != panel_name && !_current_panel.empty())
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
        log_i("Trigger condition cleared, restoring previous panel: %s", _previous_panel.c_str());

        _panel_switch_callback(_previous_panel.c_str());
        _active_trigger = nullptr;
    }
}

/// @brief Evaluate a single trigger and handle activation
/// @param trigger_id The trigger identifier
/// @param trigger The trigger to evaluate
/// @return true if trigger was activated
bool InterruptManager::evaluate_trigger(const std::string &trigger_id, std::shared_ptr<ITrigger> trigger)
{
    log_d("...");

    if (trigger->evaluate())
    {
        // Store the active trigger for restoration tracking
        _active_trigger = trigger;

        _panel_switch_callback(trigger->get_target_panel());
        return true;
    }

    return false;
}