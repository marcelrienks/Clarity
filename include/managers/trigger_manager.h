#pragma once

#include "utilities/types.h"
#include "hardware/gpio_pins.h"
#include "interfaces/i_gpio_provider.h"
#include "interfaces/i_trigger_service.h"
#include "interfaces/i_panel_service.h"
#include "interfaces/i_style_service.h"
#include "sensors/key_sensor.h"
#include "sensors/lock_sensor.h"
#include "sensors/light_sensor.h"
#include <vector>
#include <functional>
#include <memory>

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
class TriggerManager : public ITriggerService
{
public:
    // Startup panel override method
    const char *getStartupPanelOverride() const override;

    // Constructors and Destructors
    TriggerManager(std::shared_ptr<KeySensor> keySensor, std::shared_ptr<LockSensor> lockSensor, 
                   std::shared_ptr<LightSensor> lightSensor, IPanelService *panelService, IStyleService *styleService);
    TriggerManager(const TriggerManager &) = delete;
    TriggerManager &operator=(const TriggerManager &) = delete;
    ~TriggerManager() = default;

    // ITriggerService Interface Implementation
    void init() override;
    void processTriggerEvents() override;

    // Trigger Management
    void addTrigger(const std::string& triggerName, ISensor *sensor, std::function<void()> callback) override;
    bool hasTrigger(const std::string& triggerName) const override;

private:
    void InitializeTriggersFromSensors();
    GpioState ReadAllSensorStates();
    void CheckSensorChanges();
    void CheckTriggerChange(const char *triggerId, bool currentPinState);
    void InitializeTrigger(const char *triggerId, bool currentPinState);
    Trigger *FindTriggerMapping(const char *triggerId);
    void UpdateActiveTriggersSimple(Trigger *mapping, TriggerExecutionState newState);
    void executeTriggerAction(Trigger *mapping, TriggerExecutionState state);

    // Sensor and service dependencies
    std::shared_ptr<KeySensor> keySensor_;
    std::shared_ptr<LockSensor> lockSensor_;
    std::shared_ptr<LightSensor> lightSensor_;
    IPanelService *panelService_ = nullptr;
    IStyleService *styleService_ = nullptr;
    
    // State tracking
    static Trigger triggers_[];
    Trigger *activePanelTrigger_ = nullptr;  // Highest priority active panel trigger
    Trigger *activeThemeTrigger_ = nullptr;  // Active theme trigger (only one at a time)
    const char *startupPanelOverride_ = nullptr;
    bool initialized_ = false;  // Prevent double initialization
};