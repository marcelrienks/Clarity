#pragma once

#include <memory>
#include "interfaces/i_panel_factory.h"

class IGpioProvider;
class IDisplayProvider;
class IStyleService;
class SplashPanel;
class OemOilPanel;
class ErrorPanel;
class ConfigPanel;
class KeyPanel;
class LockPanel;

class PanelFactory : public IPanelFactory {
public:
    static PanelFactory& Instance();
    
    // IPanelFactory implementation
    std::unique_ptr<IPanel> CreateSplashPanel(IGpioProvider* gpio, IDisplayProvider* display, IStyleService* style) override;
    std::unique_ptr<IPanel> CreateOemOilPanel(IGpioProvider* gpio, IDisplayProvider* display, IStyleService* style) override;
    std::unique_ptr<IPanel> CreateErrorPanel(IGpioProvider* gpio, IDisplayProvider* display, IStyleService* style) override;
    std::unique_ptr<IPanel> CreateConfigPanel(IGpioProvider* gpio, IDisplayProvider* display, IStyleService* style) override;
    std::unique_ptr<IPanel> CreateKeyPanel(IGpioProvider* gpio, IDisplayProvider* display, IStyleService* style) override;
    std::unique_ptr<IPanel> CreateLockPanel(IGpioProvider* gpio, IDisplayProvider* display, IStyleService* style) override;
    
private:
    PanelFactory() = default;
    ~PanelFactory() = default;
    PanelFactory(const PanelFactory&) = delete;
    PanelFactory& operator=(const PanelFactory&) = delete;
};