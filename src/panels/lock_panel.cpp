#include "panels/lock_panel.h"
#include "factories/component_factory.h"
#include "interfaces/i_component_factory.h"
#include "managers/style_manager.h"
#include <esp32-hal-log.h>
#include <variant>

// Constructors and Destructors
LockPanel::LockPanel(IGpioProvider *gpio, IDisplayProvider *display, IStyleService *styleService,
                     IComponentFactory* componentFactory)
    : gpioProvider_(gpio), displayProvider_(display), styleService_(styleService),
      componentFactory_(componentFactory ? componentFactory : &ComponentFactory::Instance()),
      lockSensor_(std::make_shared<LockSensor>(gpio))
{
    // Component will be created during load() method
}

LockPanel::~LockPanel()
{
    if (screen_)
    {
        lv_obj_delete(screen_);
    }

    if (lockComponent_)
    {
        lockComponent_.reset();
    }

    if (lockSensor_)
    {
        lockSensor_.reset();
    }
}

// Core Functionality Methods

/// @brief Initialize the lock panel and its components
void LockPanel::Init()
{
    // Initializing lock panel with sensor and display components

    screen_ = displayProvider_->CreateScreen();

    // Apply current theme immediately after screen creation
    if (styleService_)
    {
        styleService_->ApplyThemeToScreen(screen_);
    }
    centerLocation_ = ComponentLocation(LV_ALIGN_CENTER, 0, 0);

    lockSensor_->Init();
    isLockEngaged_ = false;
}

/// @brief Load the lock panel UI components
void LockPanel::Load(std::function<void()> callbackFunction)
{
    // Loading lock panel with current lock state display
    callbackFunction_ = callbackFunction;

    // Create component using injected factory
    lockComponent_ = componentFactory_->CreateLockComponent(styleService_);

    // Create the lock component centered on screen, and immediately refresh it with the current lock status
    lockComponent_->Render(screen_, centerLocation_, displayProvider_);
    lockComponent_->Refresh(Reading{isLockEngaged_});
    lv_obj_add_event_cb(screen_, LockPanel::ShowPanelCompletionCallback, LV_EVENT_SCREEN_LOADED, this);

    log_v("loading...");
    lv_screen_load(screen_);

    // Always apply current theme to the screen when loading (ensures theme is current)
    if (styleService_)
    {
        styleService_->ApplyThemeToScreen(screen_);
    }
}

/// @brief Update the lock panel with current sensor data
void LockPanel::Update(std::function<void()> callbackFunction)
{
    callbackFunction();
}

// Static Methods

/// @brief The callback to be run once show panel has completed
/// @param event LVGL event that was used to call this
void LockPanel::ShowPanelCompletionCallback(lv_event_t *event)
{
    // Lock panel load completed - screen displayed

    auto thisInstance = static_cast<LockPanel *>(lv_event_get_user_data(event));
    thisInstance->callbackFunction_();
}

// Manager injection method
void LockPanel::SetManagers(IPanelService *panelService, IStyleService *styleService)
{
    // LockPanel doesn't use panelService (no actions), but update styleService if different
    if (styleService != styleService_)
    {
        styleService_ = styleService;
    }
    // Managers injected successfully
}