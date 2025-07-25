#pragma once

#include "interfaces/i_trigger.h"
#include "utilities/types.h"

/**
 * @class LockTrigger
 * @brief Simplified trigger for lock engagement detection
 * 
 * @details Demonstrates the simplified trigger pattern:
 * - Action: Show lock panel when lock is engaged (GPIO HIGH)
 * - Restore: Return to previous panel when lock is disengaged (GPIO LOW)
 * - Priority: IMPORTANT (lower than safety-critical alerts)
 */
class LockTrigger : public AlertTrigger
{
public:
    LockTrigger();
    void init() override;

private:
    static void LoadLockPanel();
    static void RestorePreviousPanel();
};