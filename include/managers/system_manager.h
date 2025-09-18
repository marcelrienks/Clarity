#pragma once

#include "interfaces/i_preference_service.h"
#include "config/config_types.h"

/**
 * @class SystemManager
 * @brief Manages general system configuration settings
 *
 * Handles system-wide configuration that doesn't belong to specific components,
 * such as default panel selection, update rates, and general application settings.
 */
class SystemManager
{
public:
    // ========== Constructors and Destructor ==========
    SystemManager() = default;
    SystemManager(const SystemManager&) = delete;
    SystemManager& operator=(const SystemManager&) = delete;
    ~SystemManager() = default;

    // ========== Public Interface Methods ==========
    void SetPreferenceService(IPreferenceService* preferenceService);
    void RegisterConfiguration();
    void LoadConfiguration();

    // ========== Configuration Constants ==========
    static constexpr const char* CONFIG_DEFAULT_PANEL = "system.default_panel";
    static constexpr const char* CONFIG_UPDATE_RATE = "system.update_rate";
    static constexpr const char* CONFIG_SHOW_SPLASH = "system.show_splash";

    // ========== Getters ==========
    std::string GetDefaultPanel() const { return defaultPanel_; }
    int GetUpdateRate() const { return updateRate_; }
    bool GetShowSplash() const { return showSplash_; }

private:
    // ========== Private Data Members ==========
    IPreferenceService* preferenceService_ = nullptr;

    // Configuration values
    std::string defaultPanel_ = "OemOilPanel";
    int updateRate_ = 500;
    bool showSplash_ = true;
};