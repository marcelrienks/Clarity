#include "panels/key_panel.h"
#include "components/key_component.h"
#include "handlers/trigger_handler.h"
#include "managers/interrupt_manager.h"
#include "managers/error_manager.h"
#include "sensors/gpio_sensor.h"
#include "utilities/logging.h"
#include <Arduino.h>
#include <variant>

// Constructors and Destructors
KeyPanel::KeyPanel(IGpioProvider *gpio, IDisplayProvider *display, IStyleService *styleService)
    : BasePanel(gpio, display, styleService),
      keyComponent_(styleService), componentInitialized_(false)
{
    // Determine current key state by checking which sensor is currently active
    currentKeyState_ = DetermineCurrentKeyState();
}

// BasePanel template method implementations
void KeyPanel::CreateContent()
{
    // Component is now stack-allocated and initialized in constructor
    componentInitialized_ = true;

    // Render the component
    keyComponent_.Render(screen_, centerLocation_, displayProvider_);

    // Set color directly on the concrete component
    keyComponent_.SetColor(currentKeyState_);
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