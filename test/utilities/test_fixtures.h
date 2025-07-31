#pragma once

// System/Library Includes
#include <memory>

// Project Includes
#include "test_service_container.h"
#include "test_builders.h"

// Mock includes  
#include "mocks/mock_style_service.h"
#include "mocks/mock_preference_service.h"
#include "mocks/mock_trigger_service.h"
#include "mocks/mock_panel_service.h"
#include "mocks/mock_component_factory.h"
#include "mocks/mock_panel_factory.h"
#include "providers/mock_display_provider.h"
#include "providers/mock_gpio_provider.h"

/**
 * @class BaseTestFixture
 * @brief Base test fixture providing common testing infrastructure
 * 
 * @details Provides a clean test environment with service container
 * and common utilities for all tests. Handles setup and teardown.
 */
class BaseTestFixture
{
public:
    BaseTestFixture();
    virtual ~BaseTestFixture();
    
    // Setup and teardown
    virtual void setUp();
    virtual void tearDown();
    
protected:
    std::unique_ptr<TestServiceContainer> container_;
};

/**
 * @class ComponentTestFixture
 * @brief Specialized fixture for component testing
 * 
 * @details Pre-configured with all mocks needed for component testing.
 * Provides easy access to common mock services and builders.
 */
class ComponentTestFixture : public BaseTestFixture
{
public:
    ComponentTestFixture();
    virtual ~ComponentTestFixture() = default;
    
    void setUp() override;
    
protected:
    // Pre-configured mocks
    MockStyleService* mockStyleService_;
    MockDisplayProvider* mockDisplayProvider_;
    
    // Builders for easy test object creation
    std::unique_ptr<OilComponentTestBuilder> oilComponentBuilder_;
    std::unique_ptr<ComponentFactoryTestBuilder> componentFactoryBuilder_;
};

/**
 * @class PanelTestFixture
 * @brief Specialized fixture for panel testing
 * 
 * @details Pre-configured with all mocks needed for panel testing.
 * Includes component factory, display provider, and GPIO provider mocks.
 */
class PanelTestFixture : public BaseTestFixture
{
public:
    PanelTestFixture();
    virtual ~PanelTestFixture() = default;
    
    void setUp() override;
    
protected:
    // Pre-configured mocks
    MockComponentFactory* mockComponentFactory_;
    MockDisplayProvider* mockDisplayProvider_;
    MockGpioProvider* mockGpioProvider_;
    
    // Builder for easy panel creation
    std::unique_ptr<PanelTestBuilder> panelBuilder_;
};

/**
 * @class ServiceTestFixture
 * @brief Specialized fixture for service and manager testing
 * 
 * @details Pre-configured with all service mocks for testing
 * service interactions and manager behavior.
 */
class ServiceTestFixture : public BaseTestFixture
{
public:
    ServiceTestFixture();
    virtual ~ServiceTestFixture() = default;
    
    void setUp() override;
    
protected:
    // Service mocks
    MockStyleService* mockStyleService_;
    MockPreferenceService* mockPreferenceService_;
    MockTriggerService* mockTriggerService_;
    MockPanelService* mockPanelService_;
    MockComponentFactory* mockComponentFactory_;
    MockPanelFactory* mockPanelFactory_;
    
    // Provider mocks
    MockDisplayProvider* mockDisplayProvider_;
    MockGpioProvider* mockGpioProvider_;
};

/**
 * @class IntegrationTestFixture
 * @brief Fixture for integration testing with full service setup
 * 
 * @details Provides a complete testing environment that mimics
 * the production service container setup but with controllable mocks.
 */
class IntegrationTestFixture : public BaseTestFixture
{
public:
    IntegrationTestFixture();
    virtual ~IntegrationTestFixture() = default;
    
    void setUp() override;
    
    // Helper methods for integration testing
    void simulateApplicationStartup();
    void simulatePanelTransition(const std::string& fromPanel, const std::string& toPanel);
    void simulateUserInput(int gpio, bool state);
    
protected:
    // Full service mock setup
    MockStyleService* mockStyleService_;
    MockPreferenceService* mockPreferenceService_;
    MockTriggerService* mockTriggerService_;
    MockPanelService* mockPanelService_;
    MockComponentFactory* mockComponentFactory_;
    MockPanelFactory* mockPanelFactory_;
    MockDisplayProvider* mockDisplayProvider_;
    MockGpioProvider* mockGpioProvider_;
};