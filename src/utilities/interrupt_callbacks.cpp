#include "utilities/interrupt_callbacks.h"
#include "sensors/key_present_sensor.h"
#include "sensors/key_not_present_sensor.h"
#include "sensors/lock_sensor.h" 
#include "sensors/lights_sensor.h"
#include "sensors/action_button_sensor.h"
#include "managers/panel_manager.h"
#include "managers/style_manager.h"
#include "managers/error_manager.h"
#include "managers/interrupt_manager.h"
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
        } else {
            log_d("Key present state changed - key removed, deactivating interrupt");
            // Explicitly deactivate this interrupt when key is removed
            InterruptManager::Instance().DeactivateInterrupt("key_present");
            return InterruptResult::NO_ACTION;
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
        } else {
            log_d("Key not present state changed - key inserted, deactivating interrupt");
            // Explicitly deactivate this interrupt when key is inserted
            InterruptManager::Instance().DeactivateInterrupt("key_not_present");
            return InterruptResult::NO_ACTION;
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
        } else {
            log_d("Lock state changed - lock disengaged, deactivating interrupt");
            // Explicitly deactivate this interrupt when lock is disengaged
            InterruptManager::Instance().DeactivateInterrupt("lock_state");
            return InterruptResult::NO_ACTION;
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
        // For now, any button state change can trigger short press action
        // The ActionManager will handle timing logic for short vs long press
        auto reading = sensor->GetReading();
        if (std::holds_alternative<int32_t>(reading)) {
            int32_t pressed = std::get<int32_t>(reading);
            if (pressed == 0) { // Button released - trigger action
                log_d("Button released - executing short press effect");
                return InterruptResult::EXECUTE_EFFECT;
            }
        }
    }
    return InterruptResult::NO_ACTION;
}

InterruptResult InterruptCallbacks::LongPressProcess(void* context) {
    ActionButtonSensor* sensor = static_cast<ActionButtonSensor*>(context);
    if (!sensor) {
        log_w("LongPressProcess: Invalid sensor context");
        return InterruptResult::NO_ACTION;
    }
    
    // For now, long press detection is handled by ActionManager
    // This interrupt callback is not used in the current design
    // The ActionManager directly calls panel functions based on timing
    log_v("LongPressProcess called but not implemented - ActionManager handles timing");
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