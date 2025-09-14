#pragma once

#include "interfaces/i_preference_service.h"
#include "utilities/types.h"

#include <ArduinoJson.h>
#include <Preferences.h>
#include <nvs_flash.h>

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
    // ========== Constructors and Destructor ==========
    PreferenceManager() = default;
    PreferenceManager(const PreferenceManager&) = delete;
    PreferenceManager& operator=(const PreferenceManager&) = delete;
    ~PreferenceManager() = default;

    // ========== Public Interface Methods ==========
    /**
     * @brief Initialize NVS storage and load existing configuration
     * @details Attempts NVS format and retry if initial open fails
     */
    void Init() override;

    /**
     * @brief Persist current configuration to NVS storage
     * @details Serializes config to JSON and commits to flash storage
     */
    void SaveConfig() override;

    /**
     * @brief Load configuration from NVS storage
     * @details Deserializes JSON config, falls back to defaults if corrupted
     */
    void LoadConfig() override;

    /**
     * @brief Create and save default configuration settings
     * @details Sets up initial config values and persists them
     */
    void CreateDefaultConfig() override;

    /**
     * @brief Get mutable reference to current configuration
     * @return Reference to configuration object for modification
     */
    Configs &GetConfig() override;

    /**
     * @brief Get read-only reference to current configuration
     * @return Const reference to configuration object
     */
    const Configs &GetConfig() const override;

    /**
     * @brief Update the configuration with new settings
     * @param config New configuration settings to apply
     */
    void SetConfig(const Configs &config) override;

    /**
     * @brief Get preference value by key name
     * @param key Preference key to retrieve
     * @return String representation of preference value
     */
    std::string GetPreference(const std::string &key) const override;

    /**
     * @brief Set preference value by key name
     * @param key Preference key to update
     * @param value New preference value as string
     */
    void SetPreference(const std::string &key, const std::string &value) override;

    /**
     * @brief Check if a preference key exists
     * @param key Preference key to check
     * @return true if key is recognized, false otherwise
     */
    bool HasPreference(const std::string &key) const override;

private:
    // ========== Private Data Members ==========
    static inline const char *CONFIG_KEY_ = "config";
    static inline Configs config_;
    Preferences preferences_;
};