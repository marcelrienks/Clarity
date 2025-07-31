#pragma once

#include "utilities/types.h"
#include "hardware/gpio_pins.h"
#include "interfaces/i_gpio_provider.h"
#include "interfaces/i_trigger_service.h"
#include "interfaces/i_panel_service.h"
#include "interfaces/i_style_service.h"
#include <esp32-hal-log.h>
#include <vector>

/**
 * @class TriggerManager
 * @brief Simplified direct GPIO polling trigger manager with mapping-based architecture
 * 
 * @architecture:
 * - Core 0: Direct GPIO polling and UI management
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
    // Constructors and Destructors
    TriggerManager(IGpioProvider* gpio = nullptr, IPanelService* panelService = nullptr, IStyleService* styleService = nullptr);
    TriggerManager(const TriggerManager &) = delete;
    TriggerManager &operator=(const TriggerManager &) = delete;
    ~TriggerManager() = default;

    // ITriggerService Interface Implementation
    void init() override;
    void processTriggerEvents() override;

    // Trigger Management
    void addTrigger(const std::string& triggerName, ISensor* sensor, std::function<void()> callback) override;
    bool hasTrigger(const std::string& triggerName) const override;

    // Legacy Methods (for backward compatibility during transition)
    void ProcessTriggerEvents() { processTriggerEvents(); }

private:
    void setup_gpio_pins();
    void InitializeTriggerMappings();
    void InitializeTriggersFromGpio();
    GpioState ReadAllGpioPins();
    void CheckGpioChanges();
    void CheckTriggerChange(const char* triggerId, bool currentPinState);
    void InitializeTrigger(const char* triggerId, bool currentPinState);
    Trigger* FindTriggerMapping(const char* triggerId);
    void UpdateActiveTriggersSimple(Trigger* mapping, TriggerExecutionState newState);

    // Hardware and service dependencies
    IGpioProvider* gpioProvider_ = nullptr;
    IPanelService* panelService_ = nullptr;
    IStyleService* styleService_ = nullptr;
    
    // State tracking
    std::unordered_map<std::string, std::pair<ISensor*, std::function<void()>>> triggers_;
    Trigger* activePanelTrigger_ = nullptr;  // Highest priority active panel trigger
    Trigger* activeThemeTrigger_ = nullptr;  // Active theme trigger (only one at a time)
    const char* startupPanelOverride_ = nullptr;
};