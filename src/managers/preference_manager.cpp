#include "managers/preference_manager.h"
#include <esp32-hal-log.h>

// Static Methods removed - using dependency injection

// Core Functionality Methods

/// @brief Initialises the preference manager to handle application preferences_
void PreferenceManager::Init()
{
    log_v("Init() called");

    // Initialize preferences_
    if (!preferences_.begin(SystemConstants::PREFERENCES_NAMESPACE, false))
    {
        log_w("Failed to initialize preferences_, retrying after format");

        // Format NVS if opening failed
        nvs_flash_erase();

        // Try again
        if (!preferences_.begin(SystemConstants::PREFERENCES_NAMESPACE, false))
            log_w("Failed to initialize preferences_ after format");

        else
            log_w("Preferences initialized successfully after format");
    }

    else
        log_i("PreferenceManager service started successfully - configuration loaded and ready");

    LoadConfig();
}

/// @brief Create and save a list of default panels
/// @return true if the save was successful
void PreferenceManager::CreateDefaultConfig()
{
    log_v("CreateDefaultConfig() called");
    log_v("CreateDefaultConfig() called");

    config_.panelName = PanelNames::OIL;

    PreferenceManager::SaveConfig();
    PreferenceManager::LoadConfig();
}

/// @brief Load the configuration from preferences_
/// @return true if the load was successful, false otherwise
void PreferenceManager::LoadConfig()
{
    log_v("LoadConfig() called");
    log_v("LoadConfig() called");

    String jsonString = preferences_.getString(CONFIG_KEY_, "");
    if (jsonString.length() == 0)
    {
        log_w("No config found, creating default");
        return PreferenceManager::CreateDefaultConfig();
    }

    JsonDocument doc;
    DeserializationError result = deserializeJson(doc, jsonString);
    if (result != DeserializationError::Ok)
    {
        log_w("Error deserializing config");
        return PreferenceManager::CreateDefaultConfig();
    }

    if (doc[JsonDocNames::PANEL_NAME].isNull())
    {
        log_w("Error reading config");
        return PreferenceManager::CreateDefaultConfig();
    }

    config_.panelName = std::string(doc[JsonDocNames::PANEL_NAME].as<const char *>());

    // Load all settings with defaults if not present
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

    // Load calibration settings
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

/// @brief Save the current config_uration to preferences_
/// @return true if the save was successful, false otherwise
void PreferenceManager::SaveConfig()
{
    log_v("SaveConfig() called");
    log_v("SaveConfig() called");

    preferences_.remove(CONFIG_KEY_);

    // Use the new JsonDocument instead of the deprecated classes
    JsonDocument doc;
    doc[JsonDocNames::PANEL_NAME] = config_.panelName.c_str();
    doc[JsonDocNames::SHOW_SPLASH] = config_.showSplash;
    doc[JsonDocNames::SPLASH_DURATION] = config_.splashDuration;
    doc[JsonDocNames::THEME] = config_.theme.c_str();
    doc[JsonDocNames::UPDATE_RATE] = config_.updateRate;
    doc[JsonDocNames::PRESSURE_UNIT] = config_.pressureUnit.c_str();
    doc[JsonDocNames::TEMP_UNIT] = config_.tempUnit.c_str();
    
    // Serialize calibration settings
    doc[JsonDocNames::PRESSURE_OFFSET] = config_.pressureOffset;
    doc[JsonDocNames::PRESSURE_SCALE] = config_.pressureScale;
    doc[JsonDocNames::TEMP_OFFSET] = config_.tempOffset;
    doc[JsonDocNames::TEMP_SCALE] = config_.tempScale;

    // Serialize to JSON string
    String jsonString;
    serializeJson(doc, jsonString);


    // Save the JSON string to preferences_
    size_t written = preferences_.putString(CONFIG_KEY_, jsonString);
    if (written > 0)
    {
        log_i("Configuration saved successfully (%zu bytes written)", written);
        // Commit changes to NVS - this is critical for persistence across reboots
        preferences_.end();
        // Reopen preferences for future operations
        preferences_.begin(SystemConstants::PREFERENCES_NAMESPACE, false);
        log_i("System config_uration updated and persisted to NVS storage");
    }
    else
    {
        log_e("Failed to save config_uration to NVS");
    }
}

// IPreferenceService interface implementation

/// @brief Get the current config_uration object
/// @return Reference to current config_uration settings
Configs &PreferenceManager::GetConfig()
{
    return config_;
}

/// @brief Get the current config_uration object (read-only)
/// @return Const reference to current config_uration settings
const Configs &PreferenceManager::GetConfig() const
{
    log_v("GetConfig() const called");
    return config_;
}

/// @brief Update the config_uration object
/// @param newConfig New config_uration settings to apply
void PreferenceManager::SetConfig(const Configs &newConfig)
{
    log_v("SetConfig() called");
    // Configuration updated
    config_ = newConfig;
}

// Generic preference access methods for dynamic menus

/// @brief Get a preference value by key
/// @param key The preference key
/// @return String representation of the value
std::string PreferenceManager::GetPreference(const std::string &key) const
{
    log_v("GetPreference() called");
    if (key == "panel_name")
        return config_.panelName;
    if (key == "show_splash")
        return config_.showSplash ? "true" : "false";
    if (key == "splash_duration") {
        static char buffer[16];
        snprintf(buffer, sizeof(buffer), "%d", config_.splashDuration);
        return buffer;
    }
    if (key == "theme")
        return config_.theme;
    if (key == "update_rate") {
        static char buffer[16];
        snprintf(buffer, sizeof(buffer), "%d", config_.updateRate);
        return buffer;
    }
    if (key == "pressure_unit")
        return config_.pressureUnit;
    if (key == "temp_unit")
        return config_.tempUnit;
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

/// @brief Set a preference value by key
/// @param key The preference key
/// @param value String representation of the value
void PreferenceManager::SetPreference(const std::string &key, const std::string &value)
{
    log_v("SetPreference() called");
    if (key == "panel_name")
    {
        config_.panelName = value;
    }
    else if (key == "show_splash")
    {
        config_.showSplash = (value == "true");
    }
    else if (key == "splash_duration")
    {
        config_.splashDuration = std::stoi(value);
    }
    else if (key == "theme")
    {
        config_.theme = value;
    }
    else if (key == "update_rate")
    {
        config_.updateRate = std::stoi(value);
    }
    else if (key == "pressure_unit")
    {
        config_.pressureUnit = value;
    }
    else if (key == "temp_unit")
    {
        config_.tempUnit = value;
    }
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

/// @brief Check if a preference exists
/// @param key The preference key
/// @return true if preference exists
bool PreferenceManager::HasPreference(const std::string &key) const
{
    log_v("HasPreference() called");
    bool hasKey = (key == "panel_name" || key == "show_splash" || key == "splash_duration" || key == "theme" ||
                   key == "update_rate" || key == "pressure_unit" || key == "temp_unit" ||
                   key == "pressure_offset" || key == "pressure_scale" || key == "temp_offset" || key == "temp_scale");
    return hasKey;
}

// Private Methods