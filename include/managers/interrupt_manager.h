#pragma once

#include "interfaces/i_trigger.h"
#include <string>
#include <memory>
#include <map>
#include <vector>
#include <functional>

/**
 * @class InterruptManager
 * @brief Singleton manager for interrupt triggers and panel switching conditions
 *
 * @details This manager handles the registration, evaluation, and lifecycle of
 * interrupt triggers that can cause immediate panel switches. It follows the
 * same singleton and factory patterns as PanelManager to provide centralized
 * trigger management with dynamic registration.
 *
 * @design_patterns:
 * - Singleton: Single instance manages all triggers
 * - Factory: Dynamic trigger creation via registration
 * - Template: Type-safe trigger registration
 *
 * @trigger_lifecycle:
 * 1. Register trigger types with register_trigger<T>()
 * 2. Create and add triggers via add_trigger()
 * 3. Evaluate all triggers with check_triggers()
 * 4. Handle trigger activation with callbacks
 * 5. Clean up panel-specific triggers on panel changes
 *
 * @trigger_types:
 * - Global triggers: Registered in main setup(), persist throughout application
 * - Panel triggers: Registered when panels load, removed when panels unload
 *
 * @state_management:
 * - _global_triggers: Triggers that persist throughout application lifetime
 * - _panel_triggers: Triggers specific to current panel, cleaned up on panel change
 * - _previous_panel: Track previous panel for restoration after trigger clears
 * - _active_trigger: Currently active trigger (if any)
 *
 * @integration: Works with PanelManager to force immediate panel switches
 * when trigger conditions are met, regardless of current panel state.
 */
class InterruptManager
{
public:
    // Constructors and Destructors
    InterruptManager(const InterruptManager &) = delete;
    InterruptManager &operator=(const InterruptManager &) = delete;

    // Static Methods
    static InterruptManager &get_instance();

    // Core Functionality Methods
    /// @brief Initialize the interrupt manager
    /// @param panel_switch_callback Function to call when panel should be switched
    void init(std::function<void(const char *)> panel_switch_callback = nullptr);

    /// @brief Check all registered triggers and handle any activations
    /// @return true if a trigger was activated and panel switch occurred
    bool check_triggers();

    /// @brief Register a global trigger that persists throughout application
    /// @param trigger_id Unique identifier for the trigger
    /// @param trigger Shared pointer to the trigger instance
    void register_global_trigger(const std::string &trigger_id, std::shared_ptr<ITrigger> trigger);

    /// @brief Register a panel-specific trigger (removed when panel changes)
    /// @param trigger_id Unique identifier for the trigger
    /// @param trigger Shared pointer to the trigger instance
    void add_panel_trigger(const std::string &trigger_id, std::shared_ptr<ITrigger> trigger);

    /// @brief Remove a specific trigger by ID
    /// @param trigger_id The trigger ID to remove
    void remove_trigger(const std::string &trigger_id);

    /// @brief Remove all panel-specific triggers (called when panel changes)
    void clear_panel_triggers();

    /// @brief Set the current panel name (for tracking)
    /// @param panel_name Current panel name
    void set_current_panel(const std::string &panel_name);

    // Accessor Methods
    /// @brief Get the previous panel name (for restoration)
    /// @return Previous panel name or empty string if none
    const std::string &get_previous_panel() const { return _previous_panel; }

    // Template Methods
    /// @brief Factory method for trigger registration
    /// @tparam T Trigger type that implements ITrigger interface
    /// @return Shared pointer to the created trigger instance
    template <typename T>
    std::shared_ptr<ITrigger> create_trigger()
    {
        return std::make_shared<T>();
    }

private:
    // Constructors and Destructors
    InterruptManager() = default;
    ~InterruptManager() = default;

    // Core Functionality Methods
    /// @brief Evaluate a single trigger and handle activation
    /// @param trigger_id The trigger identifier
    /// @param trigger The trigger to evaluate
    /// @param priority The trigger's priority level
    /// @return true if trigger was activated
    bool evaluate_trigger(const std::string &trigger_id, std::shared_ptr<ITrigger> trigger, int priority);

    /// @brief Handle trigger activation
    /// @param trigger_id The activated trigger ID
    /// @param trigger The activated trigger
    void handle_trigger_activation(const std::string &trigger_id, std::shared_ptr<ITrigger> trigger);

    /// @brief Check if trigger condition has cleared and handle restoration
    void check_trigger_restoration();

    // Instance Data Members
    /// @brief Trigger entry with ID and instance, ordered by registration priority
    struct TriggerEntry {
        std::string id;
        std::shared_ptr<ITrigger> trigger;
        int priority; ///< Lower number = higher priority (0 = highest)
    };
    
    std::vector<TriggerEntry> _global_triggers;                         ///< Global triggers ordered by priority
    std::vector<TriggerEntry> _panel_triggers;                          ///< Panel-specific triggers ordered by priority
    std::string _previous_panel = "";                                   ///< Previous panel for restoration
    std::string _current_panel = "";                                    ///< Current active panel
    std::shared_ptr<ITrigger> _active_trigger = nullptr;                ///< Currently active trigger (if any)
    int _active_trigger_priority = -1;                                  ///< Priority of currently active trigger (-1 = none)
    bool _trigger_has_fired = false;                                    ///< Flag indicating if any trigger has ever fired (enables restoration)
    std::function<void(const char *)> _panel_switch_callback = nullptr; ///< Callback for panel switching
};