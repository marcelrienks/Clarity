#include "main.h"

void setup()
{
  log_d("...");

  PreferenceManager &preference_manager = PreferenceManager::GetInstance();
  preference_manager.init();

  Device::GetInstance().prepare();
  Ticker::handle_lv_tasks();

  StyleManager::GetInstance().init(Themes::DAY);
  Ticker::handle_lv_tasks();

  TriggerManager &trigger_manager = TriggerManager::GetInstance();
  trigger_manager.init();
  trigger_manager.RegisterAllTriggers();

  PanelManager &panel_manager = PanelManager::GetInstance();
  panel_manager.init();
  panel_manager.RegisterAllPanels();

  panel_manager.CreateAndLoadPanelWithSplash(preference_manager.config.panelName.c_str());
  
  Ticker::handle_lv_tasks();
}

void loop()
{
  log_d("...");

  // Core 0 responsibilities: consolidate trigger states and execute smart plan
  auto consolidatedStates = TriggerManager::GetInstance().ProcessPendingTriggerEvents();
  
  // Plan and execute only if there are state changes
  if (!consolidatedStates.empty()) {
    auto plan = TriggerManager::GetInstance().PlanExecutionFromStates(consolidatedStates);
    
    // 1. Execute theme actions first (affects subsequent panel styling)
    for (const auto& request : plan.themeActions) {
      log_i("Executing theme action: %s", request.panelName);
      StyleManager::GetInstance().set_theme(request.panelName);
    }
    
    // 2. Execute non-panel actions second
    for (const auto& request : plan.nonPanelActions) {
      log_i("Executing non-panel action: type %d", (int)request.type);
      // Handle other action types as needed in future
    }
    
    // 3. Execute final panel action last (single panel load only)
    if (plan.finalPanelAction.type != TriggerActionType::None) {
      ExecutePanelAction(plan.finalPanelAction);
    } else if (!plan.themeActions.empty()) {
      // If only theme actions occurred without panel action, refresh current panel
      log_i("Theme actions without panel action - refreshing current panel");
      PanelManager::GetInstance().UpdatePanel();
      return; // Skip the UpdatePanel call at the end
    }
  }
  
  PanelManager::GetInstance().UpdatePanel();
  Ticker::handle_lv_tasks();
  Ticker::handle_dynamic_delay(millis());
}

void ExecutePanelAction(const TriggerActionRequest& request)
{
  switch (request.type)
  {
    case TriggerActionType::LoadPanel:
      log_i("Executing final panel action: Load %s", request.panelName);
      PanelManager::GetInstance().CreateAndLoadPanel(request.panelName, []() {
        PanelManager::GetInstance().TriggerPanelSwitchCallback("");
      }, request.isTriggerDriven);
      break;
        
    case TriggerActionType::RestorePanel:
      log_i("Executing final panel action: Restore panel");
      {
        auto& pm = PanelManager::GetInstance();
        const char* restorePanel = (pm.restorationPanel && strlen(pm.restorationPanel) > 0) 
                                 ? pm.restorationPanel 
                                 : PanelNames::OIL;
        pm.CreateAndLoadPanel(restorePanel, []() {
          PanelManager::GetInstance().TriggerPanelSwitchCallback("");
        }, request.isTriggerDriven);
      }
      break;
        
    case TriggerActionType::ToggleTheme:
      log_i("Executing final panel action: Toggle theme %s", request.panelName);
      StyleManager::GetInstance().set_theme(request.panelName);
      break;
        
    case TriggerActionType::None:
    default:
      // No action needed
      break;
  }
}