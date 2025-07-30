#pragma once

// Project Includes
#include "interfaces/i_panel_factory.h"
#include "interfaces/i_component_factory.h"
#include "interfaces/i_display_provider.h"
#include "interfaces/i_gpio_provider.h"
#include "panels/splash_panel.h"
#include "panels/oem_oil_panel.h"
#include "panels/key_panel.h"
#include "panels/lock_panel.h"
#include "utilities/types.h"

// System Includes
#include <map>
#include <string>
#include <memory>

/**
 * @class PanelFactory
 * @brief Factory for creating panel instances with dependency injection
 * 
 * @details This factory creates panel instances with proper dependency injection,
 * separating panel creation concerns from component creation. It maintains
 * a registry of supported panel types and their creation logic.
 * 
 * @design_pattern Factory - Creates panel objects with injected dependencies
 * @dependency_injection Injects IComponentFactory, IDisplayProvider, IGpioProvider
 * @testability Mockable through IPanelFactory interface
 * @thread_safety Not thread-safe - should be used from single thread
 */
class PanelFactory : public IPanelFactory
{
public:
    /**
     * @brief Constructor with dependency injection
     * @param componentFactory Factory for creating components within panels
     * @param displayProvider Display provider for UI operations
     * @param gpioProvider GPIO provider for hardware access
     */
    PanelFactory(IComponentFactory* componentFactory, 
                 IDisplayProvider* displayProvider, 
                 IGpioProvider* gpioProvider);

    // IPanelFactory implementation
    std::unique_ptr<IPanel> createPanel(const std::string& panelType) override;
    bool supportsPanel(const std::string& panelType) const override;

private:
    // Dependencies
    IComponentFactory* componentFactory_;
    IDisplayProvider* displayProvider_;
    IGpioProvider* gpioProvider_;
};