#include "factories/component_factory.h"

ComponentFactory::ComponentFactory(IStyleService* styleService, IDisplayProvider* displayProvider)
    : styleService_(styleService), displayProvider_(displayProvider)
{
}

void ComponentFactory::registerPanel(const std::string& name, PanelFactoryFunction factory)
{
    panelFactories_[name] = factory;
}

std::unique_ptr<IPanel> ComponentFactory::createPanel(const std::string& name, 
                                                     IGpioProvider* gpio, 
                                                     IDisplayProvider* display)
{
    auto it = panelFactories_.find(name);
    if (it != panelFactories_.end()) {
        return it->second(gpio, display);
    }
    return nullptr;
}

bool ComponentFactory::hasPanelRegistration(const std::string& name) const
{
    return panelFactories_.find(name) != panelFactories_.end();
}

void ComponentFactory::registerComponent(const std::string& name, ComponentFactoryFunction factory)
{
    componentFactories_[name] = factory;
}

std::unique_ptr<IComponent> ComponentFactory::createComponent(const std::string& name)
{
    auto it = componentFactories_.find(name);
    if (it != componentFactories_.end()) {
        return it->second(displayProvider_, styleService_);
    }
    return nullptr;
}

bool ComponentFactory::hasComponentRegistration(const std::string& name) const
{
    return componentFactories_.find(name) != componentFactories_.end();
}

void ComponentFactory::clear()
{
    panelFactories_.clear();
    componentFactories_.clear();
}

IStyleService* ComponentFactory::getStyleService() const
{
    return styleService_;
}

IDisplayProvider* ComponentFactory::getDisplayProvider() const
{
    return displayProvider_;
}