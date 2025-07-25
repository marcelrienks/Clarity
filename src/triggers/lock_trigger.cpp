#include "triggers/lock_trigger.h"
#include "managers/panel_manager.h"
#include "utilities/types.h"
#include <esp32-hal-log.h>

LockTrigger::LockTrigger() : AlertTrigger(
    TRIGGER_LOCK_STATE,
    TriggerPriority::IMPORTANT,
    LoadLockPanel,
    RestorePreviousPanel
) {}

void LockTrigger::init()
{
    log_d("LockTrigger initialized with IMPORTANT priority");
}

void LockTrigger::LoadLockPanel()
{
    log_d("Lock engaged - loading lock panel");
    PanelManager::GetInstance().CreateAndLoadPanel(PanelNames::LOCK, nullptr, true);
}

void LockTrigger::RestorePreviousPanel()
{
    log_d("Lock disengaged - restoring previous panel");
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