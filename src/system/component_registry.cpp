#include "system/component_registry.h"
#include "system/service_container.h"
#include "device.h"
#include "managers/style_manager.h"

void ComponentRegistry::registerPanel(const std::string& name, PanelFactoryFunction factory) {
    panelFactories[name] = factory;
}

void ComponentRegistry::registerComponent(const std::string& name, ComponentFactoryFunction factory) {
    componentFactories[name] = factory;
}

void ComponentRegistry::registerSensor(const std::string& name, SensorFactoryFunction factory) {
    sensorFactories[name] = factory;
}

std::unique_ptr<IPanel> ComponentRegistry::createPanel(const std::string& name, IGpioProvider* gpio, IDisplayProvider* display) {
    auto it = panelFactories.find(name);
    if (it != panelFactories.end()) {
        return it->second(gpio, display);
    }
    return nullptr;
}

std::unique_ptr<IComponent> ComponentRegistry::createComponent(const std::string& name) {
    auto it = componentFactories.find(name);
    if (it != componentFactories.end()) {
        // Use dependency injection through service container
        auto* display = serviceContainer_.resolve<IDisplayProvider>();
        auto* style = serviceContainer_.resolve<IStyleService>();
        
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

std::unique_ptr<ISensor> ComponentRegistry::createSensor(const std::string& name) {
    auto it = sensorFactories.find(name);
    if (it != sensorFactories.end()) {
        return it->second();
    }
    return nullptr;
}

bool ComponentRegistry::hasSensorRegistration(const std::string& name) const {
    return sensorFactories.find(name) != sensorFactories.end();
}

void ComponentRegistry::clear() {
    panelFactories.clear();
    componentFactories.clear();
    sensorFactories.clear();
}

IStyleService* ComponentRegistry::getStyleService() const {
    return serviceContainer_.resolve<IStyleService>();
}

IDisplayProvider* ComponentRegistry::getDisplayProvider() const {
    return serviceContainer_.resolve<IDisplayProvider>();
}