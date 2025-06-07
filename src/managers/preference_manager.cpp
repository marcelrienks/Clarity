#include "managers/preference_manager.h"

/// @brief Get the singleton instance of PanelManager
/// @return instance of PanelManager
PreferenceManager &PreferenceManager::get_instance()
{
    static PreferenceManager instance; // this ensures that the instance is created only once
    return instance;
}

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

bool PreferenceManager::save_config()
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
    return written > 0;
}

bool PreferenceManager::load_config()
{
    log_d("...");

    String jsonString = _preferences.getString(CONFIG_KEY, "");
    if (jsonString.length() == 0)
    {
        log_d("No config found, creating default");
        return PreferenceManager::create_default_config();
    }

    // Deserialize JSON using the new JsonDocument
    JsonDocument doc;
    DeserializationError result = deserializeJson(doc, jsonString);
    if (result != DeserializationError::Ok)
    {
        log_e("Error deserializing config: %s", result.c_str());
        return PreferenceManager::create_default_config();
    }

    // Clear configs and set defaults
    config = {};
    config.panel_name = PanelNames::Demo; // Default
    if (!doc[JsonDocNames::panel_name].isNull())
    {
        // Convert to std::string - this copies the data
        config.panel_name = std::string(doc[JsonDocNames::panel_name].as<const char *>());
    }

    return true;
}

/// @brief Create and save a list of default panels
/// @return true if the save was successful
bool PreferenceManager::create_default_config()
{
    log_d("...");

    config = {.panel_name = std::string(PanelNames::Oil)};

    PreferenceManager::save_config();
    return PreferenceManager::load_config();
}