#include "handlers/preference_manager.h"

void PreferenceManager::init()
{
    // Initialize preferences
    if (!_preferences.begin("clarity", false))
    { // false = read/write mode
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

const char *PreferenceManager::iteration_to_string(PanelIteration panel_iteration)
{
    switch (panel_iteration)
    {
    case PanelIteration::Infinite:
        return "Infinite";
    case PanelIteration::Disabled:
        return "Disabled";
    case PanelIteration::Once:
        return "Once";
    default:
        return "Unknown";
    }
}

/// @brief converts the name of an iteration enum to an element of the enum
/// @param name a string representing the name of the enum element
/// @return an element of the enum
PanelIteration PreferenceManager::string_to_iteration(const char *name)
{
    if (strcmp(name, "Infinite") == 0)
        return PanelIteration::Infinite;
    if (strcmp(name, "Disabled") == 0)
        return PanelIteration::Disabled;
    if (strcmp(name, "Once") == 0)
        return PanelIteration::Once;
    return PanelIteration::Disabled; // Default value
}

/// @brief Converts a Theme enum value to a string representation
/// @param theme The theme enum value to convert
/// @return A string representation of the theme
const char *PreferenceManager::theme_to_string(Theme theme)
{
    switch (theme)
    {
    case Theme::Light:
        return "Light";
    case Theme::Dark:
        return "Dark";
    default:
        return "Unknown";
    }
}

/// @brief Converts a string representation of a theme to a Theme enum value
/// @param str The string to convert
/// @return The corresponding Theme enum value
Theme PreferenceManager::string_to_theme(const char *str)
{
    if (strcmp(str, "Light") == 0)
        return Theme::Light;
    if (strcmp(str, "Dark") == 0)
        return Theme::Dark;
    return Theme::Dark; // Default value
}

bool PreferenceManager::save_config()
{
    log_d("...");
    _preferences.remove(CONFIG_KEY);

    const size_t panelCount = config.panels.size();
    // Use the new JsonDocument instead of the deprecated classes
    JsonDocument doc;

    // Add config data to the JSON document - convert theme enum to string
    doc["theme"] = theme_to_string(config.theme);

    // Create panels array using the new syntax
    doc["panels"] = JsonArray();
    JsonArray panelsArray = doc["panels"].to<JsonArray>();

    for (const auto &panel : config.panels)
    {
        // Add a new object to the array using the updated method
        JsonObject panelObj = panelsArray.add<JsonObject>();
        panelObj["name"] = panel.name;
        panelObj["iteration"] = iteration_to_string(panel.iteration);
    }

    // Serialize to JSON string
    String jsonString;
    serializeJson(doc, jsonString);

    // Save the JSON string to preferences
    return _preferences.putString(CONFIG_KEY, jsonString);
}

bool PreferenceManager::load_config()
{
    log_d("...");
    String jsonString = _preferences.getString(CONFIG_KEY, "");

    if (jsonString.length() > 0)
    {
        // Deserialize JSON using the new JsonDocument
        const size_t jsonCapacity = jsonString.length() * 2; // 2x for safety
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, jsonString);

        if (!error)
        {
            if (!doc["theme"].isNull()) {
                const char* themeStr = doc["theme"].as<const char*>();
                config.theme = string_to_theme(themeStr);
            }
            
            else
                config.theme = Theme::Dark; // Default theme

            // Process each panel
            JsonArray panelsArray = doc["panels"].as<JsonArray>();
            for (JsonObject panelObj : panelsArray)
            {
                PanelConfig panel;
                panel.name = panelObj["name"].as<const char *>();
                panel.iteration = string_to_iteration(panelObj["iteration"]);

                // Add to config
                config.panels.push_back(panel);
            }
            return true;
        }
        else
        {
            log_w("Error reading config");
            return PreferenceManager::create_default_config();
        };
    }
    
    else
        return PreferenceManager::create_default_config();
}

/// @brief Create and save a list of default panels
/// @return true if the save was successful
bool PreferenceManager::create_default_config()
{
    log_i("...");

    config = {.theme = Theme::Dark,
              .panels = {
                  {"SplashPanel", PanelIteration::Once},
                  {"DemoPanel", PanelIteration::Infinite}}};

    PreferenceManager::save_config();
    return PreferenceManager::load_config();
}
