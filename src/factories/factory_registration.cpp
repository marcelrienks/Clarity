#include "factories/factory_registration.h"
#include "factories/component_factory.h"
#include "factories/panel_factory.h"
#include "utilities/constants.h"

// Panel includes
#include "panels/splash_panel.h"
#include "panels/oem_oil_panel.h"
#include "panels/error_panel.h"
#include "panels/config_panel.h"
#include "panels/dynamic_config_panel.h"
#include "panels/key_panel.h"
#include "panels/lock_panel.h"

// Component includes
#include "components/clarity_component.h"
#include "components/oem/oem_oil_pressure_component.h"
#include "components/oem/oem_oil_temperature_component.h"
#include "components/error_component.h"
#include "components/key_component.h"
#include "components/lock_component.h"
#include "components/config_component.h"

#ifdef CLARITY_DEBUG
    #include "esp32-hal-log.h"
    #define LOG_TAG "FactoryRegistration"
#else
    #define log_d(...)
    #define log_i(...)
#endif

/// @brief Register all available panel types with PanelFactory
void RegisterAllPanels()
{
    log_d("Registering all panel types with PanelFactory...");

    // Register SplashPanel
    PanelFactory::RegisterPanel(PanelNames::SPLASH, 
        [](IGpioProvider* gpio, IDisplayProvider* display, IStyleService* style) {
            return std::make_unique<SplashPanel>(gpio, display, style);
        });

    // Register OemOilPanel (main monitoring panel)
    PanelFactory::RegisterPanel(PanelNames::OIL, 
        [](IGpioProvider* gpio, IDisplayProvider* display, IStyleService* style) {
            return std::make_unique<OemOilPanel>(gpio, display, style);
        });

    // Register ErrorPanel
    PanelFactory::RegisterPanel(PanelNames::ERROR, 
        [](IGpioProvider* gpio, IDisplayProvider* display, IStyleService* style) {
            return std::make_unique<ErrorPanel>(gpio, display, style);
        });

    // Register ConfigPanel
    PanelFactory::RegisterPanel(PanelNames::CONFIG, 
        [](IGpioProvider* gpio, IDisplayProvider* display, IStyleService* style) {
            return std::make_unique<ConfigPanel>(gpio, display, style);
        });

    // Register KeyPanel
    PanelFactory::RegisterPanel(PanelNames::KEY, 
        [](IGpioProvider* gpio, IDisplayProvider* display, IStyleService* style) {
            return std::make_unique<KeyPanel>(gpio, display, style);
        });

    // Register LockPanel
    PanelFactory::RegisterPanel(PanelNames::LOCK, 
        [](IGpioProvider* gpio, IDisplayProvider* display, IStyleService* style) {
            return std::make_unique<LockPanel>(gpio, display, style);
        });

    log_i("Registered %zu panel types with PanelFactory", 
          PanelFactory::GetRegisteredPanels().size());
}

/// @brief Register all available component types with ComponentFactory
void RegisterAllComponents()
{
    log_d("Registering all component types with ComponentFactory...");

    // Register ClarityComponent (branding/logo)
    ComponentFactory::RegisterComponent("Clarity", 
        [](IStyleService* style) {
            return std::make_unique<ClarityComponent>(style);
        });

    // Register OemOilPressureComponent
    ComponentFactory::RegisterComponent("OilPressure", 
        [](IStyleService* style) {
            return std::make_unique<OemOilPressureComponent>(style);
        });

    // Register OemOilTemperatureComponent
    ComponentFactory::RegisterComponent("OilTemperature", 
        [](IStyleService* style) {
            return std::make_unique<OemOilTemperatureComponent>(style);
        });

    // Register ErrorComponent
    ComponentFactory::RegisterComponent("Error", 
        [](IStyleService* style) {
            return std::make_unique<ErrorComponent>(style);
        });

    // Register KeyComponent
    ComponentFactory::RegisterComponent("Key", 
        [](IStyleService* style) {
            return std::make_unique<KeyComponent>(style);
        });

    // Register LockComponent
    ComponentFactory::RegisterComponent("Lock", 
        [](IStyleService* style) {
            return std::make_unique<LockComponent>(style);
        });

    // Register ConfigComponent
    ComponentFactory::RegisterComponent("Config", 
        [](IStyleService* style) {
            return std::make_unique<ConfigComponent>();
        });

    log_i("Registered %zu component types with ComponentFactory", 
          ComponentFactory::GetRegisteredComponents().size());
}

/// @brief Initialize both panel and component factories
void InitializeFactories()
{
    log_i("Initializing UI factories...");
    
    RegisterAllPanels();
    RegisterAllComponents();
    
    log_i("UI factories initialized successfully");
    log_i("Available panels: %zu types", PanelFactory::GetRegisteredPanels().size());
    log_i("Available components: %zu types", ComponentFactory::GetRegisteredComponents().size());
}