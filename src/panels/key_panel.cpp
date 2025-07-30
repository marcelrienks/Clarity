#include "panels/key_panel.h"
#include "managers/style_manager.h"
#include "hardware/gpio_pins.h"
#include <variant>
#include <Arduino.h>

// Constructors and Destructors
KeyPanel::KeyPanel(IComponentFactory* componentFactory) 
    : componentFactory_(componentFactory) 
{
    // Component will be created during load() method using the component factory
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

    // TODO: Remove fallback when providers are fully implemented in Step 4
    if (display) {
        screen_ = display->createScreen();
    } else {
        // Fallback to direct LVGL calls for now
        screen_ = LvTools::create_blank_screen();
    }
    centerLocation_ = ComponentLocation(LV_ALIGN_CENTER, 0, 0);

    // Get current key state by reading GPIO pins via provider
    // TODO: Remove fallback when providers are fully implemented in Step 4
    bool pin25High, pin26High;
    if (gpio) {
        pin25High = gpio->digitalRead(gpio_pins::KEY_PRESENT);
        pin26High = gpio->digitalRead(gpio_pins::KEY_NOT_PRESENT);
    } else {
        // Fallback to direct GPIO calls for now
        pin25High = digitalRead(gpio_pins::KEY_PRESENT);
        pin26High = digitalRead(gpio_pins::KEY_NOT_PRESENT);
    }
    
    if (pin25High && pin26High)
    {
        currentKeyState_ = KeyState::Inactive; // Both pins HIGH - invalid state
    }
    else if (pin25High)
    {
        currentKeyState_ = KeyState::Present;
    }
    else if (pin26High)
    {
        currentKeyState_ = KeyState::NotPresent;
    }
    else
    {
        currentKeyState_ = KeyState::Inactive; // Both pins LOW
    }
}

/// @brief Load the key panel UI components
void KeyPanel::load(std::function<void()> callbackFunction, IGpioProvider* gpio, IDisplayProvider* display)
{
    log_d("Loading key panel with current key state display");
    callbackFunction_ = callbackFunction;

    // Create component using the injected component factory
    IStyleService* styleService = &StyleManager::GetInstance();
    keyComponent_ = componentFactory_->createComponent("key", display, styleService);

    // Render the component
    if (display) {
        keyComponent_->render(screen_, centerLocation_, display);
    } else {
        // Fallback - components need to handle nullptr display
        keyComponent_->render(screen_, centerLocation_, nullptr);
    }
    keyComponent_->refresh(Reading{static_cast<int32_t>(currentKeyState_)});
    
    lv_obj_add_event_cb(screen_, KeyPanel::ShowPanelCompletionCallback, LV_EVENT_SCREEN_LOADED, this);

    log_v("loading...");
    lv_screen_load(screen_);
}

/// @brief Update the key panel with current sensor data
void KeyPanel::update(std::function<void()> callbackFunction, IGpioProvider* gpio, IDisplayProvider* display)
{

    // Re-read GPIO pins to determine current key state
    bool pin25High = digitalRead(gpio_pins::KEY_PRESENT);
    bool pin26High = digitalRead(gpio_pins::KEY_NOT_PRESENT);
    
    KeyState newKeyState;
    if (pin25High && pin26High)
    {
        newKeyState = KeyState::Inactive; // Both pins HIGH - invalid state
    }
    else if (pin25High)
    {
        newKeyState = KeyState::Present;
    }
    else if (pin26High)
    {
        newKeyState = KeyState::NotPresent;
    }
    else
    {
        newKeyState = KeyState::Inactive; // Both pins LOW
    }
    
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