#pragma once
#include <memory>

class IGpioProvider;
class IDisplayProvider;
class IStyleService;
class IPanel;

/**
 * @interface IPanelFactory
 * @brief Factory interface for creating screen panels with dependency injection
 *
 * @details This interface provides the contract for creating various screen panels
 * used throughout the Clarity system. It implements the Abstract Factory pattern
 * to enable testability and proper dependency injection of hardware providers and
 * services into panel constructors.
 *
 * @design_pattern Abstract Factory Pattern for panel creation
 * @testability Enables dependency injection of mock factories for testing
 * @memory_management All methods return unique_ptr for clear ownership transfer
 * @dependency_injection Panels receive required providers and services during construction
 *
 * @panel_types:
 * - SplashPanel: Startup branding screen with loading animation
 * - OemOilPanel: Main oil monitoring dashboard with gauges and sensors
 * - ErrorPanel: Error message display with navigation and auto-restoration
 * - ConfigPanel: Configuration interface with hierarchical state machine
 * - KeyPanel: Key status display panel (display-only, trigger-driven)
 * - LockPanel: Lock status display panel (display-only, trigger-driven)
 *
 * @dependency_requirements:
 * - IGpioProvider: Required for sensor data access and hardware interaction
 * - IDisplayProvider: Required for LVGL display operations and UI rendering
 * - IStyleService: Required for theme management and visual styling
 *
 * @usage_pattern:
 * 1. PanelManager requests panels from factory during system initialization
 * 2. Factory creates panel with proper provider and service injection
 * 3. PanelManager manages panel lifecycle (init, load, update, switch)
 * 4. Unique pointer ownership transfers to panel management system
 *
 * Example usage:
 * ```cpp
 * auto panel = panelFactory->CreateOemOilPanel(gpioProvider, displayProvider, styleService);
 * panel->Init();
 * panel->Load(callback_function);
 * panel->Update(update_function);
 * ```
 *
 * This factory centralizes panel creation and ensures all panels
 * receive proper hardware abstraction providers. It enables testing by allowing
 * mock panel injection and maintains clean separation between panel creation
 * and lifecycle management.
 */
class IPanelFactory {
public:
    /// @brief Virtual destructor for interface
    virtual ~IPanelFactory() = default;

    /// @brief Create startup splash screen panel with branding
    /// @param gpio GPIO provider for hardware interaction
    /// @param display Display provider for LVGL UI operations
    /// @param style Style service for theme and visual styling
    /// @return Unique pointer to IPanel instance (SplashPanel)
    virtual std::unique_ptr<IPanel> CreateSplashPanel(IGpioProvider* gpio, IDisplayProvider* display, IStyleService* style) = 0;

    /// @brief Create main oil monitoring dashboard panel
    /// @param gpio GPIO provider for sensor data acquisition
    /// @param display Display provider for gauge and component rendering
    /// @param style Style service for gauge styling and themes
    /// @return Unique pointer to IPanel instance (OemOilPanel)
    virtual std::unique_ptr<IPanel> CreateOemOilPanel(IGpioProvider* gpio, IDisplayProvider* display, IStyleService* style) = 0;

    /// @brief Create error display panel with navigation capabilities
    /// @param gpio GPIO provider for button input handling
    /// @param display Display provider for error message rendering
    /// @param style Style service for error styling
    /// @return Unique pointer to IPanel instance (ErrorPanel)
    virtual std::unique_ptr<IPanel> CreateErrorPanel(IGpioProvider* gpio, IDisplayProvider* display, IStyleService* style) = 0;

    /// @brief Create configuration interface panel
    /// @param gpio GPIO provider for button navigation
    /// @param display Display provider for configuration UI rendering
    /// @param style Style service for configuration styling
    /// @return Unique pointer to IPanel instance (ConfigPanel)
    virtual std::unique_ptr<IPanel> CreateConfigPanel(IGpioProvider* gpio, IDisplayProvider* display, IStyleService* style) = 0;

    /// @brief Create key presence status display panel (trigger-driven)
    /// @param gpio GPIO provider for key sensor reading
    /// @param display Display provider for status indicator rendering
    /// @param style Style service for indicator styling
    /// @return Unique pointer to IPanel instance (KeyPanel)
    virtual std::unique_ptr<IPanel> CreateKeyPanel(IGpioProvider* gpio, IDisplayProvider* display, IStyleService* style) = 0;

    /// @brief Create vehicle lock status display panel (trigger-driven)
    /// @param gpio GPIO provider for lock sensor reading
    /// @param display Display provider for status indicator rendering
    /// @param style Style service for indicator styling
    /// @return Unique pointer to IPanel instance (LockPanel)
    virtual std::unique_ptr<IPanel> CreateLockPanel(IGpioProvider* gpio, IDisplayProvider* display, IStyleService* style) = 0;
};