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
    log_d("Instance() called");
    static PanelFactory instance;
    return instance;
}

std::unique_ptr<IPanel> PanelFactory::CreateSplashPanel(IGpioProvider* gpio, IDisplayProvider* display, IStyleService* style)
{
    log_d("CreateSplashPanel() called");

    auto panel = std::make_unique<SplashPanel>(gpio, display, style);
    if (!panel) {
        log_e("Failed to create SplashPanel - allocation failed");
        return nullptr;
    }
    return panel;
}

std::unique_ptr<IPanel> PanelFactory::CreateOemOilPanel(IGpioProvider* gpio, IDisplayProvider* display, IStyleService* style)
{
    log_d("CreateOemOilPanel() called");

    auto panel = std::make_unique<OemOilPanel>(gpio, display, style);
    if (!panel) {
        log_e("Failed to create OemOilPanel - allocation failed");
        return nullptr;
    }
    return panel;
}

std::unique_ptr<IPanel> PanelFactory::CreateErrorPanel(IGpioProvider* gpio, IDisplayProvider* display, IStyleService* style)
{
    log_d("CreateErrorPanel() called");

    auto panel = std::make_unique<ErrorPanel>(gpio, display, style);
    if (!panel) {
        log_e("Failed to create ErrorPanel - allocation failed");
        return nullptr;
    }
    return panel;
}

std::unique_ptr<IPanel> PanelFactory::CreateConfigPanel(IGpioProvider* gpio, IDisplayProvider* display, IStyleService* style)
{
    log_d("CreateConfigPanel() called");

    auto panel = std::make_unique<ConfigPanel>(gpio, display, style);
    if (!panel) {
        log_e("Failed to create ConfigPanel - allocation failed");
        return nullptr;
    }
    return panel;
}

std::unique_ptr<IPanel> PanelFactory::CreateKeyPanel(IGpioProvider* gpio, IDisplayProvider* display, IStyleService* style)
{
    log_d("CreateKeyPanel() called");

    auto panel = std::make_unique<KeyPanel>(gpio, display, style);
    if (!panel) {
        log_e("Failed to create KeyPanel - allocation failed");
        return nullptr;
    }
    return panel;
}

std::unique_ptr<IPanel> PanelFactory::CreateLockPanel(IGpioProvider* gpio, IDisplayProvider* display, IStyleService* style)
{
    log_d("CreateLockPanel() called");

    auto panel = std::make_unique<LockPanel>(gpio, display, style);
    if (!panel) {
        log_e("Failed to create LockPanel - allocation failed");
        return nullptr;
    }
    return panel;
}