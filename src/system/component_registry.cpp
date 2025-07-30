#include "system/component_registry.h"

ComponentRegistry& ComponentRegistry::GetInstance() {
    static ComponentRegistry instance;
    return instance;
}

void ComponentRegistry::registerPanel(const std::string& name, PanelFactoryFunction factory) {
    panelFactories[name] = factory;
}

void ComponentRegistry::registerComponent(const std::string& name, ComponentFactoryFunction factory) {
    componentFactories[name] = factory;
}

std::unique_ptr<IPanel> ComponentRegistry::createPanel(const std::string& name, IGpioProvider* gpio, IDisplayProvider* display) {
    auto it = panelFactories.find(name);
    if (it != panelFactories.end()) {
        return it->second(gpio, display);
    }
    return nullptr;
}

std::unique_ptr<IComponent> ComponentRegistry::createComponent(const std::string& name, IDisplayProvider* display, IStyleService* style) {
    auto it = componentFactories.find(name);
    if (it != componentFactories.end()) {
        return it->second(display, style);
    }
    return nullptr;
}

bool ComponentRegistry::hasPanelRegistration(const std::string& name) const {
    return panelFactories.find(name) != panelFactories.end();
}

bool ComponentRegistry::hasComponentRegistration(const std::string& name) const {
    return componentFactories.find(name) != componentFactories.end();
}

void ComponentRegistry::clear() {
    panelFactories.clear();
    componentFactories.clear();
}