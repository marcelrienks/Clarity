#pragma once // preventing duplicate definitions, alternative to the traditional include guards

#include "utilities/types.h"
#include "interfaces/i_preference_service.h"

#include <Preferences.h>
#include <nvs_flash.h>
#include <ArduinoJson.h>

/**
 * @class PreferenceManager
 * @brief Configuration persistence manager using ESP32 NVS
 * 
 * @details This manager handles all persistent configuration storage and
 * retrieval using the ESP32 Non-Volatile Storage (NVS) system. It provides
 * a centralized interface for saving and loading user preferences, with
 * automatic JSON serialization and default configuration creation.
 * 
 * @design_pattern Dependency Injection - managed by service container
 * @storage_backend ESP32 NVS (Non-Volatile Storage)
 * @serialization_format JSON via ArduinoJson library
 * @configuration_key "config" (stored in NVS)
 * 
 * @managed_settings:
 * - panel_name: Default panel to load on startup
 * - theme: Day/Night theme selection
 * - Future: Display brightness, refresh rates, sensor calibration
 * 
 * @persistence_flow:
 * 1. init(): Initialize NVS and load existing config
 * 2. loadConfig(): Deserialize JSON from NVS
 * 3. createDefaultConfig(): Create defaults if no config exists
 * 4. saveConfig(): Serialize and store config to NVS
 * 
 * @error_handling:
 * - Graceful fallback to defaults on corruption
 * - Automatic config recreation on missing data
 * - NVS error logging and recovery
 * 
 * @static_config Global config instance accessible via PreferenceManager::config
 * @thread_safety NVS operations are thread-safe by design
 * 
 * @context This manager stores and retrieves user preferences like
 * the default panel to show on startup. It uses ESP32's built-in NVS for
 * persistent storage across reboots.
 */
class PreferenceManager : public IPreferenceService
{
public:
    // Static Methods removed - using dependency injection

    // IPreferenceService interface implementation
    void init() override;
    void saveConfig() override;
    void loadConfig() override;
    void createDefaultConfig() override;
    Configs& getConfig() override;
    const Configs& getConfig() const override;
    void setConfig(const Configs& config) override;

    // Public Data Members
    inline static Configs config;

private:

    // Static Data Members
    inline static const char *CONFIG_KEY = "config";

    // Instance Data Members
    Preferences preferences_;
};