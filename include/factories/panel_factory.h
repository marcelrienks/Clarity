#pragma once

#include <memory>
#include "interfaces/i_panel_factory.h"

class IGpioProvider;
class IDisplayProvider;
class IStyleService;
class SplashPanel;
class OemOilPanel;
class ErrorPanel;
class ConfigPanel;
class KeyPanel;
class LockPanel;

/**
 * @class PanelFactory
 * @brief Singleton factory for creating panel instances
 * 
 * @details This factory creates panel instances used in the MVP architecture's
 * Presenter layer. It follows the singleton pattern to provide a global panel 
 * creation service. This factory supplements the main dual factory pattern 
 * (ProviderFactory/ManagerFactory) by handling panel-level object creation.
 * 
 * @design_pattern Singleton Factory Pattern
 * @architectural_role Panel creation for MVP architecture Presenters layer
 * @dependency_injection Panels receive GPIO, Display, and Style providers
 * @testability Implements IPanelFactory interface for test injection
 * 
 * @architecture_note
 * While not documented in the main architectural overview, this factory serves
 * a crucial role in the MVP pattern by creating Presenter layer panels that
 * coordinate UI components and business logic. It complements the documented 
 * dual factory pattern by handling panel lifecycle management.
 * 
 * @panel_types
 * - SplashPanel: Startup animation and branding
 * - OemOilPanel: Primary monitoring dashboard with gauges
 * - KeyPanel: Security key status indicator (display-only)
 * - LockPanel: Vehicle security status indicator (display-only)  
 * - ErrorPanel: System error management with navigation
 * - ConfigPanel: Hierarchical system configuration
 * 
 * @example
 * @code
 * // Usage within PanelManager
 * auto& factory = PanelFactory::Instance();
 * auto oilPanel = factory.CreateOemOilPanel(gpio, display, style);
 * auto keyPanel = factory.CreateKeyPanel(gpio, display, style);
 * @endcode
 */
class PanelFactory : public IPanelFactory {
public:
    static PanelFactory& Instance();
    
    // IPanelFactory implementation
    std::unique_ptr<IPanel> CreateSplashPanel(IGpioProvider* gpio, IDisplayProvider* display, IStyleService* style) override;
    std::unique_ptr<IPanel> CreateOemOilPanel(IGpioProvider* gpio, IDisplayProvider* display, IStyleService* style) override;
    std::unique_ptr<IPanel> CreateErrorPanel(IGpioProvider* gpio, IDisplayProvider* display, IStyleService* style) override;
    std::unique_ptr<IPanel> CreateConfigPanel(IGpioProvider* gpio, IDisplayProvider* display, IStyleService* style) override;
    std::unique_ptr<IPanel> CreateKeyPanel(IGpioProvider* gpio, IDisplayProvider* display, IStyleService* style) override;
    std::unique_ptr<IPanel> CreateLockPanel(IGpioProvider* gpio, IDisplayProvider* display, IStyleService* style) override;
    
private:
    PanelFactory() = default;
    ~PanelFactory() = default;
    PanelFactory(const PanelFactory&) = delete;
    PanelFactory& operator=(const PanelFactory&) = delete;
};