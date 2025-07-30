#include "managers/panel_manager.h"

// Static Methods

/// @brief Get the singleton instance of PanelManager
/// @return instance of PanelManager
PanelManager &PanelManager::GetInstance()
{
    // Return the factory-created global instance for consistency with new architecture
    extern std::unique_ptr<PanelManager> g_panelManager;
    if (g_panelManager) {
        return *g_panelManager;
    }
    
    // Fallback to static instance if global not available (shouldn't happen in normal flow)
    static PanelManager instance;
    return instance;
}

// Core Functionality Methods

/// @brief Initialise the panel manager to control the flow and rendering of all panels
/// Registers all available panel types with the factory for dynamic creation
/// Hardware providers are already injected via constructor
void PanelManager::init()
{
    log_d("Initializing panel manager...");
    
    // Register all available panel types with the factory
    RegisterAllPanels();
    
    Ticker::handle_lv_tasks();
}

/// @brief Initialize the panel manager with hardware providers (backward compatibility)
/// @param gpio GPIO provider for hardware access
/// @param display Display provider for UI operations
void PanelManager::init(IGpioProvider* gpio, IDisplayProvider* display)
{
    log_d("Initializing panel manager with legacy init method...");
    
    // Store hardware providers if not already set
    if (!gpioProvider_) gpioProvider_ = gpio;
    if (!displayProvider_) displayProvider_ = display;
    
    // Call the main init method
    init();
}

void PanelManager::RegisterAllPanels()
{
    log_d("Registering all panels...");

    // DEPRECATED: Old template-based registration system disabled during DI migration
    // Panel registration is now handled by ComponentRegistry in main.cpp
    // This method will be removed in Step 4 of the architecture migration
    
    log_d("Panel registration now handled by ComponentRegistry - old system disabled");
}

/// @brief Creates and then loads a panel based on the given name
/// @param panelName the name of the panel to be loaded
/// @param completionCallback the function to be called when the panel load is complete
/// @param isTriggerDriven whether this panel change is triggered by an interrupt trigger
void PanelManager::CreateAndLoadPanel(const char *panelName, std::function<void()> completionCallback, bool isTriggerDriven)
{
    log_d("Creating and loading panel: %s (trigger-driven: %s)", panelName, isTriggerDriven ? "yes" : "no");

    // Track this as the last non-trigger panel only for user-driven changes
    if (!isTriggerDriven)
    {
        restorationPanel = panelName;
    }

    // Clean up existing panel before creating new one
    if (panel_)
    {
        log_d("Cleaning up existing panel before creating new one");
        panel_.reset();
    }

    panel_ = PanelManager::CreatePanel(panelName);
    panel_->init(gpioProvider_, displayProvider_);

    // Make a copy of the panel name to avoid pointer issues
    strncpy(currentPanelBuffer, panelName, sizeof(currentPanelBuffer) - 1);
    currentPanelBuffer[sizeof(currentPanelBuffer) - 1] = '\0';
    currentPanel = currentPanelBuffer;

    SetUiState(UIState::LOADING);
    panel_->load(completionCallback, gpioProvider_, displayProvider_);
    Ticker::handle_lv_tasks();
}

/// @brief Loads a panel based on the given name after first loading a splash screen
/// This function will create the panel and then call the load function on it.
/// @param panelName the name of the panel to be loaded
void PanelManager::CreateAndLoadPanelWithSplash(const char *panelName)
{
    log_d("Loading panel with splash screen transition: %s", panelName);

    CreateAndLoadPanel(PanelNames::SPLASH, [this, panelName]()
                       { this->PanelManager::SplashCompletionCallback(panelName); });
}

/// @brief Update the reading on the currently loaded panel and process trigger messages
void PanelManager::UpdatePanel()
{
    SetUiState(UIState::UPDATING);
    panel_->update([this]()
                   { this->PanelManager::PanelCompletionCallback(); }, gpioProvider_, displayProvider_);

    Ticker::handle_lv_tasks();
}

// Constructors and Destructors

PanelManager::PanelManager(IDisplayProvider* display, IGpioProvider* gpio, IPanelFactory* panelFactory)
    : gpioProvider_(gpio), displayProvider_(display), panelFactory_(panelFactory)
{
    if (display || gpio) {
        log_d("Creating PanelManager with injected dependencies");
    } else {
        log_d("Creating PanelManager with default constructor (for singleton compatibility)");
    }
    
    // Initialize the current panel buffer with the default value
    strncpy(currentPanelBuffer, PanelNames::OIL, sizeof(currentPanelBuffer) - 1);
    currentPanelBuffer[sizeof(currentPanelBuffer) - 1] = '\0';
    currentPanel = currentPanelBuffer;
}

PanelManager::~PanelManager()
{
    panel_.reset();
}

// Private Methods

/// @brief Create a panel based on the given type name
/// @param panelName the type name of the panel to be created
/// @return Interface type representing the panel
std::shared_ptr<IPanel> PanelManager::CreatePanel(const char *panelName)
{
    log_d("Creating panel instance for type: %s", panelName);

    // Use the new PanelFactory approach for dependency injection
    if (panelFactory_) {
        auto uniquePanel = panelFactory_->createPanel(panelName);
        return std::shared_ptr<IPanel>(uniquePanel.release());
    }
    
    // Fallback to old template-based approach (DEPRECATED - will be removed in Step 4)
    log_w("PanelFactory not available, falling back to old template-based creation");
    auto iterator = registeredPanels_.find(panelName);
    if (iterator != registeredPanels_.end()) {
        return iterator->second(); // Return the function stored in the map
    }
    
    log_e("Panel type not found: %s", panelName);
    return nullptr;
}



// Callback Methods

/// @brief callback function to be executed on splash panel show completion
void PanelManager::SplashCompletionCallback(const char *panelName)
{
    log_d("Splash screen animation completed, transitioning to panel: %s", panelName);

    panel_.reset();
    Ticker::handle_lv_tasks();

    CreateAndLoadPanel(panelName, [this]()
                       { this->PanelManager::PanelCompletionCallback(); });
}

/// @brief callback function to be executed on panel show completion
void PanelManager::PanelCompletionCallback()
{
    SetUiState(UIState::IDLE);
    
    // System initialization complete - triggers will remain in INIT state
    // until actual GPIO pin changes occur
    static bool systemInitialized = false;
    if (!systemInitialized)
    {
        systemInitialized = true;
    }
}

/// @brief callback function to be executed when trigger-driven panel loading is complete
void PanelManager::TriggerPanelSwitchCallback(const char *triggerId)
{
    SetUiState(UIState::IDLE);
    // No need to clear triggers - GPIO state manages trigger active/inactive status
}


/// @brief Set current UI state for Core 1 synchronization
void PanelManager::SetUiState(UIState state)
{
    uiState_ = state;
}



