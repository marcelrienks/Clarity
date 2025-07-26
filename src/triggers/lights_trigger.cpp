#include "triggers/lights_trigger.h"
#include "utilities/types.h"
#include <esp32-hal-log.h>

LightsTrigger::LightsTrigger() : AlertTrigger(
    TRIGGER_LIGHTS_STATE,
    TriggerPriority::NORMAL
) {}

void LightsTrigger::init()
{
    log_d("LightsTrigger initialized with NORMAL priority");
}

TriggerActionRequest LightsTrigger::GetActionRequest()
{
    log_d("Lights on - requesting night theme");
    return TriggerActionRequest{
        .type = TriggerActionType::ToggleTheme,
        .panelName = Themes::NIGHT,  // Use panelName field for theme name
        .triggerId = TRIGGER_LIGHTS_STATE,
        .isTriggerDriven = false
    };
}

TriggerActionRequest LightsTrigger::GetRestoreRequest()
{
    log_d("Lights off - requesting day theme");
    return TriggerActionRequest{
        .type = TriggerActionType::ToggleTheme,
        .panelName = Themes::DAY,  // Use panelName field for theme name
        .triggerId = TRIGGER_LIGHTS_STATE,
        .isTriggerDriven = false
    };
}