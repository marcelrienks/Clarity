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
    log_d("Creating default configuration with OEM oil panel as default");

    config.panelName = PanelNames::OIL;

    PreferenceManager::SaveConfig();
    PreferenceManager::LoadConfig();
}

/// @brief Load the configuration from preferences_
/// @return true if the load was successful, false otherwise
void PreferenceManager::LoadConfig()
{
    log_v("LoadConfig() called");
    log_v("LoadConfig() called");

    String jsonString = preferences_.getString(CONFIG_KEY, "");
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

    config.panelName = std::string(doc[JsonDocNames::PANEL_NAME].as<const char *>());

    // Load all settings with defaults if not present
    if (!doc[JsonDocNames::SHOW_SPLASH].isNull())
    {
        config.showSplash = doc[JsonDocNames::SHOW_SPLASH].as<bool>();
    }

    if (!doc[JsonDocNames::SPLASH_DURATION].isNull())
    {
        config.splashDuration = doc[JsonDocNames::SPLASH_DURATION].as<int>();
    }

    if (!doc[JsonDocNames::THEME].isNull())
    {
        config.theme = std::string(doc[JsonDocNames::THEME].as<const char *>());
    }

    if (!doc[JsonDocNames::UPDATE_RATE].isNull())
    {
        config.updateRate = doc[JsonDocNames::UPDATE_RATE].as<int>();
    }

    if (!doc[JsonDocNames::PRESSURE_UNIT].isNull())
    {
        config.pressureUnit = std::string(doc[JsonDocNames::PRESSURE_UNIT].as<const char *>());
    }

    if (!doc[JsonDocNames::TEMP_UNIT].isNull())
    {
        config.tempUnit = std::string(doc[JsonDocNames::TEMP_UNIT].as<const char *>());
    }

    // Load calibration settings
    if (!doc[JsonDocNames::PRESSURE_OFFSET].isNull())
    {
        config.pressureOffset = doc[JsonDocNames::PRESSURE_OFFSET].as<float>();
    }
    if (!doc[JsonDocNames::PRESSURE_SCALE].isNull())
    {
        config.pressureScale = doc[JsonDocNames::PRESSURE_SCALE].as<float>();
    }
    if (!doc[JsonDocNames::TEMP_OFFSET].isNull())
    {
        config.tempOffset = doc[JsonDocNames::TEMP_OFFSET].as<float>();
    }
    if (!doc[JsonDocNames::TEMP_SCALE].isNull())
    {
        config.tempScale = doc[JsonDocNames::TEMP_SCALE].as<float>();
    }
}

/// @brief Save the current configuration to preferences_
/// @return true if the save was successful, false otherwise
void PreferenceManager::SaveConfig()
{
    log_v("SaveConfig() called");
    log_v("SaveConfig() called");

    preferences_.remove(CONFIG_KEY);

    // Use the new JsonDocument instead of the deprecated classes
    JsonDocument doc;
    doc[JsonDocNames::PANEL_NAME] = config.panelName.c_str();
    doc[JsonDocNames::SHOW_SPLASH] = config.showSplash;
    doc[JsonDocNames::SPLASH_DURATION] = config.splashDuration;
    doc[JsonDocNames::THEME] = config.theme.c_str();
    doc[JsonDocNames::UPDATE_RATE] = config.updateRate;
    doc[JsonDocNames::PRESSURE_UNIT] = config.pressureUnit.c_str();
    doc[JsonDocNames::TEMP_UNIT] = config.tempUnit.c_str();
    
    // Serialize calibration settings
    doc[JsonDocNames::PRESSURE_OFFSET] = config.pressureOffset;
    doc[JsonDocNames::PRESSURE_SCALE] = config.pressureScale;
    doc[JsonDocNames::TEMP_OFFSET] = config.tempOffset;
    doc[JsonDocNames::TEMP_SCALE] = config.tempScale;

    // Serialize to JSON string
    String jsonString;
    serializeJson(doc, jsonString);

    log_d("Serialized config JSON: %s", jsonString.c_str());

    // Save the JSON string to preferences_
    size_t written = preferences_.putString(CONFIG_KEY, jsonString);
    if (written > 0)
    {
        log_i("Configuration saved successfully (%zu bytes written)", written);
        // Commit changes to NVS - this is critical for persistence across reboots
        preferences_.end();
        // Reopen preferences for future operations
        preferences_.begin(SystemConstants::PREFERENCES_NAMESPACE, false);
        log_i("System configuration updated and persisted to NVS storage");
    }
    else
    {
        log_e("Failed to save configuration to NVS");
    }
}

// IPreferenceService interface implementation

/// @brief Get the current configuration object
/// @return Reference to current configuration settings
Configs &PreferenceManager::GetConfig()
{
    log_v("GetConfig() called");
    return config;
}

/// @brief Get the current configuration object (read-only)
/// @return Const reference to current configuration settings
const Configs &PreferenceManager::GetConfig() const
{
    log_v("GetConfig() const called");
    return config;
}

/// @brief Update the configuration object
/// @param newConfig New configuration settings to apply
void PreferenceManager::SetConfig(const Configs &newConfig)
{
    log_v("SetConfig() called");
    log_d("Setting configuration - panel: %s, theme: %s, showSplash: %s, updateRate: %d", 
          newConfig.panelName.c_str(), newConfig.theme.c_str(), 
          newConfig.showSplash ? "true" : "false", newConfig.updateRate);
    config = newConfig;
}

// Generic preference access methods for dynamic menus

/// @brief Get a preference value by key
/// @param key The preference key
/// @return String representation of the value
std::string PreferenceManager::GetPreference(const std::string &key) const
{
    log_v("GetPreference() called");
    if (key == "panel_name")
        return config.panelName;
    if (key == "show_splash")
        return config.showSplash ? "true" : "false";
    if (key == "splash_duration")
        return std::to_string(config.splashDuration);
    if (key == "theme")
        return config.theme;
    if (key == "update_rate")
        return std::to_string(config.updateRate);
    if (key == "pressure_unit")
        return config.pressureUnit;
    if (key == "temp_unit")
        return config.tempUnit;
    if (key == "pressure_offset")
        return std::to_string(config.pressureOffset);
    if (key == "pressure_scale")
        return std::to_string(config.pressureScale);
    if (key == "temp_offset")
        return std::to_string(config.tempOffset);
    if (key == "temp_scale")
        return std::to_string(config.tempScale);

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
        config.panelName = value;
    }
    else if (key == "show_splash")
    {
        config.showSplash = (value == "true");
    }
    else if (key == "splash_duration")
    {
        config.splashDuration = std::stoi(value);
    }
    else if (key == "theme")
    {
        config.theme = value;
    }
    else if (key == "update_rate")
    {
        config.updateRate = std::stoi(value);
    }
    else if (key == "pressure_unit")
    {
        config.pressureUnit = value;
    }
    else if (key == "temp_unit")
    {
        config.tempUnit = value;
    }
    else if (key == "pressure_offset")
    {
        config.pressureOffset = std::stof(value);
    }
    else if (key == "pressure_scale")
    {
        config.pressureScale = std::stof(value);
    }
    else if (key == "temp_offset")
    {
        config.tempOffset = std::stof(value);
    }
    else if (key == "temp_scale")
    {
        config.tempScale = std::stof(value);
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
    log_d("Checking preference key: %s, exists: %s", key.c_str(), hasKey ? "true" : "false");
    return hasKey;
}

// Private Methods