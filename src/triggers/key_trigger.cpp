#include "triggers/key_trigger.h"
#include "utilities/types.h"
#include <esp32-hal-log.h>

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
    log_d("Key detected - requesting key panel load");
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

