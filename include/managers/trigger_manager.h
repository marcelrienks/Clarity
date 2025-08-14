#pragma once

#include "hardware/gpio_pins.h"
#include "interfaces/i_interrupt_service.h"
#include "utilities/types.h"

/// @brief Consolidated GPIO state structure for single-read pattern
struct GpioState
{
    bool keyPresent;
    bool keyNotPresent;
    bool lockState;
    bool lightsState;
};
#include "interfaces/i_gpio_provider.h"
#include "interfaces/i_panel_service.h"
#include "interfaces/i_style_service.h"
#include "interfaces/i_trigger_service.h"
#include "sensors/debug_error_sensor.h"
#include "sensors/key_sensor.h"
#include "sensors/lights_sensor.h"
#include "sensors/lock_sensor.h"
#include <functional>
#include <memory>
#include <vector>

/**
 * @class TriggerManager
 * @brief Simplified direct GPIO polling trigger manager with mapping-based architecture
 *
 * @architecture:
 * - Direct GPIO polling and UI management
 * - Static trigger mappings replace dynamic trigger objects
 *
 * @key_simplifications:
 * 1. Direct GPIO polling - no interrupts or queues
 * 2. Pin change detection via state comparison
 * 3. Static TriggerMapping array instead of objects
 * 4. Priority evaluation from lowest to highest (highest priority action wins)
 * 5. No cross-core communication needed
 */
class TriggerManager : public ITriggerService, public IInterruptService
{
  public:
    // Startup panel override method
    const char *GetStartupPanelOverride() const override;

    // Constructors and Destructors
    TriggerManager(std::shared_ptr<KeySensor> keySensor, std::shared_ptr<LockSensor> lockSensor,
                   std::shared_ptr<LightsSensor> lightSensor, std::shared_ptr<DebugErrorSensor> debugErrorSensor,
                   IPanelService *panelService, IStyleService *styleService);
    TriggerManager(const TriggerManager &) = delete;
    TriggerManager &operator=(const TriggerManager &) = delete;
    ~TriggerManager() = default;

    // ITriggerService Interface Implementation
    void Init() override;
    void ProcessTriggerEvents() override;

    // IInterruptService Interface Implementation

    /**
     * @brief Process trigger events (IInterruptService interface)
     * @details Called by InterruptManager during idle time
     */
    void Process() override { ProcessTriggerEvents(); }

    // Trigger Management
    void AddTrigger(const std::string &triggerName, ISensor *sensor, std::function<void()> callback) override;
    bool HasTrigger(const std::string &triggerName) const override;

  private:
    void InitializeTriggersFromSensors();
    GpioState ReadAllSensorStates();
    void CheckSensorChanges();
    void CheckErrorTrigger();
    void CheckTriggerChange(const char *triggerId, bool currentPinState);
    void InitializeTrigger(const char *triggerId, bool currentPinState);
    Trigger *FindTriggerMapping(const char *triggerId);
    void ExecuteTriggerAction(Trigger *mapping, TriggerExecutionState state) override;

    // Simple active trigger tracking helper
    Trigger *FindActivePanel();

    // Sensor and service dependencies
    std::shared_ptr<KeySensor> keySensor_;
    std::shared_ptr<LockSensor> lockSensor_;
    std::shared_ptr<LightsSensor> lightSensor_;
    std::shared_ptr<DebugErrorSensor> debugErrorSensor_;
    IPanelService *panelService_ = nullptr;
    IStyleService *styleService_ = nullptr;

    // Trigger definitions
    static Trigger triggers_[]; // Static array of all trigger definitions

    // Initialization state
    const char *startupPanelOverride_ = nullptr; // Panel to load at startup if triggers active
    bool initialized_ = false;                   // Prevent double initialization
};