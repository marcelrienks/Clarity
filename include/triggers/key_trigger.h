#pragma once

// System/Library Includes
#include <memory>

// Project Includes
#include "interfaces/i_trigger.h"
#include "sensors/key_sensor.h"
#include "utilities/types.h"

/**
 * @class KeyTrigger
 * @brief Unified interrupt trigger that monitors key states and switches panels accordingly
 * 
 * @details This trigger monitors both key present and not present states using a single
 * KeySensor, and triggers appropriate panel switches based on the key state:
 * - KeyState::Present: Switch to KeyPanel (green key)
 * - KeyState::NotPresent: Switch to KeyPanel (red key)
 * - KeyState::Inactive: Restore previous panel
 * 
 * @trigger_conditions:
 * - Key present pin HIGH (key detected/inserted) -> KeyPanel with green key
 * - Key not present pin HIGH (key removed/not detected) -> KeyPanel with red key
 * - Both pins LOW (inactive state) -> Restore previous panel
 * 
 * @target_panels:
 * - KeyPanel for both present and not present states
 * - Previous panel restoration when inactive
 * 
 * @use_cases:
 * - Automotive ignition monitoring: Show appropriate key status when requested
 * - Security monitoring: Immediate visual feedback on key state changes
 * - User interface: Unified key state handling with proper restoration
 * 
 * @implementation_details:
 * - Uses KeySensor for unified state reading
 * - Tracks previous key state to detect changes
 * - Supports restoration to previous panel when key becomes inactive
 * - Single point of truth for all key-related triggers
 * 
 * @context: This replaces the separate KeyPresentTrigger and KeyNotPresentTrigger
 * to provide a more cohesive and maintainable key monitoring system.
 */
class KeyTrigger : public ITrigger
{
public:
    // Constructors and Destructors
    /// @brief Constructor with optional restoration mode
    /// @param enable_restoration Whether to restore previous panel when key becomes inactive
    KeyTrigger(bool enable_restoration = true);

    // Core Functionality Methods
    void init() override;
    bool evaluate() override;
    const char* get_target_panel() const override;
    const char* get_id() const override;
    bool should_restore() const override;
    
    /// @brief Get current key state (public interface for components)
    /// @return Current key state as Reading variant
    Reading get_reading();

private:
    // Private Data Members
    bool _enable_restoration;                ///< Whether to restore previous panel when key becomes inactive
    KeyState _last_key_state;                ///< Previous key state for change detection
    KeySensor _key_sensor;                   ///< Key sensor for state reading
    static constexpr const char* TRIGGER_ID = "key_trigger"; ///< Unique trigger identifier
};