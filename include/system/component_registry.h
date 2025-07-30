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

class ComponentRegistry {
public:
    using PanelFactory = std::function<std::unique_ptr<IPanel>(IGpioProvider*, IDisplayProvider*)>;
    using ComponentFactory = std::function<std::unique_ptr<IComponent>(IDisplayProvider*, IStyleService*)>;

    static ComponentRegistry& GetInstance();

    void registerPanel(const std::string& name, PanelFactory factory);
    void registerComponent(const std::string& name, ComponentFactory factory);

    std::unique_ptr<IPanel> createPanel(const std::string& name, IGpioProvider* gpio, IDisplayProvider* display);
    std::unique_ptr<IComponent> createComponent(const std::string& name, IDisplayProvider* display, IStyleService* style);

    bool hasPanelRegistration(const std::string& name) const;
    bool hasComponentRegistration(const std::string& name) const;

    void clear();

private:
    std::unordered_map<std::string, PanelFactory> panelFactories;
    std::unordered_map<std::string, ComponentFactory> componentFactories;
};