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
    log_i("*** LOCK TRIGGER RESTORE CALLED - Lock disengaged, requesting panel restoration ***");
    return TriggerActionRequest{
        .type = TriggerActionType::RestorePanel,
        .panelName = nullptr,  // Main will determine restoration panel
        .triggerId = TRIGGER_LOCK_STATE,
        .isTriggerDriven = false
    };
}