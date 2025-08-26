#include "utilities/interrupt_callbacks.h"
#include "sensors/key_present_sensor.h"
#include "sensors/key_not_present_sensor.h"
#include "sensors/lock_sensor.h" 
#include "sensors/lights_sensor.h"
#include "sensors/action_button_sensor.h"
#include "managers/panel_manager.h"
#include "managers/style_manager.h"
#include "managers/error_manager.h"
#include "utilities/constants.h"  // For PanelNames
#include <esp32-hal-log.h>

// External references to global managers (defined in main.cpp)
extern std::unique_ptr<PanelManager> panelManager;
extern std::unique_ptr<StyleManager> styleManager;

// Memory-optimized single-function callbacks
InterruptResult InterruptCallbacks::KeyPresentProcess(void* context) {
    KeyPresentSensor* sensor = static_cast<KeyPresentSensor*>(context);
    if (!sensor) {
        log_w("KeyPresentProcess: Invalid sensor context");
        return InterruptResult::NO_ACTION;
    }
    
    bool changed = sensor->HasStateChanged();
    if (changed) {
        bool keyPresent = sensor->GetKeyPresentState();
        if (keyPresent) {
            log_d("Key present state changed - key detected");
            return InterruptResult::EXECUTE_EFFECT;
        }
    }
    return InterruptResult::NO_ACTION;
}

InterruptResult InterruptCallbacks::KeyNotPresentProcess(void* context) {
    KeyNotPresentSensor* sensor = static_cast<KeyNotPresentSensor*>(context);
    if (!sensor) {
        log_w("KeyNotPresentProcess: Invalid sensor context");
        return InterruptResult::NO_ACTION;
    }
    
    bool changed = sensor->HasStateChanged();
    if (changed) {
        bool keyNotPresent = sensor->GetKeyNotPresentState();
        if (keyNotPresent) {
            log_d("Key not present state changed - key removed");
            return InterruptResult::EXECUTE_EFFECT;
        }
    }
    return InterruptResult::NO_ACTION;
}

InterruptResult InterruptCallbacks::LockStateProcess(void* context) {
    LockSensor* sensor = static_cast<LockSensor*>(context);
    if (!sensor) {
        log_w("LockStateProcess: Invalid sensor context");
        return InterruptResult::NO_ACTION;
    }
    
    bool changed = sensor->HasStateChanged();
    if (changed) {
        bool lockEngaged = std::get<bool>(sensor->GetReading());
        if (lockEngaged) {
            log_d("Lock state changed - lock engaged");
            return InterruptResult::EXECUTE_EFFECT;
        }
    }
    return InterruptResult::NO_ACTION;
}

InterruptResult InterruptCallbacks::LightsStateProcess(void* context) {
    LightsSensor* sensor = static_cast<LightsSensor*>(context);
    if (!sensor) {
        log_w("LightsStateProcess: Invalid sensor context");
        return InterruptResult::NO_ACTION;
    }
    
    bool changed = sensor->HasStateChanged();
    if (changed) {
        log_d("Lights state changed - theme change needed");
        return InterruptResult::EXECUTE_EFFECT;
    }
    return InterruptResult::NO_ACTION;
}

InterruptResult InterruptCallbacks::ShortPressProcess(void* context) {
    ActionButtonSensor* sensor = static_cast<ActionButtonSensor*>(context);
    if (!sensor) {
        log_w("ShortPressProcess: Invalid sensor context");
        return InterruptResult::NO_ACTION;
    }
    
    bool hasEvent = sensor->HasStateChanged();
    if (hasEvent) {
        log_d("Button short press event detected");
        return InterruptResult::EXECUTE_EFFECT;
    }
    return InterruptResult::NO_ACTION;
}

InterruptResult InterruptCallbacks::LongPressProcess(void* context) {
    ActionButtonSensor* sensor = static_cast<ActionButtonSensor*>(context);
    if (!sensor) {
        log_w("LongPressProcess: Invalid sensor context");
        return InterruptResult::NO_ACTION;
    }
    
    // For now, long press is not implemented - always return NO_ACTION
    return InterruptResult::NO_ACTION;
}

InterruptResult InterruptCallbacks::ErrorOccurredProcess(void* context) {
    // Context would be ErrorManager or error sensor
    log_v("ErrorOccurredProcess: Checking for error conditions");
    
    // Check ErrorManager for new errors
    bool hasNewErrors = ErrorManager::Instance().HasPendingErrors();
    if (hasNewErrors) {
        log_d("New errors detected");
        return InterruptResult::EXECUTE_EFFECT;
    }
    return InterruptResult::NO_ACTION;
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