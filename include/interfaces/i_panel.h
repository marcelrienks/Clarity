#pragma once

// System/Library Includes
#include <lvgl.h>
#include <memory>

// Project Includes
#include "interfaces/i_display_provider.h"
#include "interfaces/i_gpio_provider.h"
#include "interfaces/i_sensor.h"
#include "interfaces/i_action_service.h"

// Forward declarations
class IPanelService;
class IStyleService;

/**
 * @interface IPanel
 * @brief Base interface for all screen panels with universal button handling
 *
 * @details This interface defines the contract for panels that manage complete
 * screens in the MVP architecture. Panels act as Presenters, coordinating
 * between sensors (models) and components (views) to create cohesive displays.
 * All panels implement IActionService for consistent button input handling.
 *
 * @design_pattern Presenter in MVP - coordinates models and views
 * @universal_button_system All panels inherit IActionService to provide
 * static button function pointers for universal button interrupt injection
 *
 * @lifecycle:
 * 1. init(): Initialize panel and create components
 * 2. load(): Setup UI and start async operations with callback
 * 3. update(): Periodic refresh of sensor data and UI
 * 4. show(): Make panel visible on screen
 *
 * @async_handling:
 * - load() accepts completion callbacks for smooth transitions
 * - Panels can perform time-consuming initialization without blocking
 * - PanelManager uses callbacks to coordinate panel switching
 *
 * @sensor_integration:
 * - Display-only panels (Key, Lock): Create only components, no sensors
 * - Data panels (Oil): Create own data sensors and components
 * - Trigger panels receive state from interrupt system via GPIO reads
 *
 * @button_integration:
 * - All panels must implement IActionService methods
 * - Panel functions injected into universal button interrupts when panel loads
 * - Button events execute current panel's functions with panel context
 *
 * @implementations:
 * - SplashPanel: Startup branding screen
 * - OemOilPanel: Main oil monitoring dashboard (creates own sensors)
 * - KeyPanel: Key status display (display-only)
 * - LockPanel: Lock status display (display-only)
 * - ErrorPanel: Error display with navigation and auto-restoration
 * - ConfigPanel: Configuration with hierarchical state machine
 *
 * @context All panels coordinate complete screen experiences and provide
 * consistent button handling through IActionService inheritance.
 */
class IPanel : public IActionService
{
  public:
    // Destructors
    virtual ~IPanel() = default;

    // Core Interface Methods
    /// @brief Initialize the panel and its components
    /// Uses stored providers injected via constructor
    virtual void Init() = 0;

    /// @brief Load the panel (async completion via notification service)
    /// Uses stored providers injected via constructor
    /// Calls notification service when loading is complete
    virtual void Load() = 0;

    /// @brief Update the panel data (async completion via notification service)  
    /// Uses stored providers injected via constructor
    /// Calls notification service when update is complete
    virtual void Update() = 0;

    // Note: Button handling methods inherited from IActionService
    // All panels must implement:
    // - void (*GetShortPressFunction())(void* panelContext)
    // - void (*GetLongPressFunction())(void* panelContext) 
    // - void* GetPanelContext()

    /// @brief Set manager services for panels that need them
    /// @param panelService Service for panel switching
    /// @param styleService Service for theme management
    /// @details Called after panel construction to inject manager dependencies
    virtual void SetManagers(IPanelService *panelService, IStyleService *styleService)
    {
    }

  protected:
    // Protected Data Members
    lv_obj_t *screen_ = nullptr;
};