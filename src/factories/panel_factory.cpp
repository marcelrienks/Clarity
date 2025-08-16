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

#ifdef CLARITY_DEBUG
    #include "esp32-hal-log.h"
    #define LOG_TAG "PanelFactory"
#else
    #define log_d(...)
    #define log_e(...)
#endif

PanelFactory& PanelFactory::Instance()
{
    log_v("Instance() called");
    static PanelFactory instance;
    return instance;
}

std::unique_ptr<IPanel> PanelFactory::CreateSplashPanel(IGpioProvider* gpio, IDisplayProvider* display, IStyleService* style)
{
    log_v("CreateSplashPanel() called");
    log_d("Creating SplashPanel");
    return std::make_unique<SplashPanel>(gpio, display, style);
}

std::unique_ptr<IPanel> PanelFactory::CreateOemOilPanel(IGpioProvider* gpio, IDisplayProvider* display, IStyleService* style)
{
    log_v("CreateOemOilPanel() called");
    log_d("Creating OemOilPanel");
    return std::make_unique<OemOilPanel>(gpio, display, style);
}

std::unique_ptr<IPanel> PanelFactory::CreateErrorPanel(IGpioProvider* gpio, IDisplayProvider* display, IStyleService* style)
{
    log_v("CreateErrorPanel() called");
    log_d("Creating ErrorPanel");
    return std::make_unique<ErrorPanel>(gpio, display, style);
}

std::unique_ptr<IPanel> PanelFactory::CreateConfigPanel(IGpioProvider* gpio, IDisplayProvider* display, IStyleService* style)
{
    log_v("CreateConfigPanel() called");
    log_d("Creating ConfigPanel");
    return std::make_unique<ConfigPanel>(gpio, display, style);
}

std::unique_ptr<IPanel> PanelFactory::CreateKeyPanel(IGpioProvider* gpio, IDisplayProvider* display, IStyleService* style)
{
    log_v("CreateKeyPanel() called");
    log_d("Creating KeyPanel");
    return std::make_unique<KeyPanel>(gpio, display, style);
}

std::unique_ptr<IPanel> PanelFactory::CreateLockPanel(IGpioProvider* gpio, IDisplayProvider* display, IStyleService* style)
{
    log_v("CreateLockPanel() called");
    log_d("Creating LockPanel");
    return std::make_unique<LockPanel>(gpio, display, style);
}