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
    log_v("KeyPanel constructor called");
    // Component will be created during load() method
}

KeyPanel::~KeyPanel()
{
    log_v("~KeyPanel() destructor called");
    
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
    log_v("Init() called");
    
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
    
    log_i("KeyPanel initialization completed");
}

/// @brief Load the key panel UI components
void KeyPanel::Load(std::function<void()> callbackFunction)
{
    log_v("Load() called");
    
    callbackFunction_ = callbackFunction;


    if (!componentFactory_)
    {
        log_e("KeyPanel requires component factory");
        ErrorManager::Instance().ReportError(ErrorLevel::ERROR, "KeyPanel", "ComponentFactory is null");
        return;
    }

    // Create component using injected factory
    keyComponent_ = componentFactory_->CreateKeyComponent(styleService_);
    if (!keyComponent_)
    {
        log_e("Failed to create key component");
        ErrorManager::Instance().ReportError(ErrorLevel::ERROR, "KeyPanel", "Component creation failed");
        return;
    }

    // Render the component
    if (!displayProvider_)
    {
        log_e("KeyPanel load requires display provider");
        ErrorManager::Instance().ReportError(ErrorLevel::ERROR, "KeyPanel", "Cannot load - display provider is null");
        return;
    }
    keyComponent_->Render(screen_, centerLocation_, displayProvider_);
    
    // Cast to KeyComponent to access SetColor method
    KeyComponent* keyComp = static_cast<KeyComponent*>(keyComponent_.get());
    if (keyComp)
    {
        keyComp->SetColor(currentKeyState_);
    }

    lv_obj_add_event_cb(screen_, KeyPanel::ShowPanelCompletionCallback, LV_EVENT_SCREEN_LOADED, this);

    lv_screen_load(screen_);

    // Always apply current theme to the screen when loading (ensures theme is current)
    if (styleService_)
    {
        styleService_->ApplyThemeToScreen(screen_);
    }
    
    log_i("KeyPanel loaded successfully");
}

/// @brief Update the key panel with current sensor data
void KeyPanel::Update(std::function<void()> callbackFunction)
{
    log_v("Update() called");
    
    // Key panel is static - no updates needed
    callbackFunction();
}

// Static Methods
/// @brief The callback to be run once show panel has completed
/// @param event LVGL event that was used to call this
void KeyPanel::ShowPanelCompletionCallback(lv_event_t *event)
{
    log_v("ShowPanelCompletionCallback() called");
    
    if (!event)
        return;

    auto thisInstance = static_cast<KeyPanel *>(lv_event_get_user_data(event));
    if (!thisInstance)
        return;
    
    
    thisInstance->callbackFunction_();
}

// Manager injection method
void KeyPanel::SetManagers(IPanelService *panelService, IStyleService *styleService)
{
    log_v("SetManagers() called");
    
    panelService_ = panelService;

    // Update styleService if different instance provided
    if (styleService != styleService_)
    {
        styleService_ = styleService;
    }
}

// New IActionService Interface Implementation (Phase 1 compatibility stubs)

static void KeyPanelShortPress(void* panelContext)
{
    log_v("KeyPanelShortPress() called");
    // Phase 1: Display-only panel, no action for short press
}

static void KeyPanelLongPress(void* panelContext)
{
    log_v("KeyPanelLongPress() called");
    // Phase 1: Simple stub - would load config panel in full implementation
}

void (*KeyPanel::GetShortPressFunction())(void* panelContext)
{
    return KeyPanelShortPress;
}

void (*KeyPanel::GetLongPressFunction())(void* panelContext)
{
    return KeyPanelLongPress;
}

void* KeyPanel::GetPanelContext()
{
    return this;
}