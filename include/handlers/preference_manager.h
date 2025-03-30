#pragma once // preventing duplicate definitions, alternative to the traditional include guards

#include <Preferences.h>
#include <Arduino.h>
#include <vector>
#include <string>
#include "utilities/types.h"

class PreferenceManager {
private:
    Preferences _preferences;
    const char *_namespace;
    bool _is_initialized;
    
    // Serialization helpers
    std::vector<uint8_t> serialize_panel_configs(const std::vector<PanelConfig>& configs);
    std::vector<PanelConfig> deserialize_panel_configs(const std::vector<uint8_t>& data);

public:
    PreferenceManager(const char *name = "clarity");
    ~PreferenceManager();
    
    bool begin();
    void end();
    
    // Panel configuration methods
    bool save_panel_configs(const std::vector<PanelConfig> &configs);
    std::vector<PanelConfig> load_panel_configs();
    
    // Helper methods
    bool clear_panel_configs();
    bool save_default_panel_configs();
};