#include "managers/preference_manager.h"
#include <esp32-hal-log.h>

// Static Methods removed - using dependency injection

// Core Functionality Methods

/// @brief Initialises the preference manager to handle application preferences_
void PreferenceManager::Init()
{
    log_d("Initializing preference manager and loading configuration from NVS");

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
        log_i("Preferences initialized successfully");

    LoadConfig();
}

/// @brief Create and save a list of default panels
/// @return true if the save was successful
void PreferenceManager::CreateDefaultConfig()
{
    log_d("Creating default configuration with OEM oil panel as default");

    config.panelName = PanelNames::OIL;

    PreferenceManager::SaveConfig();
    PreferenceManager::LoadConfig();
}

/// @brief Load the configuration from preferences_
/// @return true if the load was successful, false otherwise
void PreferenceManager::LoadConfig()
{
    log_d("Loading application configuration from NVS preferences");

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
#ifdef UNIT_TESTING
        log_w("Error deserializing config: %s", c_str(result));
#else
        log_w("Error deserializing config");
#endif
        return PreferenceManager::CreateDefaultConfig();
    }

    if (doc[JsonDocNames::PANEL_NAME].isNull())
    {
        log_w("Error reading config");
        return PreferenceManager::CreateDefaultConfig();
    }

    config.panelName = std::string(doc[JsonDocNames::PANEL_NAME].as<const char *>());
}

/// @brief Save the current configuration to preferences_
/// @return true if the save was successful, false otherwise
void PreferenceManager::SaveConfig()
{
    log_d("Saving current configuration to NVS preferences");

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

// IPreferenceService interface implementation

/// @brief Get the current configuration object
/// @return Reference to current configuration settings
Configs& PreferenceManager::GetConfig()
{
    return config;
}

/// @brief Get the current configuration object (read-only)
/// @return Const reference to current configuration settings
const Configs& PreferenceManager::GetConfig() const
{
    return config;
}

/// @brief Update the configuration object
/// @param newConfig New configuration settings to apply
void PreferenceManager::SetConfig(const Configs& newConfig)
{
    config = newConfig;
}

// Private Methods