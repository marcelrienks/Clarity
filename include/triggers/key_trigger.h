#pragma once

#include "interfaces/i_trigger.h"
#include "sensors/key_sensor.h"
#include "utilities/types.h"
#include <memory>

/**
 * @class KeyTrigger
 * @brief Interrupt trigger that monitors key sensor and switches to key panel
 * 
 * @details This trigger monitors the KeySensor for key presence/insertion events
 * and triggers an immediate switch to the KeyPanel when a key is detected.
 * It demonstrates the interrupt system concept where a specific condition
 * (key insertion) causes an immediate panel switch regardless of current state.
 * 
 * @trigger_condition: Key sensor returns true (key present/inserted)
 * @target_panel: KeyPanel (PanelNames::Key)
 * @restoration: Optional restoration to previous panel when key is removed
 * 
 * @use_cases:
 * - Security monitoring: Immediately show key status when key is detected
 * - User interface: Switch to key-specific controls when key is inserted
 * - System state: Change application behavior based on key presence
 * 
 * @implementation_details:
 * - Uses KeySensor for hardware abstraction
 * - Evaluates sensor reading each check cycle
 * - Supports both one-shot and restoration modes
 * - Thread-safe evaluation through sensor interface
 * 
 * @context: This is an example implementation showing how sensors can be
 * used with the interrupt system to create responsive UI behavior.
 */
class KeyTrigger : public ITrigger
{
public:
    /// @brief Constructor with optional restoration mode
    /// @param enable_restoration Whether to restore previous panel when key is removed
    KeyTrigger(bool enable_restoration = false);

    void init() override;
    bool evaluate() override;
    const char* get_target_panel() const override;
    const char* get_id() const override;
    bool should_restore() const override;

private:
    std::shared_ptr<KeySensor> _key_sensor; ///< Key sensor for monitoring key status
    bool _enable_restoration;                ///< Whether to restore previous panel when condition clears
    bool _previous_state = false;            ///< Previous key state for edge detection
    static constexpr const char* TRIGGER_ID = "key_trigger"; ///< Unique trigger identifier
};