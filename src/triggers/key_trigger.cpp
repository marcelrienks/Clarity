#include "triggers/key_trigger.h"
#include "utilities/types.h"
#include <esp32-hal-log.h>

// Constructors and Destructors

/// @brief Constructor with optional restoration mode
/// @param enable_restoration Whether to restore previous panel when key becomes inactive
KeyTrigger::KeyTrigger(bool enable_restoration)
    : enableRestoration_(enable_restoration),
      lastKeyState_(KeyState::Inactive)
{
}

// Core Functionality Methods


/// @brief Evaluate the trigger condition based on key sensor reading
/// @return true if key state has changed and trigger should activate
bool KeyTrigger::evaluate()
{
    log_d("...");

    // Get current key state from sensor
    Reading sensorReading = keySensor_.GetReading();
    KeyState currentKeyState = static_cast<KeyState>(std::get<int32_t>(sensorReading));

    // Simple logic: trigger if either pin 25 OR pin 26 is HIGH (but not both)
    bool shouldTrigger = false;

    if (currentKeyState == KeyState::Present || currentKeyState == KeyState::NotPresent)
    {
        // Pin 25 HIGH (Present) OR Pin 26 HIGH (NotPresent) - always trigger
        shouldTrigger = true;
        log_d("Key state: %s, triggering",
              currentKeyState == KeyState::Present ? "Present" : "NotPresent");
    }
    else
    {
        // Both pins LOW (Inactive) - no trigger
        shouldTrigger = false;
        log_d("Key state: Inactive, no trigger");
    }

    // Update last state
    lastKeyState_ = currentKeyState;

    log_d("currentKeyState: %d, shouldTrigger: %d", static_cast<int>(currentKeyState), shouldTrigger);

    return shouldTrigger;
}

/// @brief Get the trigger identifier
/// @return Unique trigger identifier string
const char *KeyTrigger::GetId() const
{
    return TRIGGER_ID;
}

/// @brief Get the target panel name to switch to when triggered
/// @return Panel name based on current key state
const char* KeyTrigger::GetTargetPanel() const
{
    // Always return KeyPanel for active states, restoration handled by InterruptManager
    return PanelNames::KEY;
}

/// @brief Initialize the trigger and key sensor
void KeyTrigger::init()
{
    log_d("...");

    // Initialize the key sensor
    keySensor_.init();

    // Initialize last state to Inactive to ensure trigger fires on first active state
    lastKeyState_ = KeyState::Inactive;
    Reading currentReading = keySensor_.GetReading();
    KeyState currentState = static_cast<KeyState>(std::get<int32_t>(currentReading));
    log_d("Initial key state: %d (forcing last state to Inactive to enable triggering)", static_cast<int>(currentState));
}

/// @brief Whether to restore the previous panel when key becomes inactive
/// @return true if previous panel should be restored when key becomes inactive
bool KeyTrigger::ShouldRestore() const
{
    return enableRestoration_;
}

