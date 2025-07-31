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

    // Static Methods removed - using dependency injection

    // ITriggerService Interface Implementation
    void init() override;
    void processTriggerEvents() override;
    void executeTriggerAction(Trigger* mapping, TriggerExecutionState state) override;
    const char* getStartupPanelOverride() const override { return startupPanelOverride_; }
    
    // Legacy Methods (for backward compatibility during transition)
    void ProcessTriggerEvents() { processTriggerEvents(); }
    void ExecuteTriggerAction(Trigger* mapping, TriggerExecutionState state) { executeTriggerAction(mapping, state); }
    const char* GetStartupPanelOverride() const { return getStartupPanelOverride(); }


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

    // Static trigger mappings array (Core 0 exclusive ownership - no mutex needed)
    static Trigger triggers_[];
    
    // Simplified active trigger tracking
    Trigger* activePanelTrigger_ = nullptr;  // Highest priority active panel trigger
    Trigger* activeThemeTrigger_ = nullptr;  // Active theme trigger (only one at a time)
    
    // Startup panel override (set by InitializeTriggersFromGpio if active triggers require specific panel)
    const char* startupPanelOverride_ = nullptr;
    
    // Hardware provider
    IGpioProvider* gpioProvider_ = nullptr;
    
    // Service dependencies (Step 4.5: Added for dependency injection)
    IPanelService* panelService_ = nullptr;
    IStyleService* styleService_ = nullptr;
};