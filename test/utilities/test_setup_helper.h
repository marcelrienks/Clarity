#pragma once

#include "system/service_container.h"
#include "interfaces/i_component_factory.h"
#include "interfaces/i_sensor_factory.h"
#include "interfaces/i_display_provider.h"
#include "interfaces/i_gpio_provider.h"
#include "interfaces/i_style_service.h"
#include "interfaces/i_preference_service.h"
#include "mocks/mock_component_factory.h"
#include "mocks/mock_sensor_factory.h"

namespace ArchitecturalTestHelpers {

class TestSetupHelper {
public:
    static std::unique_ptr<ServiceContainer> CreateTestContainer() {
        auto container = std::make_unique<ServiceContainer>();

        // Register core services
        container->registerSingleton<IComponentFactory>(std::make_unique<MockComponentFactory>());
        container->registerSingleton<ISensorFactory>(std::make_unique<MockSensorFactory>());
        container->registerSingleton<IDisplayProvider>(std::make_unique<TestDisplayProvider>());
        container->registerSingleton<IGpioProvider>(std::make_unique<TestGpioProvider>());
        container->registerSingleton<IStyleService>(std::make_unique<TestStyleService>());
        container->registerSingleton<IPreferenceService>(std::make_unique<TestPreferenceService>());

        return container;
    }
};

} // namespace ArchitecturalTestHelpers
