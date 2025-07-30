#include "factories/panel_factory.h"

PanelFactory::PanelFactory(IComponentFactory* componentFactory, 
                           IDisplayProvider* displayProvider, 
                           IGpioProvider* gpioProvider)
    : componentFactory_(componentFactory)
    , displayProvider_(displayProvider)
    , gpioProvider_(gpioProvider)
{
}

std::unique_ptr<IPanel> PanelFactory::createPanel(const std::string& panelType)
{
    if (panelType == PanelNames::SPLASH) {
        return std::make_unique<SplashPanel>(componentFactory_);
    }
    else if (panelType == PanelNames::OIL) {
        return std::make_unique<OemOilPanel>(componentFactory_);
    }
    else if (panelType == PanelNames::KEY) {
        return std::make_unique<KeyPanel>(componentFactory_);
    }
    else if (panelType == PanelNames::LOCK) {
        return std::make_unique<LockPanel>(componentFactory_);
    }
    
    throw std::runtime_error("Unsupported panel type: " + panelType);
}

bool PanelFactory::supportsPanel(const std::string& panelType) const
{
    return panelType == PanelNames::SPLASH ||
           panelType == PanelNames::OIL ||
           panelType == PanelNames::KEY ||
           panelType == PanelNames::LOCK;
}