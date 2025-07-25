#include "triggers/key_trigger.h"
#include "managers/panel_manager.h"
#include "utilities/types.h"
#include <esp32-hal-log.h>

KeyTrigger::KeyTrigger() : AlertTrigger(
    TRIGGER_KEY_PRESENT,
    TriggerPriority::CRITICAL,
    LoadKeyPanel,
    RestorePreviousPanel
) {}

void KeyTrigger::init()
{
    log_d("KeyTrigger initialized with CRITICAL priority");
}

void KeyTrigger::LoadKeyPanel()
{
    log_d("Key detected - loading key panel");
    PanelManager::GetInstance().CreateAndLoadPanel(PanelNames::KEY, nullptr, true);
}

void KeyTrigger::RestorePreviousPanel()
{
    log_d("Key removed - restoring previous panel");
    auto& pm = PanelManager::GetInstance();
    
    // Check if restoration panel is valid before attempting to load it
    if (pm.restorationPanel == nullptr || strlen(pm.restorationPanel) == 0)
    {
        log_w("No valid restoration panel set - defaulting to OIL panel");
        pm.CreateAndLoadPanel(PanelNames::OIL, nullptr, false);
    }
    else
    {
        log_d("Restoring to panel: %s", pm.restorationPanel);
        pm.CreateAndLoadPanel(pm.restorationPanel, nullptr, false);
    }
}

