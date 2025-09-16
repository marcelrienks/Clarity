#include "panels/key_panel.h"
#include "components/key_component.h"
#include "handlers/trigger_handler.h"
#include "managers/interrupt_manager.h"
#include "managers/error_manager.h"
#include "sensors/gpio_sensor.h"
#include "utilities/logging.h"
#include <Arduino.h>
#include <variant>

/**
 * @brief Constructs key panel with required service dependencies
 * @param gpio GPIO provider for hardware sensor access
 * @param display Display provider for screen management
 * @param styleService Style service for theme-based icon coloring
 *
 * Creates key panel that displays vehicle ignition key status. Determines
 * current key state during construction and initializes stack-allocated
 * key component for efficient memory usage.
 */
KeyPanel::KeyPanel(IGpioProvider *gpio, IDisplayProvider *display, IStyleService *styleService)
    : BasePanel(gpio, display, styleService),
      keyComponent_(styleService), componentInitialized_(false)
{
    // Determine current key state by checking which sensor is currently active
    currentKeyState_ = DetermineCurrentKeyState();
}

/**
 * @brief Creates key panel UI content with current key state display
 *
 * Renders key component on screen and applies appropriate color based on
 * current key state. Green indicates key present, red indicates key absent.
 * Essential for immediate visual feedback of vehicle ignition status.
 */
void KeyPanel::CreateContent()
{
    // Component is now stack-allocated and initialized in constructor
    componentInitialized_ = true;

    // Render the component
    keyComponent_.Render(screen_, centerLocation_, displayProvider_);

    // Set color directly on the concrete component
    keyComponent_.SetColor(currentKeyState_);
}

/**
 * @brief Updates key panel content (static panel - no updates needed)
 *
 * Key panel displays static state determined at construction time.
 * State changes trigger panel transitions through the interrupt system
 * rather than content updates within the same panel instance.
 */
void KeyPanel::UpdateContent()
{
    // Key panel is static - no updates needed
    // State changes are handled by the interrupt system loading different panels
}



// Panel-specific button handling
/**
 * @brief Handles long button press events for panel navigation
 *
 * Processes long button press events by navigating to the configuration panel.
 * This provides users with access to system settings from the key status display.
 * Includes error handling for cases where the panel service is not available.
 */
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

/**
 * @brief Determines the current key state by reading sensor inputs
 * @return Current key state (Present, NotPresent, or Inactive)
 *
 * Queries the key presence sensors through the trigger handler to determine
 * the current physical state of the ignition key. Checks both key present
 * and key not present sensors to provide accurate state detection. Returns
 * Inactive if neither sensor is active or if the trigger handler is unavailable.
 */
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