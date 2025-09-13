#include "panels/key_panel.h"
#include "interfaces/i_component_factory.h"
#include "handlers/trigger_handler.h"
#include "managers/interrupt_manager.h"
#include "managers/error_manager.h"
#include "sensors/gpio_sensor.h"
#include "utilities/logging.h"
#include <Arduino.h>
#include <variant>

// Constructors and Destructors
KeyPanel::KeyPanel(IGpioProvider *gpio, IDisplayProvider *display, IStyleService *styleService,
                   IComponentFactory* componentFactory)
    : BasePanel(gpio, display, styleService, componentFactory)
{
    // Determine current key state by checking which sensor is currently active
    currentKeyState_ = DetermineCurrentKeyState();
}

// BasePanel template method implementations
void KeyPanel::CreateContent()
{
    // Create component using injected factory
    keyComponent_ = componentFactory_->CreateKeyComponent(styleService_);
    if (!keyComponent_)
    {
        log_e("Failed to create key component");
        ErrorManager::Instance().ReportError(ErrorLevel::ERROR, "KeyPanel", "Component creation failed");
        return;
    }

    // Render the component
    keyComponent_->Render(screen_, centerLocation_, displayProvider_);

    // Cast to KeyComponent to access SetColor method
    KeyComponent* keyComp = static_cast<KeyComponent*>(keyComponent_.get());
    if (keyComp)
    {
        keyComp->SetColor(currentKeyState_);
    }
}

void KeyPanel::UpdateContent()
{
    // Key panel is static - no updates needed
    // State changes are handled by the interrupt system loading different panels
}



// Panel-specific button handling
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
    
    // Check key present sensor (now using generic GetState method)
    auto keyPresentSensor = triggerHandler->GetKeyPresentSensor();
    if (keyPresentSensor && keyPresentSensor->GetState())
    {
        return KeyState::Present;
    }

    // Check key not present sensor (now using generic GetState method)
    auto keyNotPresentSensor = triggerHandler->GetKeyNotPresentSensor();
    if (keyNotPresentSensor && keyNotPresentSensor->GetState())
    {
        return KeyState::NotPresent;
    }
    
    // If neither sensor is active, default to inactive
    return KeyState::Inactive;
}