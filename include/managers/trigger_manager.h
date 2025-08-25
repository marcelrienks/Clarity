#pragma once

#include "hardware/gpio_pins.h"
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
#include "managers/interrupt_manager.h"
#include <functional>
#include <memory>
#include <vector>

/**
 * @class TriggerManager
 * @brief Coordinated interrupt-based trigger manager using new interrupt system
 *
 * @architecture:
 * - Phase 4: Converted to use coordinated interrupt system
 * - Registers sensor interrupts instead of polling in Process()
 * - Static trigger mappings with interrupt-based evaluation
 *
 * @key_improvements:
 * 1. Event-driven via coordinated interrupts instead of polling
 * 2. Automatic sensor state change detection via BaseSensor
 * 3. Priority-based interrupt processing
 * 4. Memory-safe static interrupt registration
 * 5. Maintains backward compatibility
 */
class TriggerManager : public ITriggerService
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


    // Trigger Management
    void AddTrigger(const std::string &triggerName, ISensor *sensor, std::function<void()> callback) override;
    bool HasTrigger(const std::string &triggerName) const override;

  private:
    // Static callback functions for interrupt system
    static bool EvaluateTriggerChange(void* context);
    static void ExecuteTriggerAction(void* context);
    
    void RegisterTriggerInterrupts();
    void InitializeTriggersFromSensors();
    bool ValidateSensors();
    void InitializeAllTriggers(const GpioState& state);
    void ApplyStartupActions();
    void ApplyStartupTheme(const char* themeName);
    void ApplyStartupPanel(const char* panelName);
    
    GpioState ReadAllSensorStates();
    void CheckSensorChanges();
    void CheckErrorTrigger();
    void CheckTriggerChange(const char *triggerId, bool currentPinState);
    void InitializeTrigger(const char *triggerId, bool currentPinState);
    Trigger *FindTriggerMapping(const char *triggerId);
    void ExecuteTriggerAction(Trigger *mapping, TriggerExecutionState state) override;

    // Helper methods for ExecuteTriggerAction
    void ExecuteActivation(Trigger *mapping);
    void ExecuteDeactivation(Trigger *mapping);
    void HandlePanelDeactivation(Trigger *mapping);
    void LoadPanel(const char *panelName);
    void SetTheme(const char *themeName);

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