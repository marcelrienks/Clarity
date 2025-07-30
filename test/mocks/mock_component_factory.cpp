#include "mock_component_factory.h"
#include "mock_component.h"
#include "mock_panel.h"
#include <chrono>

MockComponentFactory::MockComponentFactory()
    : panelCreationCount_(0)
    , componentCreationCount_(0)
    , clearCalled_(false)
    , returnNullForUnknownTypes_(true)
{
}

void MockComponentFactory::registerPanel(const std::string& name, PanelFactoryFunction factory)
{
    panelFactories_[name] = factory;
}

std::unique_ptr<IPanel> MockComponentFactory::createPanel(const std::string& name, 
                                                         IGpioProvider* gpio, 
                                                         IDisplayProvider* display)
{
    panelCreationCount_++;
    bool success = false;
    
    if (panelCreationCallback_) {
        panelCreationCallback_(name);
    }
    
    // Check for simulated creation failure
    auto failureIt = creationFailures_.find(name);
    if (failureIt != creationFailures_.end() && failureIt->second) {
        recordCreation("panel", name, false);
        return nullptr;
    }
    
    // Try to create from registered factory
    auto factoryIt = panelFactories_.find(name);
    if (factoryIt != panelFactories_.end()) {
        success = true;
        recordCreation("panel", name, true);
        return factoryIt->second(gpio, display);
    }
    
    // Create mock panel if factory not found and we're not returning null
    if (!returnNullForUnknownTypes_) {
        success = true;
        recordCreation("panel", name, true);
        return createMockPanel(name);
    }
    
    recordCreation("panel", name, false);
    return nullptr;
}

bool MockComponentFactory::hasPanelRegistration(const std::string& name) const
{
    return panelFactories_.find(name) != panelFactories_.end();
}

void MockComponentFactory::registerComponent(const std::string& name, ComponentFactoryFunction factory)
{
    componentFactories_[name] = factory;
}

std::unique_ptr<IComponent> MockComponentFactory::createComponent(const std::string& name)
{
    componentCreationCount_++;
    bool success = false;
    
    if (componentCreationCallback_) {
        componentCreationCallback_(name);
    }
    
    // Check for simulated creation failure
    auto failureIt = creationFailures_.find(name);
    if (failureIt != creationFailures_.end() && failureIt->second) {
        recordCreation("component", name, false);
        return nullptr;
    }
    
    // Try to create from registered factory
    auto factoryIt = componentFactories_.find(name);
    if (factoryIt != componentFactories_.end()) {
        success = true;
        recordCreation("component", name, true);
        // Mock factory: pass null for dependencies during transition
        return factoryIt->second(nullptr, nullptr);
    }
    
    // Create mock component if factory not found and we're not returning null
    if (!returnNullForUnknownTypes_) {
        success = true;
        recordCreation("component", name, true);
        return createMockComponent(name);
    }
    
    recordCreation("component", name, false);
    return nullptr;
}

bool MockComponentFactory::hasComponentRegistration(const std::string& name) const
{
    return componentFactories_.find(name) != componentFactories_.end();
}

void MockComponentFactory::clear()
{
    clearCalled_ = true;
    panelFactories_.clear();
    componentFactories_.clear();
}

void MockComponentFactory::reset()
{
    panelCreationCount_ = 0;
    componentCreationCount_ = 0;
    clearCalled_ = false;
    returnNullForUnknownTypes_ = true;
    
    panelFactories_.clear();
    componentFactories_.clear();
    creationFailures_.clear();
    creationHistory_.clear();
    registeredPanelNames_.clear();
    registeredComponentNames_.clear();
    
    panelCreationCallback_ = nullptr;
    componentCreationCallback_ = nullptr;
}

void MockComponentFactory::simulateCreationFailure(const std::string& name, bool shouldFail)
{
    creationFailures_[name] = shouldFail;
}

const std::vector<std::string>& MockComponentFactory::getRegisteredPanelNames() const
{
    registeredPanelNames_.clear();
    for (const auto& pair : panelFactories_) {
        registeredPanelNames_.push_back(pair.first);
    }
    return registeredPanelNames_;
}

const std::vector<std::string>& MockComponentFactory::getRegisteredComponentNames() const
{
    registeredComponentNames_.clear();
    for (const auto& pair : componentFactories_) {
        registeredComponentNames_.push_back(pair.first);
    }
    return registeredComponentNames_;
}

void MockComponentFactory::registerMockPanel(const std::string& name)
{
    registerPanel(name, [name](IGpioProvider* gpio, IDisplayProvider* display) -> std::unique_ptr<IPanel> {
        auto panel = std::make_unique<MockPanel>(name);
        panel->init(gpio, display);
        return std::move(panel);
    });
}

void MockComponentFactory::registerMockComponent(const std::string& name)
{
    registerComponent(name, [name](IDisplayProvider* display, IStyleService* style) -> std::unique_ptr<IComponent> {
        return std::make_unique<MockComponent>(name);
    });
}

void MockComponentFactory::recordCreation(const std::string& type, const std::string& name, bool success)
{
    CreationEvent event;
    event.type = type;
    event.name = name;
    event.success = success;
    event.timestamp = static_cast<uint32_t>(std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count());
    
    creationHistory_.push_back(event);
}

std::unique_ptr<MockPanel> MockComponentFactory::createMockPanel(const std::string& name)
{
    return std::make_unique<MockPanel>(name);
}

std::unique_ptr<MockComponent> MockComponentFactory::createMockComponent(const std::string& name)
{
    return std::make_unique<MockComponent>(name);
}