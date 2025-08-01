#include "panels/lock_panel.h"
#include "factories/ui_factory.h"
#include "managers/style_manager.h"
#include <variant>

// Constructors and Destructors
LockPanel::LockPanel(IGpioProvider* gpio, IDisplayProvider* display, IStyleService* styleService)
    : gpioProvider_(gpio), displayProvider_(display), styleService_(styleService),
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
void LockPanel::init(IGpioProvider* gpio, IDisplayProvider* display)
{
    log_d("Initializing lock panel with sensor and display components");

    screen_ = display->createScreen();
    centerLocation_ = ComponentLocation(LV_ALIGN_CENTER, 0, 0);

    lockSensor_->init();
    isLockEngaged_ = false;
}

/// @brief Load the lock panel UI components
void LockPanel::load(std::function<void()> callbackFunction, IGpioProvider* gpio, IDisplayProvider* display)
{
    log_d("Loading lock panel with current lock state display");
    callbackFunction_ = callbackFunction;

    // Create component directly using UIFactory
    lockComponent_ = UIFactory::createLockComponent(styleService_);

    // Create the lock component centered on screen, and immediately refresh it with the current lock status
    lockComponent_->render(screen_, centerLocation_, display);
    lockComponent_->refresh(Reading{isLockEngaged_});
    lv_obj_add_event_cb(screen_, LockPanel::ShowPanelCompletionCallback, LV_EVENT_SCREEN_LOADED, this);

    log_v("loading...");
    lv_screen_load(screen_);
}

/// @brief Update the lock panel with current sensor data
void LockPanel::update(std::function<void()> callbackFunction, IGpioProvider* gpio, IDisplayProvider* display)
{
    // Immediately call the completion callback so that lock/unlock logic is processed
    callbackFunction();
}

// Static Methods

/// @brief The callback to be run once show panel has completed
/// @param event LVGL event that was used to call this
void LockPanel::ShowPanelCompletionCallback(lv_event_t *event)
{
    log_d("Lock panel load completed - screen displayed");

    auto thisInstance = static_cast<LockPanel *>(lv_event_get_user_data(event));
    thisInstance->callbackFunction_();
}