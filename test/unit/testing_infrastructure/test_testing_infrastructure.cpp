// Testing Framework Includes
#include <unity.h>

// Project Includes
#include "utilities/test_service_container.h"
#include "utilities/test_builders.h"
#include "utilities/test_fixtures.h"

// Mock Includes
#include "mocks/mock_style_service.h"
#include "mocks/mock_component_factory.h"
#include "mocks/mock_panel_factory.h"

// Component Includes
#include "components/oem/oem_oil_pressure_component.h"
#include "panels/oem_oil_panel.h"

// ============================================================================
// Test Service Container Tests
// ============================================================================

void test_service_container_registration_and_resolution()
{
    TestServiceContainer container;
    
    // Register a mock service
    auto mockStyle = std::make_unique<MockStyleService>();
    MockStyleService* mockStylePtr = mockStyle.get();
    container.registerMock<IStyleService>(std::move(mockStyle));
    
    // Verify resolution
    auto resolvedStyle = container.resolve<IStyleService>();
    TEST_ASSERT_NOT_NULL(resolvedStyle);
    TEST_ASSERT_EQUAL_PTR(mockStylePtr, resolvedStyle);
    
    // Verify service is registered
    TEST_ASSERT_TRUE(container.isRegistered<IStyleService>());
}

void test_service_container_reset()
{
    TestServiceContainer container;
    
    // Register service
    container.registerMock<IStyleService>(std::make_unique<MockStyleService>());
    TEST_ASSERT_TRUE(container.isRegistered<IStyleService>());
    
    // Reset container
    container.reset();
    
    // Verify service is cleared
    TEST_ASSERT_FALSE(container.isRegistered<IStyleService>());
}

// ============================================================================
// Test Builder Tests
// ============================================================================

void test_oil_component_builder_with_default_mocks()
{
    OilComponentTestBuilder builder;
    
    auto pressureComponent = builder.withDefaultMocks().buildPressureComponent();
    TEST_ASSERT_NOT_NULL(pressureComponent.get());
    
    auto tempComponent = builder.buildTemperatureComponent();
    TEST_ASSERT_NOT_NULL(tempComponent.get());
}

void test_oil_component_builder_with_custom_mocks()
{
    OilComponentTestBuilder builder;
    
    auto mockStyle = std::make_unique<MockStyleService>();
    mockStyle->setCurrentTheme(Themes::NIGHT);
    
    auto component = builder.withMockStyle(std::move(mockStyle))
                           .withDefaultMocks()
                           .buildPressureComponent();
    
    TEST_ASSERT_NOT_NULL(component.get());
}

void test_panel_builder_creates_all_panel_types()
{
    PanelTestBuilder builder;
    builder.withDefaultMocks();
    
    auto oilPanel = builder.buildOilPanel();
    TEST_ASSERT_NOT_NULL(oilPanel.get());
    
    auto keyPanel = builder.buildKeyPanel();
    TEST_ASSERT_NOT_NULL(keyPanel.get());
    
    auto lockPanel = builder.buildLockPanel();
    TEST_ASSERT_NOT_NULL(lockPanel.get());
    
    auto splashPanel = builder.buildSplashPanel();
    TEST_ASSERT_NOT_NULL(splashPanel.get());
}

void test_component_factory_builder()
{
    ComponentFactoryTestBuilder builder;
    
    auto factory = builder.withDefaultMocks().build();
    TEST_ASSERT_NOT_NULL(factory.get());
    
    // Test that factory can create components
    auto oilComponent = factory->createComponent(ComponentType::OilPressure);
    TEST_ASSERT_NOT_NULL(oilComponent.get());
}

// ============================================================================
// Test Fixture Tests
// ============================================================================

void test_component_test_fixture()
{
    ComponentTestFixture fixture;
    fixture.setUp();
    
    // Verify mocks are available
    TEST_ASSERT_NOT_NULL(fixture.mockStyleService_);
    TEST_ASSERT_NOT_NULL(fixture.mockDisplayProvider_);
    
    // Test that fixture can create components via builder
    auto component = fixture.oilComponentBuilder_->buildPressureComponent();
    TEST_ASSERT_NOT_NULL(component.get());
    
    fixture.tearDown();
}

void test_panel_test_fixture()
{
    PanelTestFixture fixture;
    fixture.setUp();
    
    // Verify mocks are available
    TEST_ASSERT_NOT_NULL(fixture.mockComponentFactory_);
    TEST_ASSERT_NOT_NULL(fixture.mockDisplayProvider_);
    TEST_ASSERT_NOT_NULL(fixture.mockGpioProvider_);
    
    // Test that fixture can create panels via builder
    auto panel = fixture.panelBuilder_->buildOilPanel();
    TEST_ASSERT_NOT_NULL(panel.get());
    
    fixture.tearDown();
}

