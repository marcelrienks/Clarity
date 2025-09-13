#pragma once
#include "interfaces/i_display_provider.h"
#include "interfaces/i_gpio_provider.h"
#include "interfaces/i_panel.h"
#include "interfaces/i_panel_service.h"
#include "interfaces/i_preference_service.h"
#include "interfaces/i_style_service.h"
#include "interfaces/i_panel_notification_service.h"
#include "interfaces/i_action_execution_service.h"
#include "interfaces/i_trigger_execution_service.h"
#include "utilities/types.h"

#include <functional>
#include <memory>
#include <string>

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

// Forward declarations
class StyleManager;
class InterruptManager;

class PanelManager : public IPanelService,
                     public IPanelNotificationService,
                     public IActionExecutionService,
                     public ITriggerExecutionService
{
  public:
    // Constructors and Destructors
    PanelManager(IDisplayProvider *display, IGpioProvider *gpio, IStyleService *styleService,
                 IPreferenceService *preferenceService, InterruptManager* interruptManager = nullptr);
    PanelManager(const PanelManager &) = delete;
    PanelManager &operator=(const PanelManager &) = delete;
    ~PanelManager();

    // Core Functionality Methods (IPanelService implementation)
    /// @brief Initialize the panel service and register available panels
    void Init() override;

    /// @brief Set current UI state for synchronization
    /// @param state Current UI processing state
    void SetUiState(UIState state) override;

    /// @brief Get the current UI state
    /// @return Current UI processing state
    UIState GetUiState() const override;

    /// @brief Create and load a panel by name
    /// @param panelName Name of the panel to create and load
    /// @param isTriggerDriven Whether this panel change is triggered by an interrupt trigger
    void CreateAndLoadPanel(const char *panelName, bool isTriggerDriven = false) override;

    /// @brief Update the currently active panel (called from main loop)
    void UpdatePanel() override;
    
    /// @brief Update universal button interrupts with current panel's functions
    /// @param panel The panel to extract button functions from
    void UpdatePanelButtonFunctions(IPanel* panel);

    // State Management Methods (IPanelService implementation)
    /// @brief Get the current panel name
    /// @return Current panel identifier string
    const char *GetCurrentPanel() const override;

    /// @brief Get the restoration panel name (panel to restore when triggers are inactive)
    /// @return Restoration panel identifier string
    const char *GetRestorationPanel() const override;

    /// @brief Check if the current panel is trigger-driven
    /// @return True if current panel was loaded by a trigger, false for user-driven panels
    bool IsCurrentPanelTriggerDriven() const override;

    // Trigger Integration Methods (IPanelService implementation)
    /// @brief Callback executed when trigger-driven panel loading is complete
    /// @param triggerId ID of the trigger that initiated the panel switch
    void TriggerPanelSwitchCallback(const char *triggerId) override;

  private:
    // Core Functionality Methods
    
    /// @brief Register all available panels
    void RegisterAllPanels();

    /// @brief Create a panel instance by name using the registered factory
    /// @param panel_name Name of the panel type to create
    /// @return Shared pointer to the created panel instance
    std::shared_ptr<IPanel> CreatePanel(const char *panelName);

    /// @brief Internal method to create and load a panel directly without splash
    /// @param panelName Name of the panel to create and load
    /// @param isTriggerDriven Whether this panel change is triggered by an interrupt trigger
    void CreateAndLoadPanelDirect(const char *panelName, bool isTriggerDriven = false);

    /// @brief Internal method to load a panel after first showing a splash screen transition
    /// @param panelName Name of the target panel to load after splash
    /// @param isTriggerDriven Whether the target panel load is trigger-driven
    void CreateAndLoadPanelWithSplash(const char *panelName, bool isTriggerDriven);

    // Callback Methods
    
    /// @brief Callback executed when splash screen loading is complete
    /// @param panel_name Name of the target panel to load after splash
    void SplashCompletionCallback(const char *panelName);

    /// @brief Callback executed when normal panel loading is complete
    void PanelCompletionCallback();

  public:
    // Singleton access for new interrupt architecture
    static PanelManager& Instance();
    
    // Interface accessors for dependency injection (CRITICAL for testability)
    static IPanelNotificationService& NotificationService() { return Instance(); }
    static IActionExecutionService& ActionService() { return Instance(); }
    static ITriggerExecutionService& TriggerService() { return Instance(); }
    
    // IPanelNotificationService implementation
    void OnPanelLoadComplete(IPanel* panel) override;
    void OnPanelUpdateComplete(IPanel* panel) override;
    
    // IActionExecutionService implementation  
    void HandleShortPress() override;
    void HandleLongPress() override;
    
    // ITriggerExecutionService implementation
    void LoadPanel(const char* panelName) override;
    void CheckRestoration() override;
    
    // Public Data Members - using std::string for safety (see docs/guidelines.md)
    std::string currentPanel = PanelNames::OIL;     ///< Current panel state
    std::string restorationPanel = PanelNames::OIL; ///< Panel to restore when all triggers are inactive

  private:
    // Instance Data Members
    std::shared_ptr<IPanel> panel_ = nullptr;
    UIState uiState_ = UIState::IDLE;          ///< Current UI processing state
    bool currentPanelIsTriggerDriven_ = false; ///< Track if current panel is trigger-driven

    // Panel name storage - using std::string for safety (see docs/guidelines.md)
    std::string lastUserPanel_ = PanelNames::OIL;    ///< Last user-driven panel
    std::string splashTargetPanel_;                   ///< Target panel for splash transition
    bool splashTargetTriggerDriven_ = false;          ///< Preserve trigger state through splash transitions
    IGpioProvider *gpioProvider_ = nullptr;           ///< GPIO provider for hardware access
    IDisplayProvider *displayProvider_ = nullptr;     ///< Display provider for UI operations
    IStyleService *styleService_ = nullptr;           ///< Style service for UI theming
    // IActionManager removed - button handling moved to handler-based system
    InterruptManager *interruptManager_ = nullptr;    ///< Interrupt manager for button function injection
    IPreferenceService *preferenceService_ = nullptr; ///< Preference service for configuration settings

    // Cached service references to avoid repeated singleton calls
    class ErrorManager& errorManager_;                ///< Cached ErrorManager reference
};