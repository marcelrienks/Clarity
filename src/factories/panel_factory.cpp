#include "factories/panel_factory.h"
#include "panels/splash_panel.h"
#include "panels/oem_oil_panel.h"
#include "panels/error_panel.h"
#include "panels/config_panel.h"
#include "panels/key_panel.h"
#include "panels/lock_panel.h"
#include "interfaces/i_gpio_provider.h"
#include "interfaces/i_display_provider.h"
#include "interfaces/i_style_service.h"

#include "esp32-hal-log.h"

PanelFactory& PanelFactory::Instance()
{
    log_v("Instance() called");
    static PanelFactory instance;
    return instance;
}

std::unique_ptr<IPanel> PanelFactory::CreateSplashPanel(IGpioProvider* gpio, IDisplayProvider* display, IStyleService* style)
{
    log_v("CreateSplashPanel() called");
    return std::make_unique<SplashPanel>(gpio, display, style);
}

std::unique_ptr<IPanel> PanelFactory::CreateOemOilPanel(IGpioProvider* gpio, IDisplayProvider* display, IStyleService* style)
{
    log_v("CreateOemOilPanel() called");
    return std::make_unique<OemOilPanel>(gpio, display, style);
}

std::unique_ptr<IPanel> PanelFactory::CreateErrorPanel(IGpioProvider* gpio, IDisplayProvider* display, IStyleService* style)
{
    log_v("CreateErrorPanel() called");
    return std::make_unique<ErrorPanel>(gpio, display, style);
}

std::unique_ptr<IPanel> PanelFactory::CreateConfigPanel(IGpioProvider* gpio, IDisplayProvider* display, IStyleService* style)
{
    log_v("CreateConfigPanel() called");
    return std::make_unique<ConfigPanel>(gpio, display, style);
}

std::unique_ptr<IPanel> PanelFactory::CreateKeyPanel(IGpioProvider* gpio, IDisplayProvider* display, IStyleService* style)
{
    log_v("CreateKeyPanel() called");
    return std::make_unique<KeyPanel>(gpio, display, style);
}

std::unique_ptr<IPanel> PanelFactory::CreateLockPanel(IGpioProvider* gpio, IDisplayProvider* display, IStyleService* style)
{
    log_v("CreateLockPanel() called");
    return std::make_unique<LockPanel>(gpio, display, style);
}