void test_service_test_fixture_all_services()
{
    ServiceTestFixture fixture;
    fixture.setUp();
    
    // Verify all service mocks are available
    TEST_ASSERT_NOT_NULL(fixture.mockStyleService_);
    TEST_ASSERT_NOT_NULL(fixture.mockPreferenceService_);
    TEST_ASSERT_NOT_NULL(fixture.mockTriggerService_);
    TEST_ASSERT_NOT_NULL(fixture.mockPanelService_);
    TEST_ASSERT_NOT_NULL(fixture.mockComponentFactory_);
    TEST_ASSERT_NOT_NULL(fixture.mockPanelFactory_);
    TEST_ASSERT_NOT_NULL(fixture.mockDisplayProvider_);
    TEST_ASSERT_NOT_NULL(fixture.mockGpioProvider_);
    
    fixture.tearDown();
}

void test_integration_test_fixture_simulation()
{
    IntegrationTestFixture fixture;
    fixture.setUp();
    
    // Test startup simulation
    fixture.simulateApplicationStartup();
    
    // Verify startup sequence was called on mocks
    TEST_ASSERT_EQUAL(1, fixture.mockPreferenceService_->getLoadConfigCallCount());
    TEST_ASSERT_EQUAL(1, fixture.mockStyleService_->getInitCallCount());
    TEST_ASSERT_EQUAL(1, fixture.mockTriggerService_->getInitCallCount());
    TEST_ASSERT_EQUAL(1, fixture.mockPanelService_->getLoadPanelCallCount());
    
    // Test panel transition simulation
    fixture.simulatePanelTransition("splash", "oil");
    TEST_ASSERT_EQUAL_STRING("oil", fixture.mockPanelService_->getCurrentPanel().c_str());
    
    // Test user input simulation
    fixture.simulateUserInput(GPIO_NUM_0, true);
    TEST_ASSERT_EQUAL(1, fixture.mockTriggerService_->getProcessTriggerEventsCallCount());
    
    fixture.tearDown();
}

// ============================================================================
// Mock Factory Tests
// ============================================================================

void test_mock_panel_factory_creation()
{
    MockPanelFactory factory;
    
    // Test default panel creation
    auto panel = factory.createPanel("oil");
    TEST_ASSERT_NOT_NULL(panel.get());
    
    // Test call tracking
    TEST_ASSERT_EQUAL(1, factory.getCreatePanelCallCount());
    TEST_ASSERT_EQUAL_STRING("oil", factory.getLastRequestedPanelType().c_str());
}

void test_mock_panel_factory_support_checking()
{
    MockPanelFactory factory;
    
    // Test default supported panels
    TEST_ASSERT_TRUE(factory.supportsPanel("oil"));
    TEST_ASSERT_TRUE(factory.supportsPanel("key"));
    TEST_ASSERT_TRUE(factory.supportsPanel("lock"));
    TEST_ASSERT_TRUE(factory.supportsPanel("splash"));
    TEST_ASSERT_FALSE(factory.supportsPanel("unknown"));
    
    // Test call tracking
    TEST_ASSERT_EQUAL(5, factory.getSupportsPanelCallCount());
}

void test_mock_panel_factory_custom_behavior()
{
    MockPanelFactory factory;
    
    // Configure custom creator
    bool customCreatorCalled = false;
    factory.setCreatePanelBehavior("custom", [&customCreatorCalled]() {
        customCreatorCalled = true;
        return std::make_unique<MockPanel>();
    });
    
    factory.setSupportedPanel("custom", true);
    
    // Test custom creation
    TEST_ASSERT_TRUE(factory.supportsPanel("custom"));
    auto panel = factory.createPanel("custom");
    TEST_ASSERT_NOT_NULL(panel.get());
    TEST_ASSERT_TRUE(customCreatorCalled);
}

// ============================================================================
// Test Runner Setup
// ============================================================================

void runTestingInfrastructureTests()
{
    UNITY_BEGIN();
    
    // Service Container Tests
    RUN_TEST(test_service_container_registration_and_resolution);
    RUN_TEST(test_service_container_reset);
    
    // Builder Tests
    RUN_TEST(test_oil_component_builder_with_default_mocks);
    RUN_TEST(test_oil_component_builder_with_custom_mocks);
    RUN_TEST(test_panel_builder_creates_all_panel_types);
    RUN_TEST(test_component_factory_builder);
    
    // Fixture Tests
    RUN_TEST(test_component_test_fixture);
    RUN_TEST(test_panel_test_fixture);
    RUN_TEST(test_service_test_fixture_all_services);
    RUN_TEST(test_integration_test_fixture_simulation);
    
    // Mock Factory Tests
    RUN_TEST(test_mock_panel_factory_creation);
    RUN_TEST(test_mock_panel_factory_support_checking);
    RUN_TEST(test_mock_panel_factory_custom_behavior);
    
    UNITY_END();
}