#pragma once

// System/Library Includes
#include <memory>

// Project Includes
#include "test_service_container.h"
#include "components/oem/oem_oil_component.h"
#include "components/oem/oem_oil_pressure_component.h"
#include "components/oem/oem_oil_temperature_component.h"
#include "components/key_component.h"
#include "components/lock_component.h"
#include "panels/oem_oil_panel.h"
#include "panels/key_panel.h"
#include "panels/lock_panel.h"
#include "panels/splash_panel.h"

// Mock includes
#include "mocks/mock_style_service.h"
#include "mocks/mock_component_factory.h"
#include "mocks/mock_panel_factory.h"
#include "providers/mock_display_provider.h"
#include "providers/mock_gpio_provider.h"

/**
 * @class OilComponentTestBuilder
 * @brief Builder pattern for creating oil components with configurable dependencies
 * 
 * @details Simplifies test setup by providing a fluent interface for configuring
 * oil component dependencies. Supports both pressure and temperature components
 * with default mock implementations that can be customized per test.
 */
class OilComponentTestBuilder
{
public:
    OilComponentTestBuilder();
    
    // Fluent interface for dependency configuration
    OilComponentTestBuilder& withMockStyle(std::unique_ptr<MockStyleService> mock);
    OilComponentTestBuilder& withMockDisplay(std::unique_ptr<MockDisplayProvider> mock);
    OilComponentTestBuilder& withDefaultMocks();
    
    // Component creation methods
    std::unique_ptr<OemOilPressureComponent> buildPressureComponent();
    std::unique_ptr<OemOilTemperatureComponent> buildTemperatureComponent();
    
    // Access to configured container for advanced testing
    TestServiceContainer& getContainer() { return container_; }

private:
    TestServiceContainer container_;
    bool hasStyleService_;
    bool hasDisplayProvider_;
    
    void ensureDefaultMocks();
};

/**
 * @class PanelTestBuilder  
 * @brief Builder pattern for creating panels with configurable dependencies
 * 
 * @details Provides a fluent interface for setting up panel tests with
 * all required dependencies properly mocked and configured.
 */
class PanelTestBuilder
{
public:
    PanelTestBuilder();
    
    // Fluent interface for dependency configuration
    PanelTestBuilder& withMockComponentFactory(std::unique_ptr<MockComponentFactory> mock);
    PanelTestBuilder& withMockPanelFactory(std::unique_ptr<MockPanelFactory> mock);
    PanelTestBuilder& withMockDisplay(std::unique_ptr<MockDisplayProvider> mock);
    PanelTestBuilder& withMockGpio(std::unique_ptr<MockGpioProvider> mock);
    PanelTestBuilder& withDefaultMocks();
    
    // Panel creation methods
    std::unique_ptr<OemOilPanel> buildOilPanel();
    std::unique_ptr<KeyPanel> buildKeyPanel();
    std::unique_ptr<LockPanel> buildLockPanel();
    std::unique_ptr<SplashPanel> buildSplashPanel();
    
    // Access to configured container for advanced testing
    TestServiceContainer& getContainer() { return container_; }

private:
    TestServiceContainer container_;
    bool hasComponentFactory_;
    bool hasDisplayProvider_;
    bool hasGpioProvider_;
    
    void ensureDefaultMocks();
};

/**
 * @class ComponentFactoryTestBuilder
 * @brief Builder for testing component factory with various configurations
 */
class ComponentFactoryTestBuilder
{
public:
    ComponentFactoryTestBuilder();
    
    ComponentFactoryTestBuilder& withMockStyle(std::unique_ptr<MockStyleService> mock);
    ComponentFactoryTestBuilder& withMockDisplay(std::unique_ptr<MockDisplayProvider> mock);
    ComponentFactoryTestBuilder& withDefaultMocks();
    
    std::unique_ptr<ComponentFactory> build();
    TestServiceContainer& getContainer() { return container_; }

private:
    TestServiceContainer container_;
    void ensureDefaultMocks();
};