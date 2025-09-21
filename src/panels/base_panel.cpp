#include "panels/base_panel.h"
#include "managers/error_manager.h"
#include "definitions/constants.h"
#include "utilities/logging.h"
#include <Arduino.h>

// ========== Constructors and Destructor ==========

/**
 * @brief Initialize BasePanel with dependency injection of required services
 */
BasePanel::BasePanel(IGpioProvider* gpio, IDisplayProvider* display, IStyleService* styleService)
    : gpioProvider_(gpio), displayProvider_(display), styleService_(styleService),
      panelService_(nullptr)
{
    log_v("BasePanel constructor called");
}

/**
 * @brief Destructor ensuring proper LVGL screen cleanup
 * @details Deletes LVGL screen object to prevent memory leaks
 */
BasePanel::~BasePanel()
{
    log_v("BasePanel destructor called");

    if (screen_)
    {
        lv_obj_delete(screen_);
        screen_ = nullptr;
    }
}

// ========== Public Interface Methods ==========

/**
 * @brief Template method for panel initialization
 * @details Validates providers, calls CustomInit() hook, sets up screen and center location
 */
void BasePanel::Init()
{
    log_v("%s::Init() called", GetPanelName());

    ValidateProviders();

    // Allow derived classes to perform custom initialization
    CustomInit();

    SetupScreen();

    // Set up center location for component positioning
    centerLocation_ = ComponentLocation(LV_ALIGN_CENTER, 0, 0);

    log_i("%s initialization completed", GetPanelName());
}

/**
 * @brief Template method for panel loading
 * @details Calls CreateContent(), PostLoad() hooks, sets up callbacks, and displays screen
 */
void BasePanel::Load()
{
    log_v("%s::Load() called", GetPanelName());

    // Create panel-specific content (implemented by derived classes)
    CreateContent();

    // Allow derived classes to perform additional setup
    PostLoad();

    // Set up LVGL event callback for screen load completion
    lv_obj_add_event_cb(screen_, BasePanel::ShowPanelCompletionCallback, LV_EVENT_SCREEN_LOADED, this);

    ApplyThemeAndLoadScreen();

    log_t("%s loaded successfully", GetPanelName());
}

/**
 * @brief Template method for panel updates
 * @details Calls UpdateContent() hook and resets UI state to IDLE
 */
void BasePanel::Update()
{
    log_v("%s::Update() called", GetPanelName());

    // Update panel-specific content (implemented by derived classes)
    UpdateContent();

    // Reset UI state to IDLE after update (common pattern across all panels)
    if (panelService_)
    {
        panelService_->SetUiState(UIState::IDLE);
    }
}

/**
 * @brief Inject manager service dependencies
 * @details Updates panel and style service references for runtime services
 */
void BasePanel::SetManagers(IPanelService* panelService, IStyleService* styleService)
{
    log_v("%s::SetManagers() called", GetPanelName());

    panelService_ = panelService;

    // Update styleService if different instance provided
    if (styleService != styleService_)
    {
        styleService_ = styleService;
    }
}

/**
 * @brief Get short press handler function pointer
 */

// ========== Static Methods ==========

/**
 * @brief LVGL callback for screen load completion
 * @details Resets UI state to IDLE to allow trigger evaluation after panel display
 */
void BasePanel::ShowPanelCompletionCallback(lv_event_t* event)
{
    if (!event)
        return;

    auto thisInstance = static_cast<BasePanel*>(lv_event_get_user_data(event));
    if (!thisInstance)
        return;

    log_v("%s::ShowPanelCompletionCallback() called", thisInstance->GetPanelName());

    // Set UI state to IDLE after panel loads so triggers can be evaluated again
    if (thisInstance->panelService_)
    {
        thisInstance->panelService_->SetUiState(UIState::IDLE);
    }
}

/**
 * @brief Static wrapper for short button press - delegates to virtual method
 */
void BasePanel::BasePanelShortPress(void* panelContext)
{
    auto* panel = static_cast<BasePanel*>(panelContext);
    if (panel)
    {
        log_v("%s::BasePanelShortPress() called", panel->GetPanelName());
        panel->HandleShortPress();
    }
}

/**
 * @brief Static wrapper for long button press - delegates to virtual method
 */
void BasePanel::BasePanelLongPress(void* panelContext)
{
    auto* panel = static_cast<BasePanel*>(panelContext);
    if (panel)
    {
        log_v("%s::BasePanelLongPress() called", panel->GetPanelName());
        panel->HandleLongPress();
    }
}

// ========== Private Methods ==========

/**
 * @brief Validate required providers are available
 * @details Reports critical error if display or GPIO provider is missing
 */
void BasePanel::ValidateProviders()
{
    if (!displayProvider_ || !gpioProvider_)
    {
        log_e("%s requires display and gpio providers", GetPanelName());
        ErrorManager::Instance().ReportCriticalError(GetPanelName(),
                                                     "Missing required providers - display or gpio provider is null");
        return;
    }
}

/**
 * @brief Create LVGL screen and apply current theme
 */
void BasePanel::SetupScreen()
{
    screen_ = displayProvider_->CreateScreen();

    // Apply current theme immediately after screen creation
    if (styleService_)
    {
        styleService_->ApplyThemeToScreen(screen_);
    }
}

/**
 * @brief Load screen and ensure current theme is applied
 * @details Double-applies theme to ensure consistency after screen load
 */
void BasePanel::ApplyThemeAndLoadScreen()
{
    lv_screen_load(screen_);

    // Always apply current theme to the screen when loading (ensures theme is current)
    if (styleService_)
    {
        styleService_->ApplyThemeToScreen(screen_);
    }
}