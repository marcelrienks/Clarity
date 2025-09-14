#include "managers/preference_manager.h"
#include <esp32-hal-log.h>

// ========== Public Interface Methods ==========

/**
 * @brief Initialize NVS storage system and load configuration
 * @details Handles NVS corruption by formatting and retrying, then loads config from storage
 */
void PreferenceManager::Init()
{
    log_v("Init() called");

    // Initialize NVS preferences namespace
    if (!preferences_.begin(SystemConstants::PREFERENCES_NAMESPACE, false))
    {
        log_w("Failed to initialize preferences, retrying after format");

        // Format NVS partition if opening failed (handles corruption)
        nvs_flash_erase();

        // Retry after format
        if (!preferences_.begin(SystemConstants::PREFERENCES_NAMESPACE, false))
            log_w("Failed to initialize preferences after format");

        else
            log_w("Preferences initialized successfully after format");
    }
    else
        log_i("PreferenceManager service started successfully - configuration loaded and ready");

    // Load existing configuration or create defaults
    LoadConfig();
}

/**
 * @brief Serialize and persist current configuration to NVS
 * @details Creates JSON document, serializes all config settings, commits to flash storage
 */
void PreferenceManager::SaveConfig()
{
    log_v("SaveConfig() called");

    // Clear existing config entry to prevent corruption
    preferences_.remove(CONFIG_KEY_);

    // Build JSON document with all configuration settings
    JsonDocument doc;
    doc[JsonDocNames::PANEL_NAME] = config_.panelName.c_str();
    doc[JsonDocNames::SHOW_SPLASH] = config_.showSplash;
    doc[JsonDocNames::SPLASH_DURATION] = config_.splashDuration;
    doc[JsonDocNames::THEME] = config_.theme.c_str();
    doc[JsonDocNames::UPDATE_RATE] = config_.updateRate;
    doc[JsonDocNames::PRESSURE_UNIT] = config_.pressureUnit.c_str();
    doc[JsonDocNames::TEMP_UNIT] = config_.tempUnit.c_str();

    // Include sensor calibration settings
    doc[JsonDocNames::PRESSURE_OFFSET] = config_.pressureOffset;
    doc[JsonDocNames::PRESSURE_SCALE] = config_.pressureScale;
    doc[JsonDocNames::TEMP_OFFSET] = config_.tempOffset;
    doc[JsonDocNames::TEMP_SCALE] = config_.tempScale;

    // Serialize to JSON string for storage
    String jsonString;
    serializeJson(doc, jsonString);

    // Persist JSON string to NVS flash storage
    size_t written = preferences_.putString(CONFIG_KEY_, jsonString);
    if (written > 0)
    {
        log_i("Configuration saved successfully (%zu bytes written)", written);
        // Force commit to NVS flash - critical for persistence across reboots
        preferences_.end();
        // Reopen preferences handle for future operations
        preferences_.begin(SystemConstants::PREFERENCES_NAMESPACE, false);
        log_i("System configuration updated and persisted to NVS storage");
    }
    else
    {
        log_e("Failed to save configuration to NVS");
    }
}

/**
 * @brief Load and deserialize configuration from NVS storage
 * @details Reads JSON config, validates format, falls back to defaults on error
 */
void PreferenceManager::LoadConfig()
{
    log_v("LoadConfig() called");

    // Attempt to read JSON configuration string from NVS
    String jsonString = preferences_.getString(CONFIG_KEY_, "");
    if (jsonString.length() == 0)
    {
        log_w("No config found, creating default");
        return PreferenceManager::CreateDefaultConfig();
    }

    // Deserialize JSON configuration
    JsonDocument doc;
    DeserializationError result = deserializeJson(doc, jsonString);
    if (result != DeserializationError::Ok)
    {
        log_w("Error deserializing config - JSON corruption detected");
        return PreferenceManager::CreateDefaultConfig();
    }

    // Validate essential config fields
    if (doc[JsonDocNames::PANEL_NAME].isNull())
    {
        log_w("Error reading config - missing panel name");
        return PreferenceManager::CreateDefaultConfig();
    }

    // Load core configuration settings
    config_.panelName = std::string(doc[JsonDocNames::PANEL_NAME].as<const char *>());

    // Load optional settings with graceful fallback for missing fields
    if (!doc[JsonDocNames::SHOW_SPLASH].isNull())
    {
        config_.showSplash = doc[JsonDocNames::SHOW_SPLASH].as<bool>();
    }

    if (!doc[JsonDocNames::SPLASH_DURATION].isNull())
    {
        config_.splashDuration = doc[JsonDocNames::SPLASH_DURATION].as<int>();
    }

    if (!doc[JsonDocNames::THEME].isNull())
    {
        config_.theme = std::string(doc[JsonDocNames::THEME].as<const char *>());
    }

    if (!doc[JsonDocNames::UPDATE_RATE].isNull())
    {
        config_.updateRate = doc[JsonDocNames::UPDATE_RATE].as<int>();
    }

    if (!doc[JsonDocNames::PRESSURE_UNIT].isNull())
    {
        config_.pressureUnit = std::string(doc[JsonDocNames::PRESSURE_UNIT].as<const char *>());
    }

    if (!doc[JsonDocNames::TEMP_UNIT].isNull())
    {
        config_.tempUnit = std::string(doc[JsonDocNames::TEMP_UNIT].as<const char *>());
    }

    // Load sensor calibration settings
    if (!doc[JsonDocNames::PRESSURE_OFFSET].isNull())
    {
        config_.pressureOffset = doc[JsonDocNames::PRESSURE_OFFSET].as<float>();
    }
    if (!doc[JsonDocNames::PRESSURE_SCALE].isNull())
    {
        config_.pressureScale = doc[JsonDocNames::PRESSURE_SCALE].as<float>();
    }
    if (!doc[JsonDocNames::TEMP_OFFSET].isNull())
    {
        config_.tempOffset = doc[JsonDocNames::TEMP_OFFSET].as<float>();
    }
    if (!doc[JsonDocNames::TEMP_SCALE].isNull())
    {
        config_.tempScale = doc[JsonDocNames::TEMP_SCALE].as<float>();
    }
}

