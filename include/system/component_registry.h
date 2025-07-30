#pragma once

#include <unordered_map>
#include <memory>
#include <string>
#include <functional>
#include "interfaces/i_panel.h"
#include "interfaces/i_component.h"
#include "interfaces/i_display_provider.h"
#include "interfaces/i_gpio_provider.h"
#include "interfaces/i_style_service.h"
#include "interfaces/i_component_factory.h"

class ComponentRegistry : public IComponentFactory {
public:
    static ComponentRegistry& GetInstance();

    // IComponentFactory interface implementation
    void registerPanel(const std::string& name, PanelFactoryFunction factory) override;
    void registerComponent(const std::string& name, ComponentFactoryFunction factory) override;

    std::unique_ptr<IPanel> createPanel(const std::string& name, IGpioProvider* gpio, IDisplayProvider* display) override;
    std::unique_ptr<IComponent> createComponent(const std::string& name) override;

    bool hasPanelRegistration(const std::string& name) const override;
    bool hasComponentRegistration(const std::string& name) const override;

    void clear() override;

private:
    std::unordered_map<std::string, PanelFactoryFunction> panelFactories;
    std::unordered_map<std::string, ComponentFactoryFunction> componentFactories;
};