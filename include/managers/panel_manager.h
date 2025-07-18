#pragma once // preventing duplicate definitions, alternative to the traditional include guards

#include "interfaces/i_panel.h"
#include "interfaces/i_device.h"
#include "panels/splash_panel.h"
#include "panels/oem_oil_panel.h"
#include "panels/key_panel.h"
#include "panels/lock_panel.h"
#include "utilities/ticker.h"
#include "managers/interrupt_manager.h"

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
 * - create_and_load_panel_with_splash(): Smooth transitions with splash screen
 * - refresh_panel(): Periodic updates without full reload
 * - Thread-safe loading state management
 * 
 * @context This is the main coordinator for all panel operations.
 * It manages the current panel (likely OemOilPanel) and handles transitions.
 * The factory pattern allows easy addition of new panel types.
 */

class PanelManager
{
public:
    // Constructors and Destructors
    PanelManager(const PanelManager &) = delete;
    PanelManager &operator=(const PanelManager &) = delete;

    // Static Methods
    static PanelManager &get_instance();

    // Core Functionality Methods
    /// @brief Initialize the panel manager, register panels/triggers, and setup interrupt handling
    void init();
    
    /// @brief Create and load a panel by name with optional completion callback
    /// @param panel_name Name of the panel to create and load
    /// @param completion_callback Optional callback function to execute when loading is complete
    void create_and_load_panel(const char *panel_name, std::function<void()> completion_callback = nullptr, bool is_trigger_driven = false);
    
    /// @brief Load a panel after first showing a splash screen transition
    /// @param panel_name Name of the target panel to load after splash
    void create_and_load_panel_with_splash(const char *panel_name);
    
    /// @brief Update the currently active panel (called from main loop)
    void update_panel();
    
    /// @brief Get the panel name to restore when all triggers are inactive
    /// @return Panel name for restoration, or nullptr if none set
    const char* get_restoration_panel() const;

    // Template Methods
    /// @brief Register a panel type with the factory for dynamic creation
    /// @tparam T Panel type that implements IPanel interface
    /// @param panel_name String identifier for the panel type
    template<typename T> // Note the implementation of a template type must exist in header
    void register_panel(const char *panel_name) {
        _registered_panels[panel_name] = []() -> std::shared_ptr<IPanel> { 
            return std::make_shared<T>(); 
        };
    }

    /// @brief Register a global trigger type with the interrupt manager
    /// @tparam T Trigger type that implements ITrigger interface
    /// @param trigger_id String identifier for the trigger
    /// @param auto_restore Whether the trigger should auto-restore previous panel when cleared
    template<typename T>
    void register_global_trigger(const char *trigger_id, bool auto_restore = false) {
        InterruptManager::get_instance().register_global_trigger(trigger_id, std::make_shared<T>(auto_restore));
    }

private:
    // Constructors and Destructors
    PanelManager() = default;
    ~PanelManager();

    // Core Functionality Methods
    /// @brief Create a panel instance by name using the registered factory
    /// @param panel_name Name of the panel type to create
    /// @return Shared pointer to the created panel instance
    std::shared_ptr<IPanel> create_panel(const char *panel_name);
    
    /// @brief Register all available panel types with the factory
    void register_panels();
    
    /// @brief Register all global triggers with the interrupt manager
    void register_triggers();

    // Callback Methods
    /// @brief Callback executed when splash screen loading is complete
    /// @param panel_name Name of the target panel to load after splash
    void splash_completion_callback(const char *panel_name);
    
    /// @brief Callback executed when normal panel loading is complete
    void panel_completion_callback();
    
    /// @brief Callback executed when interrupt-triggered panel loading is complete
    void interrupt_panel_switch_callback();

    // Instance Data Members
    std::shared_ptr<IPanel> _panel = nullptr;
    std::map<std::string, std::function<std::shared_ptr<IPanel>()>> _registered_panels; // Map of panel type names to creator functions for each of those names
    bool _is_loading = false; // this allows the panel to be locked during loading from updates
    std::string _last_non_trigger_panel; // Track last panel loaded by user/config (not by trigger) for restoration
};