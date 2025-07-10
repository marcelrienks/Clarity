#pragma once // preventing duplicate definitions, alternative to the traditional include guards

#include "interfaces/i_panel.h"
#include "interfaces/i_device.h"
#include "interfaces/i_sensor.h"
#include "panels/splash_panel.h"
#include "panels/oem_oil_panel.h"
#include "panels/key_panel.h"
#include "utilities/ticker.h"

#include <string>
#include <functional>
#include <memory>
#include <list>
#include <vector>
#include <map>

/**
 * @class PanelManager
 * @brief Singleton factory for panel lifecycle management and transitions
 * 
 * @details This manager handles the complete lifecycle of panels including
 * creation, loading, updating, and transitions. It implements both Factory
 * and Singleton patterns to provide centralized panel management with
 * dynamic panel registration and creation.
 * 
 * @design_patterns:
 * - Singleton: Single instance manages all panels
 * - Factory: Dynamic panel creation via registration
 * - Template: Type-safe panel registration
 * 
 * @panel_lifecycle:
 * 1. Register panel types with register_panel<T>()
 * 2. Create panels dynamically via create_panel()
 * 3. Load panels with init() → load() → show callbacks
 * 4. Update panels with periodic refresh_panel()
 * 5. Transition between panels with load_panel()
 * 
 * @registered_panels:
 * - SplashPanel: Startup/branding screen
 * - OemOilPanel: Main oil monitoring dashboard
 * - Future panels: Easy extensibility via registration
 * 
 * @state_management:
 * - _is_loading: Prevents concurrent panel operations
 * - _panel: Current active panel instance
 * - Callback-based completion handling
 * 
 * @special_features:
 * - load_panel_with_Splash(): Smooth transitions with splash screen
 * - refresh_panel(): Periodic updates without full reload
 * - Thread-safe loading state management
 * 
 * @context This is the main coordinator for all panel operations.
 * It manages the current panel (likely OemOilPanel) and handles transitions.
 * The factory pattern allows easy addition of new panel types.
 * 
 * @interrupt_system:
 * - register_interrupt(): Register sensor/condition/panel combinations
 * - check_interrupts(): Check for interrupt conditions before normal updates
 * - Priority panel switching overrides normal loading states
 * - Optional restoration to previous panel when condition clears
 */
/// @brief Interrupt trigger configuration
struct InterruptTrigger
{
    std::shared_ptr<ISensor> sensor;           // Sensor to monitor
    std::function<bool(const Reading&)> condition; // Condition function to evaluate reading
    std::string target_panel;                  // Panel to switch to when triggered
    bool restore_previous = false;             // Whether to restore previous panel when condition clears
    bool active = true;                        // Whether this interrupt is currently active
};

class PanelManager
{
public:
    static PanelManager &get_instance();

    void init();
    void load_panel(const char *panel_name);
    void load_panel_with_Splash(const char *panel_name);
    void update_panel();
    
    // Interrupt system methods
    void register_interrupt(const std::string& id, 
                           std::shared_ptr<ISensor> sensor,
                           std::function<bool(const Reading&)> condition,
                           const std::string& target_panel,
                           bool restore_previous = false);
    void enable_interrupt(const std::string& id);
    void disable_interrupt(const std::string& id);
    void check_interrupts();

    // Register a panel type with the factory
    template<typename T> // Note the implementation of a template type must exist in header
    void register_panel(const char *panel_name) {
        _registered_panels[panel_name] = []() -> std::shared_ptr<IPanel> { 
            return std::make_shared<T>(); 
        };
    }

private:
    std::shared_ptr<IPanel> _panel = nullptr;
    std::map<std::string, std::function<std::shared_ptr<IPanel>()>> _registered_panels; // Map of panel type names to creator functions for each of those names
    bool _is_loading = false; // this allows the panel to be locked during loading from show_panel() or change from update_current_panel()
    
    // Interrupt system state
    std::map<std::string, InterruptTrigger> _interrupt_triggers; // Registered interrupt conditions
    std::string _previous_panel = "";                           // Panel to restore to after interrupt clears
    std::string _current_interrupt_panel = "";                  // Currently active interrupt panel

    ~PanelManager();

    std::shared_ptr<IPanel> create_panel(const char *panel_name);
    void load_panel(std::shared_ptr<IPanel> panel, std::function<void()> completion_callback = nullptr);
    void splash_completion_callback(const char *panel_name);
    void panel_completion_callback();
};