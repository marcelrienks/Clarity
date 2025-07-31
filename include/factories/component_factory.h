#pragma once

// System/Library Includes
#include <unordered_map>
#include <memory>
#include <string>
#include <functional>

// Project Includes
#include "interfaces/i_component_factory.h"
#include "interfaces/i_component.h"
#include "interfaces/i_panel.h"
#include "interfaces/i_display_provider.h"
#include "interfaces/i_gpio_provider.h"
#include "interfaces/i_style_service.h"

/**
 * @class ComponentFactory
 * @brief Concrete implementation of IComponentFactory for component and panel creation with dependency injection
 * 
 * @details This factory handles the creation of components and panels with their required dependencies.
 * It maintains registries of factory functions and provides dependency injection capabilities.
 * Components receive both IDisplayProvider and IStyleService dependencies, while panels receive
 * IGpioProvider and IDisplayProvider dependencies.
 * 
 * @design_pattern Abstract Factory - Creates families of related objects (components/panels)
 * @design_pattern Dependency Injection - Injects required services during object creation
 * @testability Factory can be mocked and components tested with injected test dependencies
 */
class ComponentFactory : public IComponentFactory
{
public:
    ComponentFactory(IStyleService* styleService, IDisplayProvider* displayProvider);
    virtual ~ComponentFactory() = default;

    // Panel Factory Methods
    void registerPanel(const std::string& name, PanelFactoryFunction factory) override;
    std::unique_ptr<IPanel> createPanel(const std::string& name, 
                                       IGpioProvider* gpio, 
                                       IDisplayProvider* display) override;
    bool hasPanelRegistration(const std::string& name) const override;

    // Component Factory Methods
    void registerComponent(const std::string& name, ComponentFactoryFunction factory) override;
    std::unique_ptr<IComponent> createComponent(const std::string& name) override;
    bool hasComponentRegistration(const std::string& name) const override;

    // Utility Methods
    void clear() override;
    
    // Dependency Access Methods
    IStyleService* getStyleService() const override;
    IDisplayProvider* getDisplayProvider() const override;

private:
    IStyleService* styleService_;
    IDisplayProvider* displayProvider_;
    std::unordered_map<std::string, PanelFactoryFunction> panelFactories_;
    std::unordered_map<std::string, ComponentFactoryFunction> componentFactories_;
};