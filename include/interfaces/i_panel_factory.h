#pragma once
#include <memory>

class IGpioProvider;
class IDisplayProvider;
class IStyleService;
class IPanel;

class IPanelFactory {
public:
    virtual ~IPanelFactory() = default;
    virtual std::unique_ptr<IPanel> CreateSplashPanel(IGpioProvider* gpio, IDisplayProvider* display, IStyleService* style) = 0;
    virtual std::unique_ptr<IPanel> CreateOemOilPanel(IGpioProvider* gpio, IDisplayProvider* display, IStyleService* style) = 0;
    virtual std::unique_ptr<IPanel> CreateErrorPanel(IGpioProvider* gpio, IDisplayProvider* display, IStyleService* style) = 0;
    virtual std::unique_ptr<IPanel> CreateConfigPanel(IGpioProvider* gpio, IDisplayProvider* display, IStyleService* style) = 0;
    virtual std::unique_ptr<IPanel> CreateKeyPanel(IGpioProvider* gpio, IDisplayProvider* display, IStyleService* style) = 0;
    virtual std::unique_ptr<IPanel> CreateLockPanel(IGpioProvider* gpio, IDisplayProvider* display, IStyleService* style) = 0;
};