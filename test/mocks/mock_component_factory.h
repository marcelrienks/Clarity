#pragma once

// System/Library Includes
#include <functional>
#include <unordered_map>
#include <vector>
#include <string>
#include <memory>

// Project Includes
#include "interfaces/i_component_factory.h"
#include "interfaces/i_component.h"
#include "interfaces/i_panel.h"
#include "interfaces/i_display_provider.h"
#include "interfaces/i_gpio_provider.h"

// Forward Declarations
class MockComponent;
class MockPanel;

/**
 * @class MockComponentFactory
 * @brief Mock implementation of IComponentFactory for testing
 * 
 * @details Provides a testable implementation of the component factory interface
 * with controllable behavior for unit tests. Creates mock components and panels
 * instead of real implementations, allowing for isolated testing.
 * 
 * @testability Allows verification of factory registration and creation operations
 * @dependency_free No actual component dependency - uses mock implementations
 */
class MockComponentFactory : public IComponentFactory
{
public:
    struct CreationEvent {
        std::string type;
        std::string name;
        uint32_t timestamp;
        bool success;
    };

    MockComponentFactory();
    virtual ~MockComponentFactory() = default;

    // Panel Factory Methods
    void registerPanel(const std::string& name, PanelFactoryFunction factory) override;
    std::unique_ptr<IPanel> createPanel(const std::string& name, 
                                       IGpioProvider* gpio, 
                                       IDisplayProvider* display) override;
    bool hasPanelRegistration(const std::string& name) const override;

    // Component Factory Methods
    void registerComponent(const std::string& name, ComponentFactoryFunction factory) override;
    std::unique_ptr<IComponent> createComponent(const std::string& name, 
                                               IDisplayProvider* display,
                                               IStyleService* style) override;
    bool hasComponentRegistration(const std::string& name) const override;

    // Utility Methods
    void clear() override;

    // Test Helper Methods
    int getPanelRegistrationCount() const { return static_cast<int>(panelFactories_.size()); }
    int getComponentRegistrationCount() const { return static_cast<int>(componentFactories_.size()); }
    int getPanelCreationCount() const { return panelCreationCount_; }
    int getComponentCreationCount() const { return componentCreationCount_; }
    
    bool wasClearCalled() const { return clearCalled_; }

    // Test Control Methods
    void reset();
    void simulateCreationFailure(const std::string& name, bool shouldFail);
    void setReturnNullForUnknownTypes(bool returnNull) { returnNullForUnknownTypes_ = returnNull; }
    
    // Test Callbacks
    void setPanelCreationCallback(std::function<void(const std::string&)> callback) { panelCreationCallback_ = callback; }
    void setComponentCreationCallback(std::function<void(const std::string&)> callback) { componentCreationCallback_ = callback; }

    // Test Data Access
    const std::vector<CreationEvent>& getCreationHistory() const { return creationHistory_; }
    const std::vector<std::string>& getRegisteredPanelNames() const;
    const std::vector<std::string>& getRegisteredComponentNames() const;

    // Mock Creation Methods (for test setup)
    void registerMockPanel(const std::string& name);
    void registerMockComponent(const std::string& name);

private:
    // Factory Storage
    std::unordered_map<std::string, PanelFactoryFunction> panelFactories_;
    std::unordered_map<std::string, ComponentFactoryFunction> componentFactories_;

    // State Tracking
    int panelCreationCount_;
    int componentCreationCount_;
    bool clearCalled_;

    // Test Control
    std::unordered_map<std::string, bool> creationFailures_;
    bool returnNullForUnknownTypes_;

    // Test Data
    std::vector<CreationEvent> creationHistory_;
    mutable std::vector<std::string> registeredPanelNames_;
    mutable std::vector<std::string> registeredComponentNames_;

    // Test Callbacks
    std::function<void(const std::string&)> panelCreationCallback_;
    std::function<void(const std::string&)> componentCreationCallback_;

    void recordCreation(const std::string& type, const std::string& name, bool success);
    std::unique_ptr<MockPanel> createMockPanel(const std::string& name);
    std::unique_ptr<MockComponent> createMockComponent(const std::string& name);
};