#include "managers/preference_manager.h"

// Static Methods

/// @brief Get the singleton instance of PreferenceManager
/// @return instance of PreferenceManager
PreferenceManager &PreferenceManager::GetInstance()
{
    static PreferenceManager instance; // this ensures that the instance is created only once
    return instance;
}

// Core Functionality Methods

/// @brief Initialises the preference manager to handle application preferences_
void PreferenceManager::init()
{
    log_d("...");

    // Initialize preferences_
    if (!preferences_.begin("clarity", false))
    {
        log_w("Failed to initialize preferences_, retrying after format");

        // Format NVS if opening failed
        nvs_flash_erase();

        // Try again
        if (!preferences_.begin("clarity", false))
            log_w("Failed to initialize preferences_ after format");

        else
            log_w("Preferences initialized successfully after format");
    }

    else
        log_i("Preferences initialized successfully");

    loadConfig();
}

/// @brief Create and save a list of default panels
/// @return true if the save was successful
void PreferenceManager::createDefaultConfig()
{
    log_d("...");

    config.panelName = PanelNames::OIL;

    PreferenceManager::saveConfig();
    PreferenceManager::loadConfig();
}

/// @brief Load the configuration from preferences_
/// @return true if the load was successful, false otherwise
void PreferenceManager::loadConfig()
{
    log_d("...");

    String jsonString = preferences_.getString(CONFIG_KEY, "");
    if (jsonString.length() == 0)
    {
        log_w("No config found, creating default");
        return PreferenceManager::createDefaultConfig();
    }

    JsonDocument doc;
    DeserializationError result = deserializeJson(doc, jsonString);
    if (result != DeserializationError::Ok)
    {
        log_w("Error deserializing config: %s", result.c_str());
        return PreferenceManager::createDefaultConfig();
    }

    if (doc[JsonDocNames::PANEL_NAME].isNull())
    {
        log_w("Error reading config");
        return PreferenceManager::createDefaultConfig();
    }

    config.panelName = std::string(doc[JsonDocNames::PANEL_NAME].as<const char *>());
}

/// @brief Save the current configuration to preferences_
/// @return true if the save was successful, false otherwise
void PreferenceManager::saveConfig()
{
    log_d("...");

    preferences_.remove(CONFIG_KEY);

    // Use the new JsonDocument instead of the deprecated classes
    JsonDocument doc;
    doc[JsonDocNames::PANEL_NAME] = config.panelName.c_str();

    // Serialize to JSON string
    String jsonString;
    serializeJson(doc, jsonString);

    // Save the JSON string to preferences_
    size_t written = preferences_.putString(CONFIG_KEY, jsonString);
}

// Private Methods