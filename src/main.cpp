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

  // Core 0 responsibilities: trigger event processing, trigger evaluation and UI updates
  TriggerManager::GetInstance().ProcessPendingTriggerEvents();
  auto triggerRequests = TriggerManager::GetInstance().EvaluateAndGetTriggerRequests();
  
  // Process trigger requests using dependency injection
  for (const auto& request : triggerRequests)
  {
    switch (request.type)
    {
      case TriggerActionType::LoadPanel:
        log_i("Processing LoadPanel request: %s", request.panelName);
        PanelManager::GetInstance().CreateAndLoadPanel(request.panelName, []() {
          PanelManager::GetInstance().TriggerPanelSwitchCallback("");
        }, request.isTriggerDriven);
        break;
        
      case TriggerActionType::RestorePanel:
        log_i("Processing RestorePanel request");
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
        log_i("Processing ToggleTheme request: %s", request.panelName);
        StyleManager::GetInstance().set_theme(request.panelName);
        break;
        
      case TriggerActionType::None:
      default:
        // No action needed
        break;
    }
  }
  
  PanelManager::GetInstance().UpdatePanel();
  Ticker::handle_lv_tasks();

  Ticker::handle_dynamic_delay(millis());
}