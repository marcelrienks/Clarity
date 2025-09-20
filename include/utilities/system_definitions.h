#pragma once

#include "utilities/types.h"
#include "utilities/constants.h"
#include "utilities/logging.h"
#include "managers/panel_manager.h"
#include "managers/style_manager.h"
#ifdef CLARITY_DEBUG
#include "managers/error_manager.h"
#endif

/**
 * @file system_definitions.h
 * @brief System trigger and action definitions for the new interrupt architecture
 * 
 * @details This file contains all the system-wide trigger and action definitions
 * exactly as specified in docs/interrupt-architecture.md. These definitions
 * implement the modern Trigger/Action separation architecture.
 */

namespace SystemDefinitions {

inline std::vector<Trigger> GetSystemTriggers(
    class BaseSensor* keyPresentSensor,
    class BaseSensor* keyNotPresentSensor, 
    class BaseSensor* lockSensor,
    class BaseSensor* lightsSensor,
    class BaseSensor* errorSensor = nullptr) {
    
    std::vector<Trigger> triggers = {
        // Key triggers - CRITICAL priority
        {
            .id = "key_present",
            .priority = Priority::CRITICAL,
            .type = TriggerType::PANEL,
            .activateFunc = []() { 
                log_t("KeyPresentActivate() - Loading KEY panel");
                PanelManager::TriggerService().LoadPanel(PanelNames::KEY); 
            },
            .deactivateFunc = []() { 
                log_t("KeyPresentDeactivate() - Checking for restoration");
                PanelManager::TriggerService().CheckRestoration(); 
            },
            .sensor = keyPresentSensor,
            .isActive = false
        },
        {
            .id = "key_not_present", 
            .priority = Priority::CRITICAL,
            .type = TriggerType::PANEL,
            .activateFunc = []() { 
                log_t("KeyNotPresentActivate() - Loading KEY panel");
                PanelManager::TriggerService().LoadPanel(PanelNames::KEY); 
            },
            .deactivateFunc = []() { 
                log_t("KeyNotPresentDeactivate() - Checking for restoration");
                PanelManager::TriggerService().CheckRestoration(); 
            },
            .sensor = keyNotPresentSensor,
            .isActive = false
        },
        
        // Lock trigger - IMPORTANT priority
        {
            .id = "lock",
            .priority = Priority::IMPORTANT,
            .type = TriggerType::PANEL,
            .activateFunc = []() { 
                log_t("LockEngagedActivate() - Loading LOCK panel");
                PanelManager::TriggerService().LoadPanel(PanelNames::LOCK); 
            },
            .deactivateFunc = []() { 
                log_t("LockDisengagedActivate() - Checking for restoration");
                PanelManager::TriggerService().CheckRestoration(); 
            },
            .sensor = lockSensor,
            .isActive = false
        },
        
        // Lights trigger - NORMAL priority
        {
            .id = "lights",
            .priority = Priority::NORMAL,
            .type = TriggerType::STYLE,
            .activateFunc = []() { 
                log_t("LightsOnActivate() - Setting NIGHT theme");
                StyleManager::Instance().SetTheme(Themes::NIGHT); 
            },
            .deactivateFunc = []() { 
                log_t("LightsOffActivate() - Setting DAY theme");
                StyleManager::Instance().SetTheme(Themes::DAY); 
            },
            .sensor = lightsSensor,
            .isActive = false
        }
    };
    
    // Add error trigger if sensor is provided
    // Note: This is a simple event trigger - it doesn't maintain state or activate/deactivate
    // The sensor generates errors when button is pressed, error panel loading is handled automatically
    if (errorSensor) {
        triggers.push_back({
            .id = "error",
            .priority = Priority::CRITICAL,
            .type = TriggerType::PANEL,  // Keep as PANEL type for priority handling
            .activateFunc = []() {
                log_t("ErrorActivate() - Debug error button pressed, generating test errors");

#ifdef CLARITY_DEBUG
                // Generate three test errors for error panel testing
                ErrorManager::Instance().ReportWarning("DebugTest",
                                                       "Test warning from debug error trigger");

                ErrorManager::Instance().ReportError(ErrorLevel::ERROR, "DebugTest",
                                                     "Test error from debug error trigger");

                ErrorManager::Instance().ReportCriticalError("DebugTest",
                                                             "Test critical error from debug error trigger");

                log_t("Debug errors generated: 1 WARNING, 1 ERROR, 1 CRITICAL - error panel will load automatically");
#else
                log_t("Debug error generation not available in release build");
#endif
            },
            .deactivateFunc = []() { 
                // No-op: Push button doesn't have deactivation, no restoration needed
                log_t("ErrorDeactivate() - No action needed for push button");
            },
            .sensor = errorSensor,
            .isActive = false
        });
    }
    
    return triggers;
}

inline std::vector<Action> GetSystemActions() {
    return {
        // Button actions
        {
            .id = "short_press",
            .executeFunc = []() { 
                log_t("ShortPressActivate() - Executing short press action");
                PanelManager::ActionService().HandleShortPress(); 
            },
            .hasTriggered = false,
            .pressType = ActionPress::SHORT
        },
        {
            .id = "long_press",
            .executeFunc = []() { 
                log_t("LongPressActivate() - Executing long press action");
                PanelManager::ActionService().HandleLongPress(); 
            },
            .hasTriggered = false,
            .pressType = ActionPress::LONG
        }
    };
}

} // namespace SystemDefinitions