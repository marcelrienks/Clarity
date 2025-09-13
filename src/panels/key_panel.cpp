#include "panels/key_panel.h"
#include "factories/component_factory.h"
#include "interfaces/i_component_factory.h"
#include "managers/error_manager.h"
#include "managers/style_manager.h"
#include "handlers/trigger_handler.h"
#include "managers/interrupt_manager.h"
#include "sensors/key_present_sensor.h"
#include "sensors/key_not_present_sensor.h"
#include "utilities/constants.h"
#include "utilities/logging.h"
#include <Arduino.h>
#include <variant>

// Constructors and Destructors
KeyPanel::KeyPanel(IGpioProvider *gpio, IDisplayProvider *display, IStyleService *styleService,
                   IComponentFactory* componentFactory)
    : gpioProvider_(gpio), displayProvider_(display), styleService_(styleService),
      componentFactory_(componentFactory ? componentFactory : &ComponentFactory::Instance())
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

    // Note: KeyPanel is display-only, state comes from interrupt system
    // Determine current key state by checking which sensor is currently active
    currentKeyState_ = DetermineCurrentKeyState();
    
    log_i("KeyPanel initialization completed");
}

/// @brief Load the key panel UI components
void KeyPanel::Load()
{
    log_v("Load() called");


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
    
    log_t("KeyPanel loaded successfully");
}

/// @brief Update the key panel with current sensor data
void KeyPanel::Update()
{
    log_v("Update() called");
    
    // Key panel is static - no updates needed, but must reset UI state to IDLE
    if (panelService_) {
        panelService_->SetUiState(UIState::IDLE);
    }
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
    
    // Set UI state to IDLE after static panel loads so triggers can be evaluated again
    if (thisInstance->panelService_)
    {
        thisInstance->panelService_->SetUiState(UIState::IDLE);
    }
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

// IActionService Interface Implementation

static void KeyPanelShortPress(void* panelContext)
{
    log_v("KeyPanelShortPress() called");
    // Display-only panel, no action for short press
}

static void KeyPanelLongPress(void* panelContext)
{
    log_v("KeyPanelLongPress() called");
    
    auto* panel = static_cast<KeyPanel*>(panelContext);
    if (panel)
    {
        // Call public method to handle the long press
        panel->HandleLongPress();
    }
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

void KeyPanel::HandleLongPress()
{
    if (panelService_)
    {
        log_i("KeyPanel long press - loading config panel");
        panelService_->CreateAndLoadPanel(PanelNames::CONFIG, true);
    }
    else
    {
        log_w("KeyPanel: Cannot load config panel - panelService not available");
    }
}

KeyState KeyPanel::DetermineCurrentKeyState()
{
    log_v("DetermineCurrentKeyState() called");
    
    // Access the sensors through InterruptManager to check their current states
    InterruptManager& interruptManager = InterruptManager::Instance();
    auto triggerHandler = interruptManager.GetTriggerHandler();
    
    if (!triggerHandler)
    {
        log_w("DetermineCurrentKeyState: TriggerHandler not available, defaulting to Inactive");
        return KeyState::Inactive;
    }
    
    // Check key present sensor
    auto keyPresentSensor = triggerHandler->GetKeyPresentSensor();
    if (keyPresentSensor && keyPresentSensor->GetKeyPresentState())
    {
        return KeyState::Present;
    }
    
    // Check key not present sensor
    auto keyNotPresentSensor = triggerHandler->GetKeyNotPresentSensor();
    if (keyNotPresentSensor && keyNotPresentSensor->GetKeyNotPresentState())
    {
        return KeyState::NotPresent;
    }
    
    // If neither sensor is active, default to inactive
    return KeyState::Inactive;
}