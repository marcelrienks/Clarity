#pragma once

#include <memory>
#include "interfaces/i_component.h"
#include "interfaces/i_panel.h"
#include "interfaces/i_display_provider.h"
#include "interfaces/i_gpio_provider.h"
#include "interfaces/i_style_service.h"

class UIFactory {
public:
    // Component creation methods
    static std::unique_ptr<IComponent> createKeyComponent(IStyleService* styleService);
    static std::unique_ptr<IComponent> createLockComponent(IStyleService* styleService);
    static std::unique_ptr<IComponent> createClarityComponent(IStyleService* styleService);
    static std::unique_ptr<IComponent> createOemOilPressureComponent(IStyleService* styleService);
    static std::unique_ptr<IComponent> createOemOilTemperatureComponent(IStyleService* styleService);
    
    // Panel creation methods
    static std::unique_ptr<IPanel> createKeyPanel(IGpioProvider* gpio, IDisplayProvider* display, IStyleService* styleService);
    static std::unique_ptr<IPanel> createLockPanel(IGpioProvider* gpio, IDisplayProvider* display, IStyleService* styleService);
    static std::unique_ptr<IPanel> createSplashPanel(IGpioProvider* gpio, IDisplayProvider* display, IStyleService* styleService);
    static std::unique_ptr<IPanel> createOemOilPanel(IGpioProvider* gpio, IDisplayProvider* display, IStyleService* styleService);
};