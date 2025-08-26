#include "sensors/base_sensor.h"
#include "managers/interrupt_manager.h"
#include "managers/error_manager.h"
#include <Arduino.h>
#include <cstring>

#include "esp32-hal-log.h"

bool BaseSensor::RegisterPolledInterrupt(const char* id, Priority priority, InterruptEffect effect, 
                                       unsigned long intervalMs, void* data)
{
    log_v("RegisterPolledInterrupt() called for: %s", id ? id : "null");
    
    if (!id)
    {
        log_e("Cannot register interrupt with null ID");
        return false;
    }
    
    // Create interrupt structure
    Interrupt interrupt = {};
    interrupt.id = id;
    interrupt.priority = priority;
    interrupt.source = InterruptSource::POLLED;
    interrupt.effect = effect;
    interrupt.processFunc = ProcessSensorInterrupt;
    interrupt.context = this;
    interrupt.active = true;
    interrupt.lastEvaluation = 0;
    
    // Set effect-specific data
    if (data)
    {
        switch (effect)
        {
            case InterruptEffect::LOAD_PANEL:
                interrupt.data.panel.panelName = static_cast<const char*>(data);
                interrupt.data.panel.trackForRestore = false;
                break;
            case InterruptEffect::SET_THEME:
                interrupt.data.theme.theme = static_cast<const char*>(data);
                break;
            case InterruptEffect::SET_PREFERENCE:
                // Would need to handle preference data structure
                break;
            case InterruptEffect::BUTTON_ACTION:
                // For custom functions, data should be properly cast by caller
                if (data)
                {
                    interrupt.data.custom.customFunc = reinterpret_cast<void(*)(void*)>(data);
                }
                break;
        }
    }
    
    bool registered = InterruptManager::Instance().RegisterInterrupt(interrupt);
    if (registered)
    {
        log_d("Registered polled interrupt '%s' for sensor with %lu ms interval", id, intervalMs);
    }
    else
    {
        log_e("Failed to register polled interrupt '%s'", id);
    }
    
    return registered;
}

bool BaseSensor::RegisterQueuedInterrupt(const char* id, Priority priority, InterruptEffect effect, 
                                        void* data)
{
    log_v("RegisterQueuedInterrupt() called for: %s", id ? id : "null");
    
    if (!id)
    {
        log_e("Cannot register interrupt with null ID");
        return false;
    }
    
    // Create interrupt structure
    Interrupt interrupt = {};
    interrupt.id = id;
    interrupt.priority = priority;
    interrupt.source = InterruptSource::QUEUED;
    interrupt.effect = effect;
    interrupt.processFunc = ProcessSensorInterrupt;
    interrupt.context = this;
    interrupt.active = true;
    interrupt.lastEvaluation = 0;
    
    // Set effect-specific data
    if (data)
    {
        switch (effect)
        {
            case InterruptEffect::LOAD_PANEL:
                interrupt.data.panel.panelName = static_cast<const char*>(data);
                interrupt.data.panel.trackForRestore = true; // Queued interrupts should track for restore
                break;
            case InterruptEffect::SET_THEME:
                interrupt.data.theme.theme = static_cast<const char*>(data);
                break;
            case InterruptEffect::SET_PREFERENCE:
                // Would need to handle preference data structure
                break;
            case InterruptEffect::BUTTON_ACTION:
                // For custom functions, data should be properly cast by caller
                if (data)
                {
                    interrupt.data.custom.customFunc = reinterpret_cast<void(*)(void*)>(data);
                }
                break;
        }
    }
    
    bool registered = InterruptManager::Instance().RegisterInterrupt(interrupt);
    if (registered)
    {
        log_d("Registered queued interrupt '%s' for sensor", id);
    }
    else
    {
        log_e("Failed to register queued interrupt '%s'", id);
    }
    
    return registered;
}

void BaseSensor::UnregisterInterrupt(const char* id)
{
    log_v("UnregisterInterrupt() called for: %s", id ? id : "null");
    
    if (!id)
    {
        log_w("Cannot unregister interrupt with null ID");
        return;
    }
    
    InterruptManager::Instance().UnregisterInterrupt(id);
    log_d("Unregistered interrupt '%s' for sensor", id);
}

// Memory-optimized single callback function
InterruptResult BaseSensor::ProcessSensorInterrupt(void* context)
{
    log_v("ProcessSensorInterrupt() called");
    
    if (!context)
    {
        log_e("Null context in sensor interrupt processing");
        return InterruptResult::NO_ACTION;
    }
    
    auto* sensor = static_cast<BaseSensor*>(context);
    if (sensor->HasStateChanged())
    {
        log_d("Sensor state changed - triggering interrupt action");
        sensor->OnInterruptTriggered();
        return InterruptResult::EXECUTE_EFFECT;
    }
    
    return InterruptResult::NO_ACTION;
}