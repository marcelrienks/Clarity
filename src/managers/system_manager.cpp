#include "managers/system_manager.h"
#include "utilities/logging.h"
#include <esp32-hal-log.h>

// ========== Public Interface Methods ==========

/**
 * @brief Inject preference service dependency
 * @param preferenceService Preference service for configuration management
 */
void SystemManager::SetPreferenceService(IPreferenceService* preferenceService)
{
    log_v("SetPreferenceService() called");
    preferenceService_ = preferenceService;

    // Register configuration when preference service is available
    RegisterConfiguration();
}

/**
 * @brief Register SystemManager configuration section
 */
void SystemManager::RegisterConfiguration()
{
    if (!preferenceService_) return;

    using namespace Config;

    ConfigSection section("SystemManager", "system", "System Settings");
    section.displayOrder = 5; // Higher priority than other managers

    // Default panel selection
    ConfigItem defaultPanelItem("default_panel", "Default Panel", ConfigValueType::Enum,
                               std::string("OemOilPanel"), ConfigMetadata("OemOilPanel,ConfigPanel,DiagnosticPanel"));

    // Update rate setting
    ConfigItem updateRateItem("update_rate", "Update Rate", ConfigValueType::Integer,
                             500, ConfigMetadata("100-2000", "ms"));

    // Show splash screen setting
    ConfigItem showSplashItem("show_splash", "Show Splash Screen", ConfigValueType::Boolean,
                             true, ConfigMetadata());

    section.AddItem(defaultPanelItem);
    section.AddItem(updateRateItem);
    section.AddItem(showSplashItem);

    preferenceService_->RegisterConfigSection(section);
    log_i("SystemManager configuration registered");
}

/**
 * @brief Load configuration from preference system
 */
void SystemManager::LoadConfiguration()
{
    if (!preferenceService_) return;

    // Load default panel using type-safe config system
    if (auto panelValue = preferenceService_->QueryConfig<std::string>(CONFIG_DEFAULT_PANEL)) {
        defaultPanel_ = *panelValue;
    }

    // Load update rate using type-safe config system
    if (auto rateValue = preferenceService_->QueryConfig<int>(CONFIG_UPDATE_RATE)) {
        updateRate_ = *rateValue;
    }

    // Load show splash using type-safe config system
    if (auto splashValue = preferenceService_->QueryConfig<bool>(CONFIG_SHOW_SPLASH)) {
        showSplash_ = *splashValue;
    }

    log_i("Loaded system configuration: panel=%s, rate=%d, splash=%s",
          defaultPanel_.c_str(), updateRate_, showSplash_ ? "true" : "false");
}