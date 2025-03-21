// src/managers/preference_manager.cpp
#include "managers/preference_manager.h"
#include "utilities/serial_logger.h"

PreferenceManager::PreferenceManager(const char* name) : _namespace(name), _is_initialized(false) {}

PreferenceManager::~PreferenceManager() {
    end();
}

bool PreferenceManager::begin() {
    _is_initialized = _preferences.begin(_namespace, false); // false = read/write mode
    return _is_initialized;
}

void PreferenceManager::end() {
    if (_is_initialized) {
        _preferences.end();
        _is_initialized = false;
    }
}

bool PreferenceManager::save_panel_configs(const std::vector<PanelConfig> &configs) {
    if (!_is_initialized) {
        SerialLogger().log_point("PreferenceManager::savePanelConfigs", "Not initialized");
        return false;
    }
    
    // Clear existing configuration
    _preferences.remove("panel_count");
    
    // Save new configuration
    _preferences.putUInt("panel_count", configs.size());
    
    for (size_t i = 0; i < configs.size(); i++) {
        std::string key_name = "panel_" + std::to_string(i) + "_name";
        std::string key_iteration = "panel_" + std::to_string(i) + "_iteration";
        
        _preferences.putString(key_name.c_str(), configs[i].type_name.c_str());
        _preferences.putUChar(key_iteration.c_str(), static_cast<uint8_t>(configs[i].iteration));
    }
    
    return true;
}

std::vector<PanelConfig> PreferenceManager::load_panel_configs() {
    std::vector<PanelConfig> configs;
    
    if (!_is_initialized) {
        SerialLogger().log_point("PreferenceManager::loadPanelConfigs", "Not initialized");
        return configs;
    }
    
    uint32_t panelCount = _preferences.getUInt("panel_count", 0);
    
    for (uint32_t i = 0; i < panelCount; i++) {
        std::string key_name = "panel_" + std::to_string(i) + "_name";
        std::string key_iteration = "panel_" + std::to_string(i) + "_iteration";
        
        PanelConfig config;
        config.type_name = _preferences.getString(key_name.c_str(), "").c_str();
        config.iteration = static_cast<PanelIteration>(_preferences.getUChar(key_iteration.c_str(), 
            static_cast<uint8_t>(PanelIteration::Infinite)));
        
        configs.push_back(config);
    }
    
    return configs;
}

bool PreferenceManager::clear_panel_configs() {
    if (!_is_initialized) return false;
    return _preferences.remove("panel_count");
}

bool PreferenceManager::save_default_panel_configs() {
    std::vector<PanelConfig> defaultConfigs = {
        {"SplashPanel", PanelIteration::Once},
        {"DemoPanel", PanelIteration::Infinite}
    };
    
    return save_panel_configs(defaultConfigs);
}