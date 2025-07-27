#include "triggers/lock_trigger.h"
#include "utilities/types.h"
#include <esp32-hal-log.h>

LockTrigger::LockTrigger() : AlertTrigger(
    TRIGGER_LOCK_STATE,
    TriggerPriority::IMPORTANT
) {}

void LockTrigger::init()
{
    log_d("LockTrigger initialized with IMPORTANT priority");
}

TriggerActionRequest LockTrigger::GetActionRequest()
{
    log_d("Lock engaged - requesting lock panel load");
    return TriggerActionRequest{
        .type = TriggerActionType::LoadPanel,
        .panelName = PanelNames::LOCK,
        .triggerId = TRIGGER_LOCK_STATE,
        .isTriggerDriven = true
    };
}

TriggerActionRequest LockTrigger::GetRestoreRequest()
{
    log_i("Lock disengaged - no specific restoration action needed");
    // Return default request - trigger manager will handle restoration to config default
    return TriggerActionRequest{
        .type = TriggerActionType::LoadPanel,
        .panelName = "OemOilPanel",  // Default panel for restoration
        .triggerId = TRIGGER_LOCK_STATE,
        .isTriggerDriven = false
    };
}