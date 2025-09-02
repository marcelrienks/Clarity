#pragma once

#include "utilities/types.h"
#include "utilities/constants.h"
#include "managers/panel_manager.h"
#include "managers/style_manager.h"

/**
 * @file system_definitions.h
 * @brief System trigger and action definitions for the new interrupt architecture
 * 
 * @details This file contains all the system-wide trigger and action definitions
 * exactly as specified in docs/interrupt-architecture.md. These definitions
 * replace the legacy interrupt system with the new Trigger/Action separation.
 */

namespace SystemDefinitions {

/// @brief Get all system triggers as documented in interrupt-architecture.md
/// @param keyPresentSensor Pointer to the key present sensor
/// @param keyNotPresentSensor Pointer to the key not present sensor  
/// @param lockSensor Pointer to the lock sensor
/// @param lightsSensor Pointer to the lights sensor
/// @param errorSensor Pointer to the error sensor (if available)
/// @return Array of system triggers
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
            .activateFunc = []() { PanelManager::Instance().LoadPanel(PanelNames::KEY); },
            .deactivateFunc = []() { PanelManager::Instance().CheckRestoration(); },
            .sensor = keyPresentSensor,
            .canBeOverriddenOnActivate = false,
            .isActive = false
        },
        {
            .id = "key_not_present", 
            .priority = Priority::CRITICAL,
            .type = TriggerType::PANEL,
            .activateFunc = []() { PanelManager::Instance().LoadPanel(PanelNames::KEY); },
            .deactivateFunc = []() { PanelManager::Instance().CheckRestoration(); },
            .sensor = keyNotPresentSensor,
            .canBeOverriddenOnActivate = false,
            .isActive = false
        },
        
        // Lock trigger - IMPORTANT priority
        {
            .id = "lock",
            .priority = Priority::IMPORTANT,
            .type = TriggerType::PANEL,
            .activateFunc = []() { PanelManager::Instance().LoadPanel(PanelNames::LOCK); },
            .deactivateFunc = []() { PanelManager::Instance().CheckRestoration(); },
            .sensor = lockSensor,
            .canBeOverriddenOnActivate = false,
            .isActive = false
        },
        
        // Lights trigger - NORMAL priority
        {
            .id = "lights",
            .priority = Priority::NORMAL,
            .type = TriggerType::STYLE,
            .activateFunc = []() { StyleManager::Instance().SetTheme(Themes::NIGHT); },
            .deactivateFunc = []() { StyleManager::Instance().SetTheme(Themes::DAY); },
            .sensor = lightsSensor,
            .canBeOverriddenOnActivate = true,  // Can be overridden by higher priority
            .isActive = false
        }
    };
    
    // Add error trigger if sensor is provided
    if (errorSensor) {
        triggers.push_back({
            .id = "error",
            .priority = Priority::CRITICAL,
            .type = TriggerType::PANEL,
            .activateFunc = []() { PanelManager::Instance().LoadPanel(PanelNames::ERROR); },
            .deactivateFunc = []() { PanelManager::Instance().CheckRestoration(); },
            .sensor = errorSensor,
            .canBeOverriddenOnActivate = false,
            .isActive = false
        });
    }
    
    return triggers;
}

/// @brief Get all system actions as documented in interrupt-architecture.md
/// @return Array of system actions
inline std::vector<Action> GetSystemActions() {
    return {
        // Button actions
        {
            .id = "short_press",
            .executeFunc = []() { PanelManager::Instance().HandleShortPress(); },
            .hasTriggered = false,
            .pressType = ActionPress::SHORT
        },
        {
            .id = "long_press",
            .executeFunc = []() { PanelManager::Instance().HandleLongPress(); },
            .hasTriggered = false,
            .pressType = ActionPress::LONG
        }
    };
}

} // namespace SystemDefinitions