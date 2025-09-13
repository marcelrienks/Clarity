#include "panels/base_panel.h"
#include "managers/error_manager.h"
#include "utilities/constants.h"
#include "utilities/logging.h"
#include <Arduino.h>

// Constructors and Destructors
BasePanel::BasePanel(IGpioProvider* gpio, IDisplayProvider* display, IStyleService* styleService)
    : gpioProvider_(gpio), displayProvider_(display), styleService_(styleService),
      panelService_(nullptr)
{
    log_v("BasePanel constructor called");
}

BasePanel::~BasePanel()
{
    log_v("BasePanel destructor called");

    if (screen_)
    {
        lv_obj_delete(screen_);
        screen_ = nullptr;
    }
}

// Core Functionality Methods - Template Method implementations

void BasePanel::Init()
{
    log_v("%s::Init() called", GetPanelName());

    ValidateProviders();

    // Allow derived classes to perform custom initialization
    CustomInit();

    SetupScreen();

    // Set up center location for components
    centerLocation_ = ComponentLocation(LV_ALIGN_CENTER, 0, 0);

    log_i("%s initialization completed", GetPanelName());
}

void BasePanel::Load()
{
    log_v("%s::Load() called", GetPanelName());

    // Create panel-specific content (implemented by derived classes)
    CreateContent();

    // Allow derived classes to perform additional setup
    PostLoad();

    // Set up LVGL event callback and load screen
    lv_obj_add_event_cb(screen_, BasePanel::ShowPanelCompletionCallback, LV_EVENT_SCREEN_LOADED, this);

    ApplyThemeAndLoadScreen();

    log_t("%s loaded successfully", GetPanelName());
}

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

// Manager injection method
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

// IActionService Interface Implementation

void (*BasePanel::GetShortPressFunction())(void* panelContext)
{
    return BasePanelShortPress;
}

void (*BasePanel::GetLongPressFunction())(void* panelContext)
{
    return BasePanelLongPress;
}

void* BasePanel::GetPanelContext()
{
    return this;
}

// Private Methods

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

void BasePanel::SetupScreen()
{
    screen_ = displayProvider_->CreateScreen();

    // Apply current theme immediately after screen creation
    if (styleService_)
    {
        styleService_->ApplyThemeToScreen(screen_);
    }
}

void BasePanel::ApplyThemeAndLoadScreen()
{
    lv_screen_load(screen_);

    // Always apply current theme to the screen when loading (ensures theme is current)
    if (styleService_)
    {
        styleService_->ApplyThemeToScreen(screen_);
    }
}

// Static Methods

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

// Static button press functions for IActionService

void BasePanel::BasePanelShortPress(void* panelContext)
{
    auto* panel = static_cast<BasePanel*>(panelContext);
    if (panel)
    {
        log_v("%s::BasePanelShortPress() called", panel->GetPanelName());
        panel->HandleShortPress();
    }
}

void BasePanel::BasePanelLongPress(void* panelContext)
{
    auto* panel = static_cast<BasePanel*>(panelContext);
    if (panel)
    {
        log_v("%s::BasePanelLongPress() called", panel->GetPanelName());
        panel->HandleLongPress();
    }
}