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

    // Add config data to the JSON document - convert theme enum to string
    doc[JsonDocNames::theme] = theme_to_string(config.theme);
    doc[JsonDocNames::panel_name] = config.panel_name;

log_d("doc[JsonDocNames::panel_name]: %s", doc[JsonDocNames::panel_name]);

    // Serialize to JSON string
    String jsonString;
    serializeJson(doc, jsonString);

log_d("jsonString: %s", jsonString);

    // Save the JSON string to preferences
    return _preferences.putString(CONFIG_KEY, jsonString);
}

bool PreferenceManager::load_config()
{
    log_d("...");

    String jsonString = _preferences.getString(CONFIG_KEY, "");

    if (jsonString.length() == 0)
        return PreferenceManager::create_default_config();

    // Deserialize JSON using the new JsonDocument
    const size_t jsonCapacity = jsonString.length() * 2; // 2x for safety
    JsonDocument doc;

log_d("jsonString: %s", jsonString);
    DeserializationError result = deserializeJson(doc, jsonString);
log_d("doc[JsonDocNames::panel_name]: %s", doc[JsonDocNames::panel_name]);

    if (result != DeserializationError::Ok)
    {
        log_e("Error reading config:  %s", result.c_str());
        return PreferenceManager::create_default_config();
    };

    // Clear configs
    config = {};

    config.theme = Themes::Day; // Default theme
    if (!doc[JsonDocNames::theme].isNull())
    {
        const char *themeStr = doc[JsonDocNames::theme].as<const char *>();
        config.theme = string_to_theme(themeStr);
    }

    config.panel_name = PanelNames::Demo; // Default
    if (!doc[JsonDocNames::panel_name].isNull())
    {
        const char *panelStr = doc[JsonDocNames::panel_name].as<const char *>();
        config.panel_name = panelStr;
    }

    return true;
}

/// @brief Create and save a list of default panels
/// @return true if the save was successful
bool PreferenceManager::create_default_config()
{
    log_d("...");

    config = {.theme = Themes::Day,
              .panel_name = PanelNames::Demo};

    PreferenceManager::save_config();
    return PreferenceManager::load_config();
}
