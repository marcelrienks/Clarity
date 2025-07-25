#pragma once

#include "interfaces/i_trigger.h"
#include "utilities/types.h"

/**
 * @class KeyTrigger
 * @brief Simplified trigger for key presence detection with action/restore pattern
 * 
 * @details Demonstrates the new simplified trigger system:
 * - Action: Show key panel when GPIO pin goes HIGH (key detected)
 * - Restore: Restore previous panel when GPIO pin goes LOW (key removed)
 * - Priority: CRITICAL (highest priority for safety-related alerts)
 * - State: Active/inactive controlled directly by GPIO pin state
 */
class KeyTrigger : public AlertTrigger
{
public:
    KeyTrigger();
    void init() override;

private:
    static void LoadKeyPanel();
    static void RestorePreviousPanel();
};