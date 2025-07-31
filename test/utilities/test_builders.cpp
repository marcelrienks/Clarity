// Project Includes
#include "test_builders.h"
#include "factories/component_factory.h"

// ============================================================================
// OilComponentTestBuilder Implementation
// ============================================================================

OilComponentTestBuilder::OilComponentTestBuilder()
    : hasStyleService_(false)
    , hasDisplayProvider_(false)
{
}

OilComponentTestBuilder& OilComponentTestBuilder::withMockStyle(std::unique_ptr<MockStyleService> mock)
{
    container_.registerMock<IStyleService>(std::move(mock));
    hasStyleService_ = true;
    return *this;
}

OilComponentTestBuilder& OilComponentTestBuilder::withMockDisplay(std::unique_ptr<MockDisplayProvider> mock)
{
    container_.registerMock<IDisplayProvider>(std::move(mock));
    hasDisplayProvider_ = true;
    return *this;
}

OilComponentTestBuilder& OilComponentTestBuilder::withDefaultMocks()
{
    ensureDefaultMocks();
    return *this;
}

std::unique_ptr<OemOilPressureComponent> OilComponentTestBuilder::buildPressureComponent()
{
    ensureDefaultMocks();
    auto styleService = container_.resolve<IStyleService>();
    return std::make_unique<OemOilPressureComponent>(styleService);
}

std::unique_ptr<OemOilTemperatureComponent> OilComponentTestBuilder::buildTemperatureComponent()
{
    ensureDefaultMocks();
    auto styleService = container_.resolve<IStyleService>();
    return std::make_unique<OemOilTemperatureComponent>(styleService);
}

void OilComponentTestBuilder::ensureDefaultMocks()
{
    if (!hasStyleService_) {
        container_.registerMock<IStyleService>(std::make_unique<MockStyleService>());
        hasStyleService_ = true;
    }
    
    if (!hasDisplayProvider_) {
        container_.registerMock<IDisplayProvider>(std::make_unique<MockDisplayProvider>());
        hasDisplayProvider_ = true;
    }
}

// ============================================================================
// PanelTestBuilder Implementation
// ============================================================================

PanelTestBuilder::PanelTestBuilder()
    : hasComponentFactory_(false)
    , hasDisplayProvider_(false)
    , hasGpioProvider_(false)
{
}

PanelTestBuilder& PanelTestBuilder::withMockComponentFactory(std::unique_ptr<MockComponentFactory> mock)
{
    container_.registerMock<IComponentFactory>(std::move(mock));
    hasComponentFactory_ = true;
    return *this;
}

PanelTestBuilder& PanelTestBuilder::withMockPanelFactory(std::unique_ptr<MockPanelFactory> mock)
{
    container_.registerMock<IPanelFactory>(std::move(mock));
    return *this;
}

PanelTestBuilder& PanelTestBuilder::withMockDisplay(std::unique_ptr<MockDisplayProvider> mock)
{
    container_.registerMock<IDisplayProvider>(std::move(mock));
    hasDisplayProvider_ = true;
    return *this;
}

PanelTestBuilder& PanelTestBuilder::withMockGpio(std::unique_ptr<MockGpioProvider> mock)
{
    container_.registerMock<IGpioProvider>(std::move(mock));
    hasGpioProvider_ = true;
    return *this;
}

PanelTestBuilder& PanelTestBuilder::withDefaultMocks()
{
    ensureDefaultMocks();
    return *this;
}

std::unique_ptr<OemOilPanel> PanelTestBuilder::buildOilPanel()
{
    ensureDefaultMocks();
    auto componentFactory = container_.resolve<IComponentFactory>();
    auto displayProvider = container_.resolve<IDisplayProvider>();
    auto gpioProvider = container_.resolve<IGpioProvider>();
    
    return std::make_unique<OemOilPanel>(componentFactory, displayProvider, gpioProvider);
}

std::unique_ptr<KeyPanel> PanelTestBuilder::buildKeyPanel()
{
    ensureDefaultMocks();
    auto componentFactory = container_.resolve<IComponentFactory>();
    auto displayProvider = container_.resolve<IDisplayProvider>();
    auto gpioProvider = container_.resolve<IGpioProvider>();
    
    return std::make_unique<KeyPanel>(componentFactory, displayProvider, gpioProvider);
}

std::unique_ptr<LockPanel> PanelTestBuilder::buildLockPanel()
{
    ensureDefaultMocks();
    auto componentFactory = container_.resolve<IComponentFactory>();
    auto displayProvider = container_.resolve<IDisplayProvider>();
    auto gpioProvider = container_.resolve<IGpioProvider>();
    
    return std::make_unique<LockPanel>(componentFactory, displayProvider, gpioProvider);
}

std::unique_ptr<SplashPanel> PanelTestBuilder::buildSplashPanel()
{
    ensureDefaultMocks();
    auto componentFactory = container_.resolve<IComponentFactory>();
    auto displayProvider = container_.resolve<IDisplayProvider>();
    
    return std::make_unique<SplashPanel>(componentFactory, displayProvider);
}

void PanelTestBuilder::ensureDefaultMocks()
{
    if (!hasComponentFactory_) {
        container_.registerMock<IComponentFactory>(std::make_unique<MockComponentFactory>());
        hasComponentFactory_ = true;
    }
    
    if (!hasDisplayProvider_) {
        container_.registerMock<IDisplayProvider>(std::make_unique<MockDisplayProvider>());
        hasDisplayProvider_ = true;
    }
    
    if (!hasGpioProvider_) {
        container_.registerMock<IGpioProvider>(std::make_unique<MockGpioProvider>());
        hasGpioProvider_ = true;
    }
}

// ============================================================================
// ComponentFactoryTestBuilder Implementation
// ============================================================================

ComponentFactoryTestBuilder::ComponentFactoryTestBuilder()
{
}

ComponentFactoryTestBuilder& ComponentFactoryTestBuilder::withMockStyle(std::unique_ptr<MockStyleService> mock)
{
    container_.registerMock<IStyleService>(std::move(mock));
    return *this;
}

ComponentFactoryTestBuilder& ComponentFactoryTestBuilder::withMockDisplay(std::unique_ptr<MockDisplayProvider> mock)
{
    container_.registerMock<IDisplayProvider>(std::move(mock));
    return *this;
}

ComponentFactoryTestBuilder& ComponentFactoryTestBuilder::withDefaultMocks()
{
    ensureDefaultMocks();
    return *this;
}

std::unique_ptr<ComponentFactory> ComponentFactoryTestBuilder::build()
{
    ensureDefaultMocks();
    auto styleService = container_.resolve<IStyleService>();
    auto displayProvider = container_.resolve<IDisplayProvider>();
    
    return std::make_unique<ComponentFactory>(styleService, displayProvider);
}

void ComponentFactoryTestBuilder::ensureDefaultMocks()
{
    if (!container_.isRegistered<IStyleService>()) {
        container_.registerMock<IStyleService>(std::make_unique<MockStyleService>());
    }
    
    if (!container_.isRegistered<IDisplayProvider>()) {
        container_.registerMock<IDisplayProvider>(std::make_unique<MockDisplayProvider>());
    }
}