#pragma once

#include "utilities/types.h"
#include "hardware/gpio_pins.h"
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
class TriggerManager
{
public:
    TriggerManager(const TriggerManager &) = delete;
    TriggerManager &operator=(const TriggerManager &) = delete;
    static TriggerManager &GetInstance();

    // Core Functionality
    void init();
    void ProcessTriggerEvents();
    void ExecuteTriggerAction(TriggerMapping* mapping, TriggerExecutionState state);
    
    // Get startup panel override (null if no override needed)
    const char* GetStartupPanelOverride() const { return startupPanelOverride_; }


private:
    TriggerManager() = default;
    ~TriggerManager() = default;

    void setup_gpio_pins();
    void InitializeTriggerMappings();
    void InitializeTriggersFromGpio();
    GpioState ReadAllGpioPins();
    void CheckGpioChanges();
    void CheckTriggerChange(const char* triggerId, bool currentPinState);
    void InitializeTrigger(const char* triggerId, bool currentPinState);
    TriggerMapping* FindTriggerMapping(const char* triggerId);
    void UpdateActiveTriggersSimple(TriggerMapping* mapping, TriggerExecutionState newState);

    // Static trigger mappings array (Core 0 exclusive ownership - no mutex needed)
    static TriggerMapping triggerMappings_[];
    
    // Simplified active trigger tracking
    TriggerMapping* activePanelTrigger_ = nullptr;  // Highest priority active panel trigger
    TriggerMapping* activeThemeTrigger_ = nullptr;  // Active theme trigger (only one at a time)
    
    // Startup panel override (set by InitializeTriggersFromGpio if active triggers require specific panel)
    const char* startupPanelOverride_ = nullptr;
    
};