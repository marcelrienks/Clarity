#pragma once

#include "interfaces/i_trigger.h"
#include "utilities/types.h"

/**
 * @class LightsTrigger
 * @brief Simplified trigger for lights detection with theme switching actions
 * 
 * @details Monitors lights GPIO pin and changes theme based on lighting conditions:
 * - Action: Switch to night theme when lights are on (GPIO HIGH)
 * - Restore: Switch to day theme when lights are off (GPIO LOW)
 * - Priority: NORMAL (lowest priority - aesthetic changes)
 */
class LightsTrigger : public AlertTrigger
{
public:
    LightsTrigger();
    void init() override;
    
    TriggerActionRequest GetActionRequest() override;
    TriggerActionRequest GetRestoreRequest() override;
};