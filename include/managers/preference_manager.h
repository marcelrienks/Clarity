#pragma once // preventing duplicate definitions, alternative to the traditional include guards

#include <Preferences.h>
#include <Arduino.h>
#include <vector>
#include <string>
#include "utilities/types.h"

struct PanelConfig {
    std::string type_name;
    PanelIteration iteration;
};

class PreferenceManager {
private:
    Preferences _preferences;
    const char* _namespace;
    bool _is_initialized;

public:
    PreferenceManager(const char* name = "clarity");
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