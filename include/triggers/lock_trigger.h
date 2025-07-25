#pragma once

// System/Library Includes
#include <memory>

// Project Includes
#include "interfaces/i_trigger.h"
#include "sensors/lock_sensor.h"
#include "utilities/types.h"

/**
 * @class LockTrigger
 * @brief Lock detection trigger for panel switching
 * 
 * @details This trigger monitors lock sensor readings and evaluates conditions
 * to determine when the lock panel should be displayed. It supports optional
 * panel restoration when the lock condition clears.
 * 
 * @trigger_role Evaluates lock sensor data to trigger panel switches
 * @data_source LockSensor providing boolean lock status
 * @target_panel LockPanel for displaying lock status
 * 
 * @evaluation_logic:
 * - Monitors lock sensor for state changes
 * - Triggers when lock is detected/engaged
 * - Supports restoration to previous panel when lock disengages
 * 
 * @lifecycle:
 * 1. init(): Initialize sensor and get baseline state
 * 2. evaluate(): Check for lock condition changes
 * 3. get_target_panel(): Return LockPanel identifier
 * 4. should_restore(): Return restoration preference
 * 
 * @context This trigger allows the system to automatically switch to
 * the lock panel when lock-related events occur, providing immediate
 * visual feedback for security status.
 */
class LockTrigger : public ITrigger
{
public:
    // Constructors and Destructors
    LockTrigger(bool enableRestoration = false);

    // Core Functionality Methods
    void init() override;
    bool evaluate() override;
    const char* GetTargetPanel() const override;
    const char* GetId() const override;
    bool ShouldRestore() const override;

private:
    // Static Data Members
    static constexpr const char* TRIGGER_ID = "lock_trigger";

    // Instance Data Members
    std::shared_ptr<LockSensor> lockSensor_;
    bool enableRestoration_;
};