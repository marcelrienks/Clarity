#pragma once

#include "interfaces/i_component.h"
#include "interfaces/i_display_provider.h"
#include "interfaces/i_gpio_provider.h"
#include "interfaces/i_panel.h"
#include "interfaces/i_style_service.h"
#include <memory>

/**
 * @class UIFactory
 * @brief Static factory for creating UI components and panels
 *
 * @details This factory provides centralized creation of all UI elements
 * in the Clarity system. It implements the Factory design pattern to
 * encapsulate object creation logic and manage dependencies.
 *
 * @design_pattern Factory - centralizes object creation with dependency injection
 * @creation_strategy:
 * - Static methods for type-safe object creation
 * - Automatic dependency injection via constructor parameters
 * - Returns smart pointers for memory safety
 *
 * @component_creation:
 * - Key components: Key status indicators
 * - Lock components: Lock status displays
 * - Oil components: Pressure and temperature gauges
 * - Branding components: Clarity logo/text
 *
 * @panel_creation:
 * - Splash panels: Startup branding screens
 * - OEM oil panels: Main monitoring dashboards
 * - Status panels: Key and lock displays
 *
 * @dependency_management:
 * - All objects receive required services via constructors
 * - StyleService: Theme and styling management
 * - DisplayProvider: LVGL display operations
 * - GpioProvider: Hardware GPIO access
 *
 * @memory_safety:
 * - Returns std::unique_ptr for automatic cleanup
 * - No manual memory management required
 * - Exception-safe construction
 *
 * @context This factory is used throughout the system to create
 * UI objects with proper dependency injection. It's the central
 * creation point for all components and panels.
 */
class UIFactory
{
  public:
    // Component creation methods
    static std::unique_ptr<IComponent> createKeyComponent(IStyleService *styleService);
    static std::unique_ptr<IComponent> createLockComponent(IStyleService *styleService);
    static std::unique_ptr<IComponent> createClarityComponent(IStyleService *styleService);
    static std::unique_ptr<IComponent> createOemOilPressureComponent(IStyleService *styleService);
    static std::unique_ptr<IComponent> createOemOilTemperatureComponent(IStyleService *styleService);
    static std::unique_ptr<IComponent> createErrorComponent(IStyleService *styleService);

    // Panel creation methods
    static std::unique_ptr<IPanel> createKeyPanel(IGpioProvider *gpio, IDisplayProvider *display,
                                                  IStyleService *styleService);
    static std::unique_ptr<IPanel> createLockPanel(IGpioProvider *gpio, IDisplayProvider *display,
                                                   IStyleService *styleService);
    static std::unique_ptr<IPanel> createSplashPanel(IGpioProvider *gpio, IDisplayProvider *display,
                                                     IStyleService *styleService);
    static std::unique_ptr<IPanel> createOemOilPanel(IGpioProvider *gpio, IDisplayProvider *display,
                                                     IStyleService *styleService);
    static std::unique_ptr<IPanel> createErrorPanel(IGpioProvider *gpio, IDisplayProvider *display,
                                                    IStyleService *styleService);
    static std::unique_ptr<IPanel> createConfigPanel(IGpioProvider *gpio, IDisplayProvider *display,
                                                     IStyleService *styleService);
};