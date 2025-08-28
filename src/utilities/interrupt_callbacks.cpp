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
    log_v("ShortPressProcess: Callback triggered by InterruptManager 8-step flow");
    // The InterruptManager already determined this is a short press
    // Just signal that the effect should be executed
    return InterruptResult::EXECUTE_EFFECT;
}

InterruptResult InterruptCallbacks::LongPressProcess(void* context) {
    log_v("LongPressProcess: Callback triggered by InterruptManager 8-step flow");
    // The InterruptManager already determined this is a long press
    // Just signal that the effect should be executed
    return InterruptResult::EXECUTE_EFFECT;
}

InterruptResult InterruptCallbacks::ErrorOccurredProcess(void* context) {
    // Context would be ErrorManager or error sensor
    log_v("ErrorOccurredProcess: Checking for error conditions");
    
    static size_t lastErrorCount = 0;
    auto& errorManager = ErrorManager::Instance();
    
    // Get current error count
    size_t currentErrorCount = errorManager.GetErrorQueue().size();
    bool hasErrors = errorManager.HasPendingErrors();
    
    // Only trigger if we have errors AND the count has increased (new errors)
    if (hasErrors && currentErrorCount > lastErrorCount) {
        log_d("New errors detected - count increased from %d to %d", lastErrorCount, currentErrorCount);
        lastErrorCount = currentErrorCount;
        return InterruptResult::EXECUTE_EFFECT;
    }
    
    // Update the count even if we don't trigger (for when errors are cleared)
    lastErrorCount = currentErrorCount;
    
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