/**
 * @brief Create default configuration and persist to storage
 * @details Sets up initial default values and saves to NVS
 */
void PreferenceManager::CreateDefaultConfig()
{
    log_v("CreateDefaultConfig() called");

    // Set default panel selection
    config_.panelName = PanelNames::OIL;

    // Persist defaults to storage and reload to verify
    PreferenceManager::SaveConfig();
    PreferenceManager::LoadConfig();
}

/**
 * @brief Get mutable reference to current configuration
 */
Configs &PreferenceManager::GetConfig()
{
    return config_;
}

/**
 * @brief Get read-only reference to current configuration
 */
const Configs &PreferenceManager::GetConfig() const
{
    log_v("GetConfig() const called");
    return config_;
}

/**
 * @brief Replace current configuration with new settings
 * @details Updates in-memory config - call SaveConfig() to persist
 */
void PreferenceManager::SetConfig(const Configs &newConfig)
{
    log_v("SetConfig() called");
    // Update in-memory configuration
    config_ = newConfig;
}

/**
 * @brief Get preference value as string using key lookup
 * @details Converts typed config values to string representation
 */
std::string PreferenceManager::GetPreference(const std::string &key) const
{
    log_v("GetPreference() called");

    // String preferences (direct return)
    if (key == "panel_name")
        return config_.panelName;
    if (key == "theme")
        return config_.theme;
    if (key == "pressure_unit")
        return config_.pressureUnit;
    if (key == "temp_unit")
        return config_.tempUnit;

    // Boolean preferences
    if (key == "show_splash")
        return config_.showSplash ? "true" : "false";

    // Integer preferences
    if (key == "splash_duration") {
        static char buffer[16];
        snprintf(buffer, sizeof(buffer), "%d", config_.splashDuration);
        return buffer;
    }
    if (key == "update_rate") {
        static char buffer[16];
        snprintf(buffer, sizeof(buffer), "%d", config_.updateRate);
        return buffer;
    }

    // Float preferences (calibration values)
    if (key == "pressure_offset") {
        static char buffer[16];
        snprintf(buffer, sizeof(buffer), "%.2f", config_.pressureOffset);
        return buffer;
    }
    if (key == "pressure_scale") {
        static char buffer[16];
        snprintf(buffer, sizeof(buffer), "%.2f", config_.pressureScale);
        return buffer;
    }
    if (key == "temp_offset") {
        static char buffer[16];
        snprintf(buffer, sizeof(buffer), "%.2f", config_.tempOffset);
        return buffer;
    }
    if (key == "temp_scale") {
        static char buffer[16];
        snprintf(buffer, sizeof(buffer), "%.2f", config_.tempScale);
        return buffer;
    }

    log_w("Unknown preference key: %s", key.c_str());
    return "";
}

/**
 * @brief Update preference value using key-based lookup
 * @details Parses string value and updates appropriate config field
 */
void PreferenceManager::SetPreference(const std::string &key, const std::string &value)
{
    log_v("SetPreference() called");

    // String preferences (direct assignment)
    if (key == "panel_name")
    {
        config_.panelName = value;
    }
    else if (key == "theme")
    {
        config_.theme = value;
    }
    else if (key == "pressure_unit")
    {
        config_.pressureUnit = value;
    }
    else if (key == "temp_unit")
    {
        config_.tempUnit = value;
    }
    // Boolean preferences
    else if (key == "show_splash")
    {
        config_.showSplash = (value == "true");
    }
    // Integer preferences
    else if (key == "splash_duration")
    {
        config_.splashDuration = std::stoi(value);
    }
    else if (key == "update_rate")
    {
        config_.updateRate = std::stoi(value);
    }
    // Float preferences (sensor calibration)
    else if (key == "pressure_offset")
    {
        config_.pressureOffset = std::stof(value);
    }
    else if (key == "pressure_scale")
    {
        config_.pressureScale = std::stof(value);
    }
    else if (key == "temp_offset")
    {
        config_.tempOffset = std::stof(value);
    }
    else if (key == "temp_scale")
    {
        config_.tempScale = std::stof(value);
    }
    else
    {
        log_w("Unknown preference key: %s", key.c_str());
    }
}

/**
 * @brief Validate if preference key is recognized by the system
 * @details Checks against all supported configuration keys
 */
bool PreferenceManager::HasPreference(const std::string &key) const
{
    log_v("HasPreference() called");

    // Check against all supported preference keys
    bool hasKey = (key == "panel_name" || key == "show_splash" || key == "splash_duration" || key == "theme" ||
                   key == "update_rate" || key == "pressure_unit" || key == "temp_unit" ||
                   key == "pressure_offset" || key == "pressure_scale" || key == "temp_offset" || key == "temp_scale");
    return hasKey;
}