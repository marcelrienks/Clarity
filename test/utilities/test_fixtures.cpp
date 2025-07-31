// Project Includes
#include "test_fixtures.h"

// ============================================================================
// BaseTestFixture Implementation
// ============================================================================

BaseTestFixture::BaseTestFixture()
    : container_(nullptr)
{
}

BaseTestFixture::~BaseTestFixture()
{
    tearDown();
}

void BaseTestFixture::setUp()
{
    container_ = std::make_unique<TestServiceContainer>();
}

void BaseTestFixture::tearDown()
{
    if (container_) {
        container_->reset();
        container_.reset();
    }
}

// ============================================================================
// ComponentTestFixture Implementation
// ============================================================================

ComponentTestFixture::ComponentTestFixture()
    : BaseTestFixture()
    , mockStyleService_(nullptr)
    , mockDisplayProvider_(nullptr)
{
}

void ComponentTestFixture::setUp()
{
    BaseTestFixture::setUp();
    
    // Create and register mock services
    auto styleService = std::make_unique<MockStyleService>();
    mockStyleService_ = styleService.get();
    container_->registerMock<IStyleService>(std::move(styleService));
    
    auto displayProvider = std::make_unique<MockDisplayProvider>();
    mockDisplayProvider_ = displayProvider.get();
    container_->registerMock<IDisplayProvider>(std::move(displayProvider));
    
    // Create builders
    oilComponentBuilder_ = std::make_unique<OilComponentTestBuilder>();
    oilComponentBuilder_->withMockStyle(std::make_unique<MockStyleService>());
    oilComponentBuilder_->withMockDisplay(std::make_unique<MockDisplayProvider>());
    
    componentFactoryBuilder_ = std::make_unique<ComponentFactoryTestBuilder>();
    componentFactoryBuilder_->withDefaultMocks();
}

// ============================================================================
// PanelTestFixture Implementation
// ============================================================================

PanelTestFixture::PanelTestFixture()
    : BaseTestFixture()
    , mockComponentFactory_(nullptr)
    , mockDisplayProvider_(nullptr)
    , mockGpioProvider_(nullptr)
{
}

void PanelTestFixture::setUp()
{
    BaseTestFixture::setUp();
    
    // Create and register mock services
    auto componentFactory = std::make_unique<MockComponentFactory>();
    mockComponentFactory_ = componentFactory.get();
    container_->registerMock<IComponentFactory>(std::move(componentFactory));
    
    auto displayProvider = std::make_unique<MockDisplayProvider>();
    mockDisplayProvider_ = displayProvider.get();
    container_->registerMock<IDisplayProvider>(std::move(displayProvider));
    
    auto gpioProvider = std::make_unique<MockGpioProvider>();
    mockGpioProvider_ = gpioProvider.get();
    container_->registerMock<IGpioProvider>(std::move(gpioProvider));
    
    // Create builder
    panelBuilder_ = std::make_unique<PanelTestBuilder>();
    panelBuilder_->withDefaultMocks();
}

// ============================================================================
// ServiceTestFixture Implementation
// ============================================================================

ServiceTestFixture::ServiceTestFixture()
    : BaseTestFixture()
    , mockStyleService_(nullptr)
    , mockPreferenceService_(nullptr)
    , mockTriggerService_(nullptr)
    , mockPanelService_(nullptr)
    , mockComponentFactory_(nullptr)
    , mockPanelFactory_(nullptr)
    , mockDisplayProvider_(nullptr)
    , mockGpioProvider_(nullptr)
{
}

