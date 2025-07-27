#include "triggers/key_trigger.h"
#include "utilities/types.h"
#include <esp32-hal-log.h>

// KeyTrigger - handles key present events
KeyTrigger::KeyTrigger() : AlertTrigger(
    TRIGGER_KEY_PRESENT,
    TriggerPriority::CRITICAL
) {}

void KeyTrigger::init()
{
    log_d("KeyTrigger initialized with CRITICAL priority");
}

TriggerActionRequest KeyTrigger::GetActionRequest()
{
    log_d("Key detected - requesting key panel load (present=true)");
    return TriggerActionRequest{
        .type = TriggerActionType::LoadPanel,
        .panelName = PanelNames::KEY,
        .triggerId = TRIGGER_KEY_PRESENT,
        .isTriggerDriven = true
    };
}

TriggerActionRequest KeyTrigger::GetRestoreRequest()
{
    log_d("Key removed - requesting panel restoration");
    return TriggerActionRequest{
        .type = TriggerActionType::RestorePanel,
        .panelName = nullptr,  // Main will determine restoration panel
        .triggerId = TRIGGER_KEY_PRESENT,
        .isTriggerDriven = false
    };
}

// KeyNotPresentTrigger - handles key not present events  
KeyNotPresentTrigger::KeyNotPresentTrigger() : AlertTrigger(
    TRIGGER_KEY_NOT_PRESENT,
    TriggerPriority::CRITICAL
) {}

void KeyNotPresentTrigger::init()
{
    log_d("KeyNotPresentTrigger initialized with CRITICAL priority");
}

TriggerActionRequest KeyNotPresentTrigger::GetActionRequest()
{
    log_d("Key not detected - requesting key panel load (present=false)");
    return TriggerActionRequest{
        .type = TriggerActionType::LoadPanel,
        .panelName = PanelNames::KEY,
        .triggerId = TRIGGER_KEY_NOT_PRESENT,
        .isTriggerDriven = true
    };
}

TriggerActionRequest KeyNotPresentTrigger::GetRestoreRequest()
{
    log_d("Key not present trigger restore - requesting panel restoration");
    return TriggerActionRequest{
        .type = TriggerActionType::RestorePanel,
        .panelName = nullptr,  // Main will determine restoration panel
        .triggerId = TRIGGER_KEY_NOT_PRESENT,
        .isTriggerDriven = false
    };
}

