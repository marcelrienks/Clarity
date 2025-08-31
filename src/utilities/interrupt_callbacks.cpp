#include "utilities/interrupt_callbacks.h"
#include "sensors/key_present_sensor.h"
#include "sensors/key_not_present_sensor.h"
#include "sensors/lock_sensor.h" 
#include "sensors/lights_sensor.h"
#include "sensors/button_sensor.h"
#include "managers/panel_manager.h"
#include "managers/style_manager.h"
#include "managers/error_manager.h"
#include "managers/interrupt_manager.h"
#include "utilities/constants.h"
#include <esp32-hal-log.h>

// External references to global managers (defined in main.cpp)
extern std::unique_ptr<PanelManager> panelManager;
extern std::unique_ptr<StyleManager> styleManager;

// Evaluation functions - check if interrupt condition is met

bool InterruptCallbacks::KeyPresentEvaluate(void* context) {
    KeyPresentSensor* sensor = static_cast<KeyPresentSensor*>(context);
    if (!sensor) return false;
    
    bool changed = sensor->HasStateChanged();
    if (changed && sensor->GetKeyPresentState()) {
        log_d("Key present detected");
        return true;
    }
    return false;
}

bool InterruptCallbacks::KeyNotPresentEvaluate(void* context) {
    KeyNotPresentSensor* sensor = static_cast<KeyNotPresentSensor*>(context);
    if (!sensor) return false;
    
    bool changed = sensor->HasStateChanged();
    if (changed && sensor->GetKeyNotPresentState()) {
        log_d("Key not present detected");
        return true;
    }
    return false;
}

bool InterruptCallbacks::LockStateEvaluate(void* context) {
    LockSensor* sensor = static_cast<LockSensor*>(context);
    if (!sensor) return false;
    
    bool changed = sensor->HasStateChanged();
    if (changed) {
        bool lockEngaged = std::get<bool>(sensor->GetReading());
        if (lockEngaged) {
            log_d("Lock engaged detected");
            return true;
        }
    }
    return false;
}

bool InterruptCallbacks::LightsStateEvaluate(void* context) {
    LightsSensor* sensor = static_cast<LightsSensor*>(context);
    if (!sensor) return false;
    
    bool changed = sensor->HasStateChanged();
    if (changed) {
        log_d("Lights state changed");
        return true;
    }
    return false;
}

bool InterruptCallbacks::ShortPressEvaluate(void* context) {
    ButtonSensor* sensor = static_cast<ButtonSensor*>(context);
    if (!sensor) return false;
    
    ButtonAction action = sensor->GetButtonAction();
    if (action == ButtonAction::SHORT_PRESS) {
        log_d("Short press detected");
        return true;
    }
    return false;
}

bool InterruptCallbacks::LongPressEvaluate(void* context) {
    ButtonSensor* sensor = static_cast<ButtonSensor*>(context);
    if (!sensor) return false;
    
    ButtonAction action = sensor->GetButtonAction();
    if (action == ButtonAction::LONG_PRESS) {
        log_d("Long press detected");
        return true;
    }
    return false;
}

bool InterruptCallbacks::ErrorOccurredEvaluate(void* context) {
    ErrorManager* errorManager = static_cast<ErrorManager*>(context);
    if (!errorManager) return false;
    
    return errorManager->HasPendingErrors();
}

// Execution functions - perform the interrupt action

void InterruptCallbacks::KeyPresentExecute(void* context) {
    if (panelManager) {
        panelManager->CreateAndLoadPanel(PanelNames::KEY, true);
        log_i("Loaded KEY panel for key present");
    }
}

void InterruptCallbacks::KeyNotPresentExecute(void* context) {
    if (panelManager) {
        panelManager->CreateAndLoadPanel(PanelNames::KEY, true);
        log_i("Loaded KEY panel for key not present");
    }
}

void InterruptCallbacks::LockStateExecute(void* context) {
    if (panelManager) {
        panelManager->CreateAndLoadPanel(PanelNames::LOCK, true);
        log_i("Loaded LOCK panel for lock state");
    }
}

void InterruptCallbacks::LightsStateExecute(void* context) {
    LightsSensor* sensor = static_cast<LightsSensor*>(context);
    if (!sensor || !styleManager) return;
    
    bool lightsOn = std::get<bool>(sensor->GetReading());
    const char* theme = lightsOn ? Themes::NIGHT : Themes::DAY;
    styleManager->SetTheme(theme);
    log_i("Set theme to %s based on lights state", theme);
}

void InterruptCallbacks::ShortPressExecute(void* context) {
    // Button action execution is handled by InterruptManager's button system
    log_d("Short press action executed");
}

void InterruptCallbacks::LongPressExecute(void* context) {
    // Button action execution is handled by InterruptManager's button system
    log_d("Long press action executed");
}

void InterruptCallbacks::ErrorOccurredExecute(void* context) {
    if (panelManager) {
        panelManager->CreateAndLoadPanel(PanelNames::ERROR, true);
        log_i("Loaded ERROR panel for error condition");
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
    return panelManager ? const_cast<char*>(panelManager->GetCurrentPanel()) : nullptr;
}