void ServiceTestFixture::setUp()
{
    BaseTestFixture::setUp();
    
    // Register all service mocks
    auto styleService = std::make_unique<MockStyleService>();
    mockStyleService_ = styleService.get();
    container_->registerMock<IStyleService>(std::move(styleService));
    
    auto preferenceService = std::make_unique<MockPreferenceService>();
    mockPreferenceService_ = preferenceService.get();
    container_->registerMock<IPreferenceService>(std::move(preferenceService));
    
    auto triggerService = std::make_unique<MockTriggerService>();
    mockTriggerService_ = triggerService.get();
    container_->registerMock<ITriggerService>(std::move(triggerService));
    
    auto panelService = std::make_unique<MockPanelService>();
    mockPanelService_ = panelService.get();
    container_->registerMock<IPanelService>(std::move(panelService));
    
    auto componentFactory = std::make_unique<MockComponentFactory>();
    mockComponentFactory_ = componentFactory.get();
    container_->registerMock<IComponentFactory>(std::move(componentFactory));
    
    auto panelFactory = std::make_unique<MockPanelFactory>();
    mockPanelFactory_ = panelFactory.get();
    container_->registerMock<IPanelFactory>(std::move(panelFactory));
    
    auto displayProvider = std::make_unique<MockDisplayProvider>();
    mockDisplayProvider_ = displayProvider.get();
    container_->registerMock<IDisplayProvider>(std::move(displayProvider));
    
    auto gpioProvider = std::make_unique<MockGpioProvider>();
    mockGpioProvider_ = gpioProvider.get();
    container_->registerMock<IGpioProvider>(std::move(gpioProvider));
}

// ============================================================================
// IntegrationTestFixture Implementation
// ============================================================================

IntegrationTestFixture::IntegrationTestFixture()
    : BaseTestFixture()
    , mockStyleService_(nullptr)
    , mockPreferenceService_(nullptr)
    , mockTriggerService_(nullptr)
    , mockPanelService_(nullptr)
    , mockComponentFactory_(nullptr)
    , mockPanelFactory_(nullptr)
    , mockDisplayProvider_(nullptr)
    , mockGpioProvider_(nullptr)
{
}

void IntegrationTestFixture::setUp()
{
    BaseTestFixture::setUp();
    
    // Set up the same mock services as ServiceTestFixture
    // but with integration-focused configurations
    auto styleService = std::make_unique<MockStyleService>();
    mockStyleService_ = styleService.get();
    // Configure for realistic theme switching
    mockStyleService_->setCurrentTheme(Themes::DAY);
    container_->registerMock<IStyleService>(std::move(styleService));
    
    auto preferenceService = std::make_unique<MockPreferenceService>();
    mockPreferenceService_ = preferenceService.get();
    // Set up default configuration
    mockPreferenceService_->setDefaultConfig();
    container_->registerMock<IPreferenceService>(std::move(preferenceService));
    
    auto triggerService = std::make_unique<MockTriggerService>();
    mockTriggerService_ = triggerService.get();
    container_->registerMock<ITriggerService>(std::move(triggerService));
    
    auto panelService = std::make_unique<MockPanelService>();
    mockPanelService_ = panelService.get();
    // Start with splash panel as default
    mockPanelService_->setCurrentPanel("splash");
    container_->registerMock<IPanelService>(std::move(panelService));
    
    auto componentFactory = std::make_unique<MockComponentFactory>();
    mockComponentFactory_ = componentFactory.get();
    container_->registerMock<IComponentFactory>(std::move(componentFactory));
    
    auto panelFactory = std::make_unique<MockPanelFactory>();
    mockPanelFactory_ = panelFactory.get();
    container_->registerMock<IPanelFactory>(std::move(panelFactory));
    
    auto displayProvider = std::make_unique<MockDisplayProvider>();
    mockDisplayProvider_ = displayProvider.get();
    container_->registerMock<IDisplayProvider>(std::move(displayProvider));
    
    auto gpioProvider = std::make_unique<MockGpioProvider>();
    mockGpioProvider_ = gpioProvider.get();
    container_->registerMock<IGpioProvider>(std::move(gpioProvider));
}

void IntegrationTestFixture::simulateApplicationStartup()
{
    // Simulate the startup sequence
    mockPreferenceService_->loadConfig();
    mockStyleService_->init(mockPreferenceService_->getConfig().theme);
    mockTriggerService_->init();
    mockPanelService_->loadPanel("splash");
}

void IntegrationTestFixture::simulatePanelTransition(const std::string& fromPanel, const std::string& toPanel)
{
    // Simulate panel transition logic
    mockPanelService_->setCurrentPanel(fromPanel);
    mockTriggerService_->setStartupPanelOverride(toPanel);
    mockPanelService_->loadPanel(toPanel);
}

void IntegrationTestFixture::simulateUserInput(int gpio, bool state)
{
    // Simulate GPIO input
    mockGpioProvider_->setGpioState(gpio, state);
    mockTriggerService_->processTriggerEvents();
}