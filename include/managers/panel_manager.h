#pragma once // preventing duplicate definitions, alternative to the traditional include guards

#include "interfaces/i_panel.h"
#include "interfaces/i_device.h"
#include "interfaces/i_gpio_provider.h"
#include "interfaces/i_display_provider.h"
#include "panels/splash_panel.h"
#include "panels/oem_oil_panel.h"
#include "panels/key_panel.h"
#include "panels/lock_panel.h"
#include "utilities/ticker.h"
#include "managers/trigger_manager.h"
#include "managers/style_manager.h"

#include <string>
#include <functional>
#include <memory>
#include <list>
#include <vector>
#include <map>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <esp32-hal-log.h>

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
 * 2. Create panels dynamically via CreatePanel()
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
 * - _ui_state: Controls UI processing state and prevents concurrent operations
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
    PanelManager(IDisplayProvider* display, IGpioProvider* gpio);
    PanelManager(const PanelManager &) = delete;
    PanelManager &operator=(const PanelManager &) = delete;
    ~PanelManager();

    // Static Methods (kept for backward compatibility during transition)
    static PanelManager &GetInstance();

    // Core Functionality Methods
    /// @brief Initialize the panel manager (dependencies already injected via constructor)
    void init();
    
    
    
    /// @brief Set current UI state for Core 1 synchronization
    /// @param state Current UI processing state
    void SetUiState(UIState state);
    
    
    /// @brief Create and load a panel by name with optional completion callback
    /// @param panel_name Name of the panel to create and load
    /// @param completion_callback Optional callback function to execute when loading is complete
    void CreateAndLoadPanel(const char* panelName, std::function<void()> completionCallback = nullptr, bool isTriggerDriven = false);
    
    /// @brief Load a panel after first showing a splash screen transition
    /// @param panel_name Name of the target panel to load after splash
    void CreateAndLoadPanelWithSplash(const char* panelName);
    
    /// @brief Update the currently active panel (called from main loop)
    void UpdatePanel();
    
    

    // Template Methods
    /// @brief Register a panel type with the factory for dynamic creation
    /// @tparam T Panel type that implements IPanel interface
    /// @param panel_name String identifier for the panel type
    template<typename T> // Note the implementation of a template type must exist in header
    void register_panel(const char *panelName) {
        registeredPanels_[panelName] = []() -> std::shared_ptr<IPanel> { 
            return std::make_shared<T>(); 
        };
    }

    // Trigger Callbacks (public for trigger access)
    /// @brief Callback executed when trigger-driven panel loading is complete
    /// @param trigger_id ID of the trigger that initiated the panel switch
    void TriggerPanelSwitchCallback(const char *triggerId);

private:
    // Constructors and Destructors (private default constructor for singleton compatibility)
    PanelManager();

    // Core Functionality Methods
    /// @brief Register all available panels
    void RegisterAllPanels();
    
    /// @brief Create a panel instance by name using the registered factory
    /// @param panel_name Name of the panel type to create
    /// @return Shared pointer to the created panel instance
    std::shared_ptr<IPanel> CreatePanel(const char *panelName);
    
    
    

    // Callback Methods
    /// @brief Callback executed when splash screen loading is complete
    /// @param panel_name Name of the target panel to load after splash
    void SplashCompletionCallback(const char* panelName);
    
    /// @brief Callback executed when normal panel loading is complete
    void PanelCompletionCallback();
    
    
    
    /// @brief Notify Core 1 of state changes
    /// @param panel_name Current panel name
    /// @param theme_name Current theme name
    void NotifyCore1StateChange(const char* panelName, const char* themeName);

public:
    // Public Data Members
    const char* currentPanel = PanelNames::OIL;     ///< Current panel for Core 1 sync
    const char* restorationPanel = PanelNames::OIL; ///< Panel to restore when all triggers are inactive

private:
    // Instance Data Members
    std::shared_ptr<IPanel> panel_ = nullptr;
    std::map<std::string, std::function<std::shared_ptr<IPanel>()>> registeredPanels_; // Map of panel type names to creator functions for each of those names
    UIState uiState_ = UIState::IDLE;             ///< Current UI processing state
    char currentPanelBuffer[32];                  ///< Buffer for current panel name to avoid pointer issues
    IGpioProvider* gpioProvider_ = nullptr;       ///< GPIO provider for hardware access
    IDisplayProvider* displayProvider_ = nullptr; ///< Display provider for UI operations
    // Removed queue handles - now using shared state trigger system
};