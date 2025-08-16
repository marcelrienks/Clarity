#include "panels/key_panel.h"
#include "factories/component_factory.h"
#include "interfaces/i_component_factory.h"
#include "managers/error_manager.h"
#include "managers/style_manager.h"
#include <Arduino.h>
#include <variant>

// Constructors and Destructors
KeyPanel::KeyPanel(IGpioProvider *gpio, IDisplayProvider *display, IStyleService *styleService,
                   IComponentFactory* componentFactory)
    : gpioProvider_(gpio), displayProvider_(display), styleService_(styleService),
      componentFactory_(componentFactory ? componentFactory : &ComponentFactory::Instance()),
      keySensor_(std::make_shared<KeySensor>(gpio))
{
    // Component will be created during load() method
}

KeyPanel::~KeyPanel()
{
    if (screen_)
    {
        lv_obj_delete(screen_);
    }

    if (keyComponent_)
    {
        keyComponent_.reset();
    }
}

// Core Functionality Methods
/// @brief Initialize the key panel and its components
void KeyPanel::Init()
{
    if (!displayProvider_ || !gpioProvider_)
    {
        log_e("KeyPanel requires display and gpio providers");
        ErrorManager::Instance().ReportCriticalError("KeyPanel",
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

    // Initialize sensor and get current key state
    keySensor_->Init();
    currentKeyState_ = keySensor_->GetKeyState();
}

/// @brief Load the key panel UI components
void KeyPanel::Load(std::function<void()> callbackFunction)
{
    // Loading key panel with current key state display
    callbackFunction_ = callbackFunction;

    // Set BUSY at start of load
    if (panelService_)
    {
        panelService_->SetUiState(UIState::BUSY);
    }

    // Create component using injected factory
    keyComponent_ = componentFactory_->CreateKeyComponent(styleService_);

    // Render the component
    if (!displayProvider_)
    {
        log_e("KeyPanel load requires display provider");
        ErrorManager::Instance().ReportError(ErrorLevel::ERROR, "KeyPanel", "Cannot load - display provider is null");
        return;
    }
    keyComponent_->Render(screen_, centerLocation_, displayProvider_);
    keyComponent_->Refresh(Reading{static_cast<int32_t>(currentKeyState_)});

    lv_obj_add_event_cb(screen_, KeyPanel::ShowPanelCompletionCallback, LV_EVENT_SCREEN_LOADED, this);

    lv_screen_load(screen_);

    // Always apply current theme to the screen when loading (ensures theme is current)
    if (styleService_)
    {
        styleService_->ApplyThemeToScreen(screen_);
    }
}

/// @brief Update the key panel with current sensor data
void KeyPanel::Update(std::function<void()> callbackFunction)
{
    // Set BUSY at start of update
    if (panelService_)
    {
        panelService_->SetUiState(UIState::BUSY);
    }

    if (!gpioProvider_)
    {
        log_e("KeyPanel update requires gpio provider");
        ErrorManager::Instance().ReportError(ErrorLevel::ERROR, "KeyPanel", "Cannot update - gpio provider is null");
        return;
    }

    // Get current key state from sensor
    KeyState newKeyState = keySensor_->GetKeyState();

    // Update display if key state has changed
    if (newKeyState != currentKeyState_)
    {
        log_d("Key state changed from %d to %d - updating display", (int)currentKeyState_, (int)newKeyState);
        currentKeyState_ = newKeyState;
        keyComponent_->Refresh(Reading{static_cast<int32_t>(currentKeyState_)});
    }

    // Set IDLE when update completes
    if (panelService_)
    {
        panelService_->SetUiState(UIState::IDLE);
    }

    callbackFunction();
}

// Static Methods
/// @brief The callback to be run once show panel has completed
/// @param event LVGL event that was used to call this
void KeyPanel::ShowPanelCompletionCallback(lv_event_t *event)
{
    // Key panel load completed - screen displayed

    auto thisInstance = static_cast<KeyPanel *>(lv_event_get_user_data(event));
    
    // Set IDLE when load completes
    if (thisInstance->panelService_)
    {
        thisInstance->panelService_->SetUiState(UIState::IDLE);
    }
    
    thisInstance->callbackFunction_();
}

// Manager injection method
void KeyPanel::SetManagers(IPanelService *panelService, IStyleService *styleService)
{
    panelService_ = panelService;

    // Update styleService if different instance provided
    if (styleService != styleService_)
    {
        styleService_ = styleService;
    }
}