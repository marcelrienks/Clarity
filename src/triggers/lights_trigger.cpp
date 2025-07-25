#include "triggers/lights_trigger.h"
#include "managers/style_manager.h"
#include "utilities/types.h"
#include <esp32-hal-log.h>

LightsTrigger::LightsTrigger() : AlertTrigger(
    TRIGGER_LIGHTS_STATE,
    TriggerPriority::NORMAL,
    SetNightTheme,
    SetDayTheme
) {}

void LightsTrigger::init()
{
    log_d("LightsTrigger initialized with NORMAL priority");
}

void LightsTrigger::SetNightTheme()
{
    log_d("Lights on - switching to night theme");
    StyleManager::GetInstance().set_theme(Themes::NIGHT);
}

void LightsTrigger::SetDayTheme()
{
    log_d("Lights off - switching to day theme");
    StyleManager::GetInstance().set_theme(Themes::DAY);
}