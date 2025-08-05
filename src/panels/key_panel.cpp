#include "panels/key_panel.h"
#include "factories/ui_factory.h"
#include "managers/style_manager.h"
#include <variant>
#include <Arduino.h>

// Constructors and Destructors
KeyPanel::KeyPanel(IGpioProvider* gpio, IDisplayProvider* display, IStyleService* styleService) 
    : gpioProvider_(gpio), displayProvider_(display), styleService_(styleService),
      keySensor_(std::make_shared<KeySensor>(gpio))
{
    // Component will be created during load() method
}

KeyPanel::~KeyPanel()
{
    if (screen_) {
        lv_obj_delete(screen_);
    }

    if (keyComponent_)
    {
        keyComponent_.reset();
    }
}

// Core Functionality Methods
/// @brief Initialize the key panel and its components
void KeyPanel::init(IGpioProvider* gpio, IDisplayProvider* display)
{
    log_d("Initializing key panel and reading current GPIO key state");

    if (!display || !gpio) {
        log_e("KeyPanel requires display and gpio providers");
        return;
    }

    screen_ = display->createScreen();
    
    // Apply current theme immediately after screen creation
    if (styleService_) {
        styleService_->applyThemeToScreen(screen_);
    }
    centerLocation_ = ComponentLocation(LV_ALIGN_CENTER, 0, 0);

    // Initialize sensor and get current key state
    keySensor_->init();
    currentKeyState_ = keySensor_->getKeyState();
}

/// @brief Load the key panel UI components
void KeyPanel::load(std::function<void()> callbackFunction, IGpioProvider* gpio, IDisplayProvider* display)
{
    log_d("Loading key panel with current key state display");
    callbackFunction_ = callbackFunction;

    // Create component directly using UIFactory
    keyComponent_ = UIFactory::createKeyComponent(styleService_);

    // Render the component
    if (!display) {
        log_e("KeyPanel load requires display provider");
        return;
    }
    keyComponent_->render(screen_, centerLocation_, display);
    keyComponent_->refresh(Reading{static_cast<int32_t>(currentKeyState_)});
    
    lv_obj_add_event_cb(screen_, KeyPanel::ShowPanelCompletionCallback, LV_EVENT_SCREEN_LOADED, this);

    log_v("loading...");
    lv_screen_load(screen_);
    
    // Always apply current theme to the screen when loading (ensures theme is current)
    if (styleService_) {
        styleService_->applyThemeToScreen(screen_);
    }
}

/// @brief Update the key panel with current sensor data
void KeyPanel::update(std::function<void()> callbackFunction, IGpioProvider* gpio, IDisplayProvider* display)
{
    if (!gpio) {
        log_e("KeyPanel update requires gpio provider");
        return;
    }

    // Get current key state from sensor
    KeyState newKeyState = keySensor_->getKeyState();
    
    // Update display if key state has changed
    if (newKeyState != currentKeyState_)
    {
        log_d("Key state changed from %d to %d - updating display", (int)currentKeyState_, (int)newKeyState);
        currentKeyState_ = newKeyState;
        keyComponent_->refresh(Reading{static_cast<int32_t>(currentKeyState_)});
    }
    
    callbackFunction();
}

// Static Methods
/// @brief The callback to be run once show panel has completed
/// @param event LVGL event that was used to call this
void KeyPanel::ShowPanelCompletionCallback(lv_event_t *event)
{
    log_d("Key panel load completed - screen displayed");

    auto thisInstance = static_cast<KeyPanel *>(lv_event_get_user_data(event));
    thisInstance->callbackFunction_();
}