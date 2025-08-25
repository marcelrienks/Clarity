#include "utilities/interrupt_callbacks.h"
#include "sensors/key_present_sensor.h"
#include "sensors/key_not_present_sensor.h"
#include "sensors/lock_sensor.h" 
#include "sensors/lights_sensor.h"
#include "sensors/action_button_sensor.h"
#include "managers/panel_manager.h"
#include "managers/style_manager.h"
#include "managers/error_manager.h"
#include <esp32-hal-log.h>

// External references to global managers (defined in main.cpp)
extern std::unique_ptr<PanelManager> panelManager;
extern std::unique_ptr<StyleManager> styleManager;

// Key Present Sensor Callbacks
bool InterruptCallbacks::KeyPresentChanged(void* context) {
    KeyPresentSensor* sensor = static_cast<KeyPresentSensor*>(context);
    if (!sensor) {
        log_w("KeyPresentChanged: Invalid sensor context");
        return false;
    }
    
    bool changed = sensor->HasStateChanged();
    if (changed) {
        log_d("Key present state changed");
    }
    return changed;
}

void InterruptCallbacks::LoadKeyPanel(void* context) {
    KeyPresentSensor* sensor = static_cast<KeyPresentSensor*>(context);
    if (!sensor) {
        log_e("LoadKeyPanel: Invalid sensor context");
        return;
    }
    
    bool keyPresent = sensor->GetKeyPresentState();
    if (keyPresent && panelManager) {
        log_i("Key detected - loading Key panel");
        panelManager->CreateAndLoadPanel("KEY");
    }
}

// Key Not Present Sensor Callbacks
bool InterruptCallbacks::KeyNotPresentChanged(void* context) {
    KeyNotPresentSensor* sensor = static_cast<KeyNotPresentSensor*>(context);
    if (!sensor) {
        log_w("KeyNotPresentChanged: Invalid sensor context");
        return false;
    }
    
    bool changed = sensor->HasStateChanged();
    if (changed) {
        log_d("Key not present state changed");
    }
    return changed;
}

void InterruptCallbacks::RestoreFromKeyPanel(void* context) {
    KeyNotPresentSensor* sensor = static_cast<KeyNotPresentSensor*>(context);
    if (!sensor) {
        log_e("RestoreFromKeyPanel: Invalid sensor context");
        return;
    }
    
    bool keyNotPresent = sensor->GetKeyNotPresentState();
    if (keyNotPresent && panelManager) {
        log_i("Key removed - restoring previous panel");
        panelManager->CreateAndLoadPanel("OEM_OIL");
    }
}

// Lock Sensor Callbacks
bool InterruptCallbacks::LockStateChanged(void* context) {
    LockSensor* sensor = static_cast<LockSensor*>(context);
    if (!sensor) {
        log_w("LockStateChanged: Invalid sensor context");
        return false;
    }
    
    bool changed = sensor->HasStateChanged();
    if (changed) {
        log_d("Lock state changed");
    }
    return changed;
}

void InterruptCallbacks::LoadLockPanel(void* context) {
    LockSensor* sensor = static_cast<LockSensor*>(context);
    if (!sensor) {
        log_e("LoadLockPanel: Invalid sensor context");
        return;
    }
    
    bool lockEngaged = std::get<bool>(sensor->GetReading());
    if (lockEngaged && panelManager) {
        log_i("Lock engaged - loading Lock panel");
        panelManager->CreateAndLoadPanel("LOCK");
    }
}

// Lights Sensor Callbacks (Theme switching)
bool InterruptCallbacks::LightsStateChanged(void* context) {
    LightsSensor* sensor = static_cast<LightsSensor*>(context);
    if (!sensor) {
        log_w("LightsStateChanged: Invalid sensor context");
        return false;
    }
    
    bool changed = sensor->HasStateChanged();
    if (changed) {
        log_d("Lights state changed");
    }
    return changed;
}

void InterruptCallbacks::SetThemeBasedOnLights(void* context) {
    LightsSensor* sensor = static_cast<LightsSensor*>(context);
    if (!sensor) {
        log_e("SetThemeBasedOnLights: Invalid sensor context");
        return;
    }
    
    bool lightsOn = sensor->GetLightsState();
    if (styleManager) {
        const char* newTheme = lightsOn ? "NIGHT" : "DAY";
        log_i("Lights %s - switching to %s theme", lightsOn ? "ON" : "OFF", newTheme);
        styleManager->SetTheme(newTheme);
    }
}

// Button Input Callbacks (Universal button system)
bool InterruptCallbacks::HasShortPressEvent(void* context) {
    ActionButtonSensor* sensor = static_cast<ActionButtonSensor*>(context);
    if (!sensor) {
        log_w("HasShortPressEvent: Invalid sensor context");
        return false;
    }
    
    // Check if sensor has detected a short press event
    bool hasEvent = sensor->HasStateChanged();
    if (hasEvent) {
        log_d("Button short press event detected");
    }
    return hasEvent;
}

void InterruptCallbacks::ExecutePanelShortPress(void* context) {
    ActionButtonSensor* sensor = static_cast<ActionButtonSensor*>(context);
    if (!sensor || !panelManager) {
        log_e("ExecutePanelShortPress: Invalid context");
        return;
    }
    
    log_i("Executing panel short press function");
    log_d("Panel short press function executed");
}

bool InterruptCallbacks::HasLongPressEvent(void* context) {
    ActionButtonSensor* sensor = static_cast<ActionButtonSensor*>(context);
    if (!sensor) {
        log_w("HasLongPressEvent: Invalid sensor context");
        return false;
    }
    
    return false;
}

void InterruptCallbacks::ExecutePanelLongPress(void* context) {
    ActionButtonSensor* sensor = static_cast<ActionButtonSensor*>(context);
    if (!sensor || !panelManager) {
        log_e("ExecutePanelLongPress: Invalid context");
        return;
    }
    
    log_i("Executing panel long press function");
    log_d("Panel long press function executed");
}

// Error System Callbacks
bool InterruptCallbacks::ErrorOccurred(void* context) {
    // Context would be ErrorManager or error sensor
    log_d("Checking for error conditions");
    
    // Check ErrorManager for new errors
    bool hasNewErrors = ErrorManager::Instance().HasPendingErrors();
    if (hasNewErrors) {
        log_d("New errors detected");
    }
    return hasNewErrors;
}

void InterruptCallbacks::LoadErrorPanel(void* context) {
    if (panelManager) {
        log_i("Loading Error panel due to error conditions");
        panelManager->CreateAndLoadPanel("ERROR");
    }
}

// Helper functions
void* InterruptCallbacks::GetPanelManager() {
    return panelManager.get();
}

void* InterruptCallbacks::GetStyleManager() {
    return styleManager.get();
}

void* InterruptCallbacks::GetCurrentPanel() {
    if (panelManager) {
        return nullptr;
    }
    return nullptr;
}