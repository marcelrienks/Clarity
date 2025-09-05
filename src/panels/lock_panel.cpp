#include "panels/lock_panel.h"
#include "factories/component_factory.h"
#include "interfaces/i_component_factory.h"
#include "managers/style_manager.h"
#include "managers/error_manager.h"
#include "utilities/constants.h"
#include <esp32-hal-log.h>
#include <variant>

// Constructors and Destructors
LockPanel::LockPanel(IGpioProvider *gpio, IDisplayProvider *display, IStyleService *styleService,
                     IComponentFactory* componentFactory)
    : gpioProvider_(gpio), displayProvider_(display), styleService_(styleService),
      componentFactory_(componentFactory ? componentFactory : &ComponentFactory::Instance())
{
    log_v("LockPanel constructor called");
    // Component will be created during load() method
}

LockPanel::~LockPanel()
{
    log_v("~LockPanel() destructor called");
    if (screen_)
    {
        lv_obj_delete(screen_);
    }

    if (lockComponent_)
    {
        lockComponent_.reset();
    }

}

// Core Functionality Methods

/// @brief Initialize the lock panel and its components
void LockPanel::Init()
{
    log_v("Init() called");

    if (!displayProvider_ || !gpioProvider_)
    {
        log_e("LockPanel requires display and gpio providers");
        ErrorManager::Instance().ReportCriticalError("LockPanel",
                                                     "Missing required providers - display or gpio provider is null");
        return;
    }

    screen_ = displayProvider_->CreateScreen();

    // Apply current theme immediately after screen creation
    if (styleService_)
    {
        styleService_->ApplyThemeToScreen(screen_);
    }
    centerLocation_ = ComponentLocation(LV_ALIGN_CENTER, 0, 0);

    // Note: LockPanel is display-only, state comes from interrupt system
    // Since this panel is only loaded when lock state changes, assume engaged
    isLockEngaged_ = true;
    
    log_i("LockPanel initialization completed");
}

/// @brief Load the lock panel UI components
void LockPanel::Load()
{
    log_v("Load() called");


    if (!componentFactory_)
    {
        log_e("LockPanel requires component factory");
        ErrorManager::Instance().ReportError(ErrorLevel::ERROR, "LockPanel", "ComponentFactory is null");
        return;
    }

    // Create component using injected factory
    lockComponent_ = componentFactory_->CreateLockComponent(styleService_);
    if (!lockComponent_)
    {
        log_e("Failed to create lock component");
        ErrorManager::Instance().ReportError(ErrorLevel::ERROR, "LockPanel", "Component creation failed");
        return;
    }

    // Create the lock component centered on screen - always shows red (engaged) state
    lockComponent_->Render(screen_, centerLocation_, displayProvider_);
    lv_obj_add_event_cb(screen_, LockPanel::ShowPanelCompletionCallback, LV_EVENT_SCREEN_LOADED, this);

    lv_screen_load(screen_);

    // Always apply current theme to the screen when loading (ensures theme is current)
    if (styleService_)
    {
        styleService_->ApplyThemeToScreen(screen_);
    }
    
    log_i("LockPanel loaded successfully");
}

/// @brief Update the lock panel with current sensor data
void LockPanel::Update()
{
    log_v("Update() called");
    
    // Lock panel is static - no updates needed, but must reset UI state to IDLE
    if (panelService_) {
        panelService_->SetUiState(UIState::IDLE);
        log_d("LockPanel: Update() - setting UI state to IDLE after static panel update");
    }
}

// Static Methods

/// @brief The callback to be run once show panel has completed
/// @param event LVGL event that was used to call this
void LockPanel::ShowPanelCompletionCallback(lv_event_t *event)
{
    log_v("ShowPanelCompletionCallback() called");
    
    if (!event)
        return;

    auto thisInstance = static_cast<LockPanel *>(lv_event_get_user_data(event));
    if (!thisInstance)
        return;
    
    // Set UI state to IDLE after static panel loads so triggers can be evaluated again
    if (thisInstance->panelService_)
    {
        thisInstance->panelService_->SetUiState(UIState::IDLE);
        log_d("LockPanel: ShowPanelCompletionCallback - setting UI state to IDLE");
    }
}

// Manager injection method
void LockPanel::SetManagers(IPanelService *panelService, IStyleService *styleService)
{
    log_v("SetManagers() called");
    panelService_ = panelService;

    // Update styleService if different instance provided
    if (styleService != styleService_)
    {
        styleService_ = styleService;
    }
}

// IActionService Interface Implementation

static void LockPanelShortPress(void* panelContext)
{
    log_v("LockPanelShortPress() called");
    // Display-only panel, no action for short press
}

static void LockPanelLongPress(void* panelContext)
{
    log_v("LockPanelLongPress() called");
    
    auto* panel = static_cast<LockPanel*>(panelContext);
    if (panel)
    {
        // Call public method to handle the long press
        panel->HandleLongPress();
    }
}

void (*LockPanel::GetShortPressFunction())(void* panelContext)
{
    return LockPanelShortPress;
}

void (*LockPanel::GetLongPressFunction())(void* panelContext)
{
    return LockPanelLongPress;
}

void* LockPanel::GetPanelContext()
{
    return this;
}

void LockPanel::HandleLongPress()
{
    if (panelService_)
    {
        log_i("LockPanel long press - loading config panel");
        panelService_->CreateAndLoadPanel(PanelNames::CONFIG, true);
    }
    else
    {
        log_w("LockPanel: Cannot load config panel - panelService not available");
    }
}