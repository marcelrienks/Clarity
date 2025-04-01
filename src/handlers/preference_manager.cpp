#include "handlers/preference_manager.h"

PreferenceManager::PreferenceManager()
{
    SerialLogger().log_point("PreferenceManager::PreferenceManager()", "...");
}

PreferenceManager::~PreferenceManager()
{
    _preferences.end();
}

void PreferenceManager::init()
{
    SerialLogger().log_point("PreferenceManager::Init()", "...");

    // Initialize preferences
    if (!_preferences.begin("clarity", false)) { // false = read/write mode
        SerialLogger().log_point("PreferenceManager::init()", "Failed to initialize preferences, retrying after format");
        
        // Format NVS if opening failed
        nvs_flash_erase();
        
        // Try again
        if (!_preferences.begin("clarity", false))
            SerialLogger().log_point("PreferenceManager::init()", "Failed to initialize preferences after format");
        else
            SerialLogger().log_point("PreferenceManager::init()", "Preferences initialized successfully after format");
    } else {
        SerialLogger().log_point("PreferenceManager::init()", "Preferences initialized successfully");
    }
    
    load_config();
}

/// @brief converts an element of the iteration enum to it's equivelent string
/// @param panel_iteration the item from the enum to convert
/// @return a string representing the name of the enum element
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

bool PreferenceManager::save_config()
{
    SerialLogger().log_point("PreferenceManager::save_config", "...");
    _preferences.remove(CONFIG_KEY);

    const size_t panelCount = config.panels.size();
    // Use the new JsonDocument instead of the deprecated classes
    JsonDocument doc;

    // Add config data to the JSON document
    doc["theme"] = config.theme;

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
    SerialLogger().log_point("PreferenceManager::load_config", "...");
    String jsonString = _preferences.getString(CONFIG_KEY, "");

    if (jsonString.length() > 0)
    {
        // Deserialize JSON using the new JsonDocument
        const size_t jsonCapacity = jsonString.length() * 2; // 2x for safety
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, jsonString);

        if (!error)
        {
            // Get all the config values
            config.theme = doc["theme"];

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
            SerialLogger::log_point("PreferenceManager::load_config", "Error reading config");
            return PreferenceManager::create_default_config();
        };
    }
    else
    {
        // No existing config found, create default
        return PreferenceManager::create_default_config();
    }
}

/// @brief Create and save a list of default panels
/// @return true if the save was successful
bool PreferenceManager::create_default_config()
{
    SerialLogger().log_point("PreferenceManager::create_default_configs", "...");

    config = {.theme = Theme::Dark,
              .panels = {
                  {"SplashPanel", PanelIteration::Once},
                  {"DemoPanel", PanelIteration::Infinite}}};

    PreferenceManager::save_config();
    return PreferenceManager::load_config();
}
