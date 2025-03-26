#include "managers/preference_manager.h"
#include "utilities/serial_logger.h"

PreferenceManager::PreferenceManager(const char* name) : _namespace(name), _is_initialized(false) {}

PreferenceManager::~PreferenceManager() {
    end();
}

/// @brief Start the preferences configuration
/// @return True if initilisation was successful
bool PreferenceManager::begin() {
    _is_initialized = _preferences.begin(_namespace, false); // false = read/write mode
    return _is_initialized;
}

/// @brief End the preferences configuration
void PreferenceManager::end() {
    if (_is_initialized) {
        _preferences.end();
        _is_initialized = false;
    }
}

/// @brief Save serialised panel configuration
/// @param configs the panel configuration to be saved
/// @return True if the save was successful
bool PreferenceManager::save_panel_configs(const std::vector<PanelConfig> &configs) {
    if (!_is_initialized) {
        SerialLogger().log_point("PreferenceManager::savePanelConfigs", "Not initialized");
        return false;
    }
    
    // Serialize the config vector to a byte array
    std::vector<uint8_t> serialized_data = serialize_panel_configs(configs);
    
    // Store the serialized data
    bool success = _preferences.putBytes("panel_configs", serialized_data.data(), serialized_data.size());
    
    if (success) {
        SerialLogger().log_point("PreferenceManager::savePanelConfigs", 
                                "Saved " + std::to_string(configs.size()) + " panel configs (" + 
                                std::to_string(serialized_data.size()) + " bytes)");
    } else {
        SerialLogger().log_point("PreferenceManager::savePanelConfigs", "Failed to save panel configs");
    }
    
    return success;
}

/// @brief Load all the configured panels from Preferences
/// @return list of configured panels to be shown
std::vector<PanelConfig> PreferenceManager::load_panel_configs() {
    std::vector<PanelConfig> configs;
    
    if (!_is_initialized) {
        SerialLogger().log_point("PreferenceManager::loadPanelConfigs", "Not initialized");
        return configs;
    }
    
    // Get the size of the stored data
    size_t data_size = _preferences.getBytesLength("panel_configs");
    
    if (data_size == 0) {
        SerialLogger().log_point("PreferenceManager::loadPanelConfigs", "No panel configs found");
        return configs;
    }
    
    // Create a buffer to hold the serialized data
    std::vector<uint8_t> serialized_data(data_size);
    
    // Load the serialized data
    size_t read_size = _preferences.getBytes("panel_configs", serialized_data.data(), data_size);
    
    if (read_size != data_size) {
        SerialLogger().log_point("PreferenceManager::loadPanelConfigs", 
                               "Error reading panel configs: read " + std::to_string(read_size) + 
                               " of " + std::to_string(data_size) + " bytes");
        return configs;
    }
    
    // Deserialize the data
    configs = deserialize_panel_configs(serialized_data);
    
    SerialLogger().log_point("PreferenceManager::loadPanelConfigs", 
                           "Loaded " + std::to_string(configs.size()) + " panel configs");
    
    return configs;
}

/// @brief Clear all the panels from preferences
/// @return True if the clearing was successful
bool PreferenceManager::clear_panel_configs() {
    if (!_is_initialized) return false;
    return _preferences.remove("panel_configs");
}

/// @brief Create and save a list of default panels
/// @return true if the save was successful
bool PreferenceManager::save_default_panel_configs() {
    SerialLogger().log_point("PreferenceManager::save_default_panel_configs", "...");

    std::vector<PanelConfig> defaultConfigs = {
        {"SplashPanel", PanelIteration::Once},
        {"DemoPanel", PanelIteration::Infinite}
    };
    
    return save_panel_configs(defaultConfigs);
}

/// @brief Serialise panel configs for storage using preferences
/// @param configs the configurations to be stored in preferences
/// @return serialised data to be stored
std::vector<uint8_t> PreferenceManager::serialize_panel_configs(const std::vector<PanelConfig>& configs) {
    std::vector<uint8_t> data;
    
    // Add the number of configs (4 bytes)
    uint32_t count = configs.size();
    data.push_back((count >> 0) & 0xFF);
    data.push_back((count >> 8) & 0xFF);
    data.push_back((count >> 16) & 0xFF);
    data.push_back((count >> 24) & 0xFF);
    
    // Add each config
    for (const auto& config : configs) {
        // Add string length (1 byte, assuming panel names are short)
        uint8_t name_length = config.panel_name.length();
        data.push_back(name_length);
        
        // Add the panel name
        for (char c : config.panel_name) {
            data.push_back(static_cast<uint8_t>(c));
        }
        
        // Add the iteration value (1 byte)
        data.push_back(static_cast<uint8_t>(config.iteration));
    }
    
    return data;
}

/// @brief Deserialise Panel configurations
/// @param data the data retrieved from preferences
/// @return list of panel configs
std::vector<PanelConfig> PreferenceManager::deserialize_panel_configs(const std::vector<uint8_t>& data) {
    std::vector<PanelConfig> configs;
    
    // Check for minimum size (at least 4 bytes for the count)
    if (data.size() < 4) {
        SerialLogger().log_point("PreferenceManager::deserialize_panel_configs", 
                               "Data too small to contain panel configs");
        return configs;
    }
    
    // Read the number of configs
    uint32_t count = 0;
    count |= static_cast<uint32_t>(data[0]) << 0;
    count |= static_cast<uint32_t>(data[1]) << 8;
    count |= static_cast<uint32_t>(data[2]) << 16;
    count |= static_cast<uint32_t>(data[3]) << 24;
    
    // Sanity check on count
    if (count > 100) { // Arbitrary limit to prevent memory issues
        SerialLogger().log_point("PreferenceManager::deserialize_panel_configs", 
                               "Invalid config count: " + std::to_string(count));
        return configs;
    }
    
    // Parse each config
    size_t pos = 4; // Start after the count
    
    for (uint32_t i = 0; i < count && pos < data.size(); i++) {
        // Read string length
        uint8_t name_length = data[pos++];
        
        // Check if we have enough data for the name
        if (pos + name_length > data.size()) {
            SerialLogger().log_point("PreferenceManager::deserialize_panel_configs", 
                                   "Unexpected end of data while reading panel name");
            break;
        }
        
        // Read the panel name
        std::string panel_name;
        for (uint8_t j = 0; j < name_length; j++) {
            panel_name += static_cast<char>(data[pos++]);
        }
        
        // Check if we have enough data for the iteration value
        if (pos >= data.size()) {
            SerialLogger().log_point("PreferenceManager::deserialize_panel_configs", 
                                   "Unexpected end of data while reading iteration value");
            break;
        }
        
        // Read the iteration value
        PanelIteration iteration = static_cast<PanelIteration>(data[pos++]);
        
        // Add the config
        configs.push_back({panel_name, iteration});
    }
    
    return configs;
}