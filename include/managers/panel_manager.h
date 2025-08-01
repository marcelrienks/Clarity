#pragma once // preventing duplicate definitions, alternative to the traditional include guards

#include "interfaces/i_panel.h"
#include "interfaces/i_gpio_provider.h"
#include "interfaces/i_display_provider.h"
#include "interfaces/i_style_service.h"
#include "factories/ui_factory.h"
#include "interfaces/i_panel_service.h"
#include "utilities/types.h"

#include <string>
#include <functional>
#include <memory>

/**
 * @class PanelManager
 * @brief Panel lifecycle management and transitions service implementing IPanelService
 * 
 * @details This service handles the complete lifecycle of panels including
 * creation, loading, updating, and transitions. It implements dependency injection
 * patterns to provide centralized panel management with dynamic panel creation.
 * 
 * @design_patterns:
 * - Dependency Injection: Dependencies injected via constructor
 * - Factory: Dynamic panel creation via IPanelFactory
 * - Service: Implements IPanelService interface
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

class PanelManager : public IPanelService
{
public:
    // Constructors and Destructors
    PanelManager(IDisplayProvider* display, IGpioProvider* gpio, IStyleService* styleService);
    PanelManager(const PanelManager &) = delete;
    PanelManager &operator=(const PanelManager &) = delete;
    ~PanelManager();


    // Core Functionality Methods (IPanelService implementation)
    /// @brief Initialize the panel service and register available panels
    void init() override;
    
    
    
    /// @brief Set current UI state for synchronization
    /// @param state Current UI processing state
    void setUiState(UIState state) override;
    
    
    /// @brief Create and load a panel by name with optional completion callback
    /// @param panelName Name of the panel to create and load
    /// @param completionCallback Optional callback function to execute when loading is complete
    /// @param isTriggerDriven Whether this panel change is triggered by an interrupt trigger
    void createAndLoadPanel(const char* panelName, std::function<void()> completionCallback = nullptr, bool isTriggerDriven = false) override;
    
    /// @brief Load a panel after first showing a splash screen transition
    /// @param panelName Name of the target panel to load after splash
    void createAndLoadPanelWithSplash(const char* panelName) override;
    
    /// @brief Update the currently active panel (called from main loop)
    void updatePanel() override;
    
    


    // State Management Methods (IPanelService implementation)
    /// @brief Get the current panel name
    /// @return Current panel identifier string
    const char* getCurrentPanel() const override;
    
    /// @brief Get the restoration panel name (panel to restore when triggers are inactive)
    /// @return Restoration panel identifier string
    const char* getRestorationPanel() const override;
    
    // Trigger Integration Methods (IPanelService implementation)
    /// @brief Callback executed when trigger-driven panel loading is complete
    /// @param triggerId ID of the trigger that initiated the panel switch
    void triggerPanelSwitchCallback(const char *triggerId) override;

private:

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

public:
    // Public Data Members
    const char* currentPanel = PanelNames::OIL;     ///< Current panel state
    const char* restorationPanel = PanelNames::OIL; ///< Panel to restore when all triggers are inactive

private:
    // Instance Data Members
    std::shared_ptr<IPanel> panel_ = nullptr;
    UIState uiState_ = UIState::IDLE;             ///< Current UI processing state
    char currentPanelBuffer[32];                  ///< Buffer for current panel name to avoid pointer issues
    IGpioProvider* gpioProvider_ = nullptr;       ///< GPIO provider for hardware access
    IDisplayProvider* displayProvider_ = nullptr; ///< Display provider for UI operations
    IStyleService* styleService_ = nullptr;       ///< Style service for UI theming
    // Removed IPanelFactory - using UIFactory directly
    // Removed queue handles - now using shared state trigger system
};