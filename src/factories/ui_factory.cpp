#include "factories/ui_factory.h"
#include "components/key_component.h"
#include "components/lock_component.h"
#include "components/clarity_component.h"
#include "components/oem/oem_oil_pressure_component.h"
#include "components/oem/oem_oil_temperature_component.h"
#include "components/error_list_component.h"
#include "panels/key_panel.h"
#include "panels/lock_panel.h"
#include "panels/splash_panel.h"
#include "panels/oem_oil_panel.h"
#include "panels/error_panel.h"
#include "panels/config_panel.h"

std::unique_ptr<IComponent> UIFactory::createKeyComponent(IStyleService *styleService) {
    return std::make_unique<KeyComponent>(styleService);
}

std::unique_ptr<IComponent> UIFactory::createLockComponent(IStyleService *styleService) {
    return std::make_unique<LockComponent>(styleService);
}

std::unique_ptr<IComponent> UIFactory::createClarityComponent(IStyleService *styleService) {
    return std::make_unique<ClarityComponent>(styleService);
}

std::unique_ptr<IComponent> UIFactory::createOemOilPressureComponent(IStyleService *styleService) {
    return std::make_unique<OemOilPressureComponent>(styleService);
}

std::unique_ptr<IComponent> UIFactory::createOemOilTemperatureComponent(IStyleService *styleService) {
    return std::make_unique<OemOilTemperatureComponent>(styleService);
}

std::unique_ptr<IComponent> UIFactory::createErrorListComponent(IStyleService *styleService) {
    return std::make_unique<ErrorListComponent>(styleService);
}

std::unique_ptr<IPanel> UIFactory::createKeyPanel(IGpioProvider *gpio, IDisplayProvider *display, IStyleService *styleService) {
    return std::make_unique<KeyPanel>(gpio, display, styleService);
}

std::unique_ptr<IPanel> UIFactory::createLockPanel(IGpioProvider *gpio, IDisplayProvider *display, IStyleService *styleService) {
    return std::make_unique<LockPanel>(gpio, display, styleService);
}

std::unique_ptr<IPanel> UIFactory::createSplashPanel(IGpioProvider *gpio, IDisplayProvider *display, IStyleService *styleService) {
    return std::make_unique<SplashPanel>(gpio, display, styleService);
}

std::unique_ptr<IPanel> UIFactory::createOemOilPanel(IGpioProvider *gpio, IDisplayProvider *display, IStyleService *styleService) {
    return std::make_unique<OemOilPanel>(gpio, display, styleService);
}

std::unique_ptr<IPanel> UIFactory::createErrorPanel(IGpioProvider *gpio, IDisplayProvider *display, IStyleService *styleService) {
    return std::make_unique<ErrorPanel>(gpio, display, styleService);
}

std::unique_ptr<IPanel> UIFactory::createConfigPanel(IGpioProvider *gpio, IDisplayProvider *display, IStyleService *styleService) {
    return std::make_unique<ConfigPanel>(gpio, display, styleService);
}