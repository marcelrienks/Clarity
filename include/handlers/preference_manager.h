#pragma once // preventing duplicate definitions, alternative to the traditional include guards

#include "utilities/types.h"
#include "utilities/serial_logger.h"

#include <Preferences.h>
#include <Arduino.h>
#include <vector>
#include <string>
#include <ArduinoJson.h>
#include <nvs_flash.h>

class PreferenceManager
{
private:
    inline static const char *CONFIG_KEY = "config";
    inline static const size_t JSON_CAPACITY_PER_PANEL = 256;

    Preferences _preferences;

    const char *iteration_to_string(PanelIteration iter);
    PanelIteration string_to_iteration(const char *str);

public:
    inline static Config config;

    PreferenceManager();
    ~PreferenceManager();

    void init();
    bool save_config();
    bool load_config();
    bool create_default_config();
};