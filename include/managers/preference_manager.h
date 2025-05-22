#pragma once // preventing duplicate definitions, alternative to the traditional include guards

#include "utilities/types.h"
#include "managers/panel_manager.h"

#include <Preferences.h>
#include <nvs_flash.h>
#include <ArduinoJson.h>

class PreferenceManager
{
public:
    inline static Configs config;

    static PreferenceManager &get_instance();

    void init();
    bool save_config();
    bool load_config();
    bool create_default_config();

private:
    inline static const char *CONFIG_KEY = "config";
    inline static const char *THEME_KEY = "theme";
    inline static const size_t JSON_CAPACITY_PER_PANEL = 256;

    Preferences _preferences;
    const char *theme_to_string(Themes theme);
    Themes string_to_theme(const char *str);
};