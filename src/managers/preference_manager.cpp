#include "managers/preference_manager.h"

// Static Methods

/// @brief Get the singleton instance of PreferenceManager
/// @return instance of PreferenceManager
PreferenceManager &PreferenceManager::get_instance()
{
    static PreferenceManager instance; // this ensures that the instance is created only once
    return instance;
}

// Core Functionality Methods

/// @brief Initialises the preference manager to handle application preferences
void PreferenceManager::init()
{
    log_d("...");

    // Initialize preferences
    if (!_preferences.begin("clarity", false))
    {
        log_w("Failed to initialize preferences, retrying after format");

        // Format NVS if opening failed
        nvs_flash_erase();

        // Try again
        if (!_preferences.begin("clarity", false))
            log_w("Failed to initialize preferences after format");

        else
            log_w("Preferences initialized successfully after format");
    }

    else
        log_i("Preferences initialized successfully");

    load_config();
}

/// @brief Create and save a list of default panels
/// @return true if the save was successful
void PreferenceManager::create_default_config()
{
    log_d("...");

    config = {.panel_name = std::string(PanelNames::Oil)};

    PreferenceManager::save_config();
    PreferenceManager::load_config();
}

/// @brief Load the configuration from preferences
/// @return true if the load was successful, false otherwise
void PreferenceManager::load_config()
{
    log_d("...");

    String jsonString = _preferences.getString(CONFIG_KEY, "");
    if (jsonString.length() == 0)
    {
        log_w("No config found, creating default");
        return PreferenceManager::create_default_config();
    }

    JsonDocument doc;
    DeserializationError result = deserializeJson(doc, jsonString);
    if (result != DeserializationError::Ok)
    {
        log_w("Error deserializing config: %s", result.c_str());
        return PreferenceManager::create_default_config();
    }

    if (doc[JsonDocNames::panel_name].isNull())
    {
        log_w("Error reading config");
        return PreferenceManager::create_default_config();
    }

    config.panel_name = std::string(doc[JsonDocNames::panel_name].as<const char *>());
    log_i("Preferences loaded successfully: %s", config.panel_name.c_str());
}

/// @brief Save the current configuration to preferences
/// @return true if the save was successful, false otherwise
void PreferenceManager::save_config()
{
    log_d("...");

    _preferences.remove(CONFIG_KEY);

    // Use the new JsonDocument instead of the deprecated classes
    JsonDocument doc;
    doc[JsonDocNames::panel_name] = config.panel_name.c_str();

    // Serialize to JSON string
    String jsonString;
    serializeJson(doc, jsonString);

    // Save the JSON string to preferences
    size_t written = _preferences.putString(CONFIG_KEY, jsonString);
}

// Private Methods

/// @brief Converts a string representation of a theme to a Theme enum value
/// @param str The string to convert
/// @return The corresponding Theme enum value
Themes PreferenceManager::string_to_theme(const char *string)
{
    if (strcmp(string, "Day") == 0)
        return Themes::Day;

    if (strcmp(string, "Night") == 0)
        return Themes::Night;

    return Themes::Day;
}

/// @brief Converts a Theme enum value to a string representation
/// @param theme The theme enum value to convert
/// @return A string representation of the theme
const char *PreferenceManager::theme_to_string(Themes theme)
{
    switch (theme)
    {
    case Themes::Day:
        return "Day";

    case Themes::Night:
        return "Night";

    default:
        return "Day";
    }
}