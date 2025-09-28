#pragma once

#include "definitions/types.h"
#include "definitions/constants.h"
#include "utilities/logging.h"
#include "managers/panel_manager.h"
#include "managers/style_manager.h"
#ifdef CLARITY_DEBUG
#include "managers/error_manager.h"
#include <Arduino.h>
#endif

/**
 * @file definitions/interrupts.h
 * @brief System trigger and action definitions for the interrupt architecture
 *
 * @details This file contains all the system-wide interrupt trigger and action
 * definitions exactly as specified in docs/interrupt-architecture.md. These
 * definitions implement the modern Trigger/Action separation architecture for
 * interrupt-driven system behavior.
 */

namespace Interrupts {

inline std::vector<Trigger> GetSystemTriggers(
    class BaseSensor* keyPresentSensor,
    class BaseSensor* keyNotPresentSensor, 
    class BaseSensor* lockSensor,
    class BaseSensor* lightsSensor,
    class BaseSensor* errorSensor = nullptr) {
    
    std::vector<Trigger> triggers = {
        // Key triggers - CRITICAL priority
        {
            .id = TriggerIds::KEY_PRESENT,
            .priority = Priority::CRITICAL,
            .type = TriggerType::PANEL,
            .activateFunc = []() { 
                log_t("KeyPresentActivate() - Loading KEY panel");
                PanelManager::Instance().LoadPanel(PanelNames::KEY); 
            },
            .deactivateFunc = []() { 
                log_t("KeyPresentDeactivate() - Checking for restoration");
                PanelManager::Instance().CheckRestoration(); 
            },
            .sensor = keyPresentSensor,
            .isActive = false
        },
        {
            .id = TriggerIds::KEY_NOT_PRESENT, 
            .priority = Priority::CRITICAL,
            .type = TriggerType::PANEL,
            .activateFunc = []() { 
                log_t("KeyNotPresentActivate() - Loading KEY panel");
                PanelManager::Instance().LoadPanel(PanelNames::KEY); 
            },
            .deactivateFunc = []() { 
                log_t("KeyNotPresentDeactivate() - Checking for restoration");
                PanelManager::Instance().CheckRestoration(); 
            },
            .sensor = keyNotPresentSensor,
            .isActive = false
        },
        
        // Lock trigger - IMPORTANT priority
        {
            .id = TriggerIds::LOCK,
            .priority = Priority::IMPORTANT,
            .type = TriggerType::PANEL,
            .activateFunc = []() { 
                log_t("LockEngagedActivate() - Loading LOCK panel");
                PanelManager::Instance().LoadPanel(PanelNames::LOCK); 
            },
            .deactivateFunc = []() { 
                log_t("LockDisengagedActivate() - Checking for restoration");
                PanelManager::Instance().CheckRestoration(); 
            },
            .sensor = lockSensor,
            .isActive = false
        },
        
        // Lights trigger - NORMAL priority
        {
            .id = TriggerIds::LIGHTS,
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
            .id = TriggerIds::ERROR,
            .priority = Priority::CRITICAL,
            .type = TriggerType::PANEL,  // Keep as PANEL type for priority handling
            .activateFunc = []() {
                log_t("ErrorActivate() - Debug error button pressed, generating test errors");

#ifdef CLARITY_DEBUG
                // Generate three test errors for error panel testing (unique timestamps)
                uint32_t timestamp = millis();
                ErrorManager::Instance().ReportWarning("DebugTest",
                                                       "Test warning from debug error trigger @" + std::to_string(timestamp));

                ErrorManager::Instance().ReportError(ErrorLevel::ERROR, "DebugTest",
                                                     "Test error from debug error trigger @" + std::to_string(timestamp));

                ErrorManager::Instance().ReportCriticalError("DebugTest",
                                                             "Test critical error from debug error trigger @" + std::to_string(timestamp));

                log_t("Debug errors generated: 1 WARNING, 1 ERROR, 1 CRITICAL - error panel will load automatically");
#else
                log_t("Debug error generation not available in release build");
#endif
            },
            .deactivateFunc = nullptr,  // No deactivate - one-shot button press only
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
            .id = TriggerIds::SHORT_PRESS,
            .executeFunc = []() { 
                log_t("ShortPressActivate() - Executing short press action");
                PanelManager::ActionService().HandleShortPress(); 
            },
            .hasTriggered = false,
            .pressType = ActionPress::SHORT
        },
        {
            .id = TriggerIds::LONG_PRESS,
            .executeFunc = []() { 
                log_t("LongPressActivate() - Executing long press action");
                PanelManager::ActionService().HandleLongPress(); 
            },
            .hasTriggered = false,
            .pressType = ActionPress::LONG
        }
    };
}

} // namespace Interrupts