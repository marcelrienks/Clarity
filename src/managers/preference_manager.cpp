#include "managers/preference_manager.h"
#include "managers/error_manager.h"
#include "utilities/logging.h"
#include <algorithm>
#include <sstream>

// Helper class for RAII mutex handling
class SemaphoreGuard {
private:
    SemaphoreHandle_t& semaphore_;
public:
    SemaphoreGuard(SemaphoreHandle_t& sem) : semaphore_(sem) {
        xSemaphoreTake(semaphore_, portMAX_DELAY);
    }
    ~SemaphoreGuard() {
        xSemaphoreGive(semaphore_);
    }
};

// ========== Constructor ==========

PreferenceManager::PreferenceManager() {
    // Initialize thread safety semaphore
    configMutex_ = xSemaphoreCreateMutex();
    if (!configMutex_) {
        log_e("Failed to create config mutex");
        ErrorManager::Instance().ReportCriticalError("PreferenceManager",
                                                     "Failed to create config mutex - thread safety compromised");
        return;
    }

    // Initialize NVS
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        nvs_flash_erase();
        err = nvs_flash_init();
    }

    if (err != ESP_OK) {
        log_e("Failed to initialize NVS: %s", esp_err_to_name(err));
        ErrorManager::Instance().ReportCriticalError("PreferenceManager",
                                                     "Failed to initialize NVS - preferences cannot be saved");
        return;
    }

    // Create default sections
    CreateDefaultSections();

    // Load all registered sections
    LoadAllConfigSections();

    log_i("PreferenceManager initialized");
}

// ========== IPreferenceManager Implementation ==========


/**
 * @brief Save current configuration to persistent storage
 *
 * Legacy interface method that delegates to SaveAllConfigSections().
 * Provides backward compatibility while using the new sectioned storage system.
 */
void PreferenceManager::SaveConfig() {
    log_v("SaveConfig() called");
    SemaphoreGuard lock(configMutex_);
    SaveAllConfigSections();
}

/**
 * @brief Load configuration from persistent storage
 *
 * Legacy interface method that delegates to LoadAllConfigSections().
 * Restores all registered configuration sections from their respective NVS namespaces.
 */
void PreferenceManager::LoadConfig() {
    log_v("LoadConfig() called");
    SemaphoreGuard lock(configMutex_);
    LoadAllConfigSections();
}

/**
 * @brief Create default configuration if none exists
 *
 * Creates default system configuration sections and saves them to storage.
 * Called during first boot or when configuration reset is requested.
 */
void PreferenceManager::CreateDefaultConfig() {
    log_v("CreateDefaultConfig() called");
    SemaphoreGuard lock(configMutex_);
    CreateDefaultSections();
    SaveAllConfigSections();
    log_i("Default configuration created");
}


/**
 * @brief Get a preference value by key
 * @param key The preference key (dot-separated format)
 * @return String representation of the value
 *
 * Legacy string-based interface that delegates to the type-safe QueryConfig system.
 * Automatically converts the stored ConfigValue to string representation.
 */
std::string PreferenceManager::GetPreference(const std::string& key) const {
    log_v("GetPreference() called");
    SemaphoreGuard lock(configMutex_);
    if (auto value = QueryConfigImpl(key)) {
        return Config::ConfigValueHelper::ToString(*value);
    }
    return "";
}

/**
 * @brief Set a preference value by key
 * @param key The preference key (dot-separated format)
 * @param value String representation of the value
 *
 * Legacy string-based interface that delegates to the type-safe UpdateConfig system.
 * Automatically converts the string value to appropriate ConfigValue type.
 */
void PreferenceManager::SetPreference(const std::string& key, const std::string& value) {
    log_v("SetPreference() called");
    SemaphoreGuard lock(configMutex_);
    UpdateConfigImpl(key, Config::ConfigValue(value));
}

/**
 * @brief Check if a preference exists
 * @param key The preference key to check
 * @return true if preference exists in any registered section
 *
 * Queries the dynamic configuration system to determine if the specified
 * key exists in any registered configuration section.
 */
bool PreferenceManager::HasPreference(const std::string& key) const {
    SemaphoreGuard lock(configMutex_);
    return QueryConfigImpl(key).has_value();
}

// ========== Dynamic Configuration Implementation ==========

/**
 * @brief Register a configuration section for component self-registration
 * @param section ConfigSection containing items, metadata, and validation rules
 * @return true if registration successful, false if section already exists
 *
 * Implements component self-registration pattern from dynamic-config-implementation.md.
 * Components call this during initialization to register their configuration requirements.
 * The section defines configuration items with types, default values, validation constraints,
 * and UI metadata for automatic menu generation.
 *
 * After registration, any existing persisted values are automatically loaded from NVS,
 * allowing components to immediately access their stored configuration.
 */
bool PreferenceManager::RegisterConfigSection(const Config::ConfigSection& section) {
    log_v("RegisterConfigSection() called");
    SemaphoreGuard lock(configMutex_);

    // Check if section already exists - prevent duplicate registration
    if (registeredSections_.find(section.sectionName) != registeredSections_.end()) {
        log_w("Section '%s' already registered", section.sectionName.c_str());
        return false;
    }

    // Store section definition in memory for later access
    registeredSections_[section.sectionName] = section;
    log_i("Registered config section: %s", section.sectionName.c_str());

    // Load any existing persisted values for this section from NVS
    // This allows components to register config and immediately access stored values
    LoadConfigSection(section.sectionName);

    return true;
}

/**
 * @brief Get list of all registered configuration section names
 * @return Vector of section names for UI generation and iteration
 *
 * Used primarily by ConfigPanel for automatic menu generation. The returned
 * list enables dynamic UI creation where menus are built based on what
 * components have registered, rather than hardcoded menu structures.
 */
std::vector<std::string> PreferenceManager::GetRegisteredSectionNames() const {
    SemaphoreGuard lock(configMutex_);
    std::vector<std::string> names;
    names.reserve(registeredSections_.size());

    for (const auto& [name, section] : registeredSections_) {
        names.push_back(name);
    }

    return names;
}

/**
 * @brief Retrieve a specific configuration section by name
 * @param sectionName Name of the section to retrieve
 * @return Optional containing section if found, nullopt if not registered
 *
 * Returns the complete section definition including current values and metadata.
 * Used by ConfigPanel to build configuration UIs and by components to access
 * their registered configuration structure.
 */
std::optional<Config::ConfigSection> PreferenceManager::GetConfigSection(const std::string& sectionName) const {
    SemaphoreGuard lock(configMutex_);

    auto it = registeredSections_.find(sectionName);
    if (it != registeredSections_.end()) {
        return it->second;
    }

    return std::nullopt;
}


/**
 * @brief Save a specific configuration section to NVS storage
 * @param sectionName Name of the section to persist
 * @return true if save operation successful
 *
 * Persists all configuration items in the specified section to sectioned NVS storage.
 * Each section gets its own NVS namespace for organization. Used when components
 * want to save their specific configuration without affecting other sections.
 */
bool PreferenceManager::SaveConfigSection(const std::string& sectionName) {
    auto it = registeredSections_.find(sectionName);
    if (it == registeredSections_.end()) {
        log_w("Section not found: %s", sectionName.c_str());
        return false;
    }

    const Config::ConfigSection& section = it->second;
    std::string nsName = GetSectionNamespace(sectionName);

    preferences_.begin(nsName.c_str(), false);
    bool success = true;

    for (const auto& item : section.items) {
        if (!StoreValueToNVS(preferences_, item.key, item.value, item.type)) {
            log_e("Failed to store config item: %s.%s", sectionName.c_str(), item.key.c_str());
            ErrorManager::Instance().ReportError(ErrorLevel::ERROR, "PreferenceManager",
                                                "Failed to store config item to NVS");
            success = false;
        }
    }

    preferences_.end();

    return success;
}

/**
 * @brief Load a specific configuration section from NVS storage
 * @param sectionName Name of the section to load
 * @return true if load operation successful
 *
 * Loads persisted values from sectioned NVS storage into the in-memory section definition.
 * Called automatically after component registration to restore previous configuration.
 * Updates the registered section with stored values while preserving metadata.
 */
bool PreferenceManager::LoadConfigSection(const std::string& sectionName) {
    auto it = registeredSections_.find(sectionName);
    if (it == registeredSections_.end()) {
        return false;
    }

    Config::ConfigSection& section = it->second;
    std::string nsName = GetSectionNamespace(sectionName);

    preferences_.begin(nsName.c_str(), true); // read-only

    for (auto& item : section.items) {
        item.value = LoadValueFromNVS(preferences_, item.key, item.type);
    }

    preferences_.end();
    return true;
}

/**
 * @brief Save all registered configuration sections to storage
 * @return true if all sections saved successfully
 *
 * Batch operation for complete configuration persistence. Iterates through all
 * registered sections and saves each one to its respective NVS namespace.
 * Used during application shutdown or when performing complete configuration backup.
 */
bool PreferenceManager::SaveAllConfigSections() {
    bool allSuccess = true;
    for (const auto& [name, section] : registeredSections_) {
        if (!SaveConfigSection(name)) {
            allSuccess = false;
        }
    }
    return allSuccess;
}

/**
 * @brief Load all registered configuration sections from storage
 * @return true if all sections loaded successfully
 *
 * Batch operation for complete configuration restoration. Iterates through all
 * registered sections and loads persisted values from their respective NVS namespaces.
 * Called during application startup to restore previous configuration state.
 */
bool PreferenceManager::LoadAllConfigSections() {
    bool allSuccess = true;
    for (const auto& [name, section] : registeredSections_) {
        if (!LoadConfigSection(name)) {
            allSuccess = false;
        }
    }
    return allSuccess;
}

bool PreferenceManager::ValidateConfigValue(const std::string& fullKey, const Config::ConfigValue& value) const {
    auto [sectionName, itemKey] = ParseConfigKey(fullKey);

    auto sectionIt = registeredSections_.find(sectionName);
    if (sectionIt == registeredSections_.end()) {
        return false;
    }

    const Config::ConfigSection& section = sectionIt->second;
    auto itemIt = std::find_if(section.items.begin(), section.items.end(),
        [&itemKey](const Config::ConfigItem& item) { return item.key == itemKey; });

    if (itemIt == section.items.end()) {
        return false;
    }

    const Config::ConfigItem& item = *itemIt;

    // Type validation
    if (value.index() != static_cast<size_t>(item.type)) {
        log_w("Type mismatch for key %s", fullKey.c_str());
        return false;
    }

    // Range validation based on metadata
    switch (item.type) {
        case Config::ConfigValueType::Integer:
            if (auto val = Config::ConfigValueHelper::GetValue<int>(value)) {
                return ValidateIntRange(*val, item.metadata.constraints);
            }
            break;

        case Config::ConfigValueType::Float:
            if (auto val = Config::ConfigValueHelper::GetValue<float>(value)) {
                return ValidateFloatRange(*val, item.metadata.constraints);
            }
            break;

        case Config::ConfigValueType::Enum:
            if (auto val = Config::ConfigValueHelper::GetValue<std::string>(value)) {
                return ValidateEnumValue(*val, item.metadata.constraints);
            }
            break;

        case Config::ConfigValueType::Boolean:
        case Config::ConfigValueType::String:
            return true; // No additional validation needed
    }

    return false;
}



// ========== Live Update Implementation ==========

uint32_t PreferenceManager::RegisterChangeCallback(const std::string& fullKey, ConfigChangeCallback callback) {
    SemaphoreGuard lock(configMutex_);
    uint32_t callbackId = nextCallbackId_++;
    changeCallbacks_[callbackId] = std::make_pair(fullKey, callback);
    return callbackId;
}







// ========== Protected Implementation Methods ==========

std::optional<Config::ConfigValue> PreferenceManager::QueryConfigImpl(const std::string& fullKey) const {
    // Parse dot-separated key into section and item (e.g., "oil_temp_sensor.unit")
    auto [sectionName, itemKey] = ParseConfigKey(fullKey);

    // Find the registered section by name
    auto sectionIt = registeredSections_.find(sectionName);
    if (sectionIt == registeredSections_.end()) {
        return std::nullopt; // Section not registered
    }

    // Search for the specific configuration item within the section
    const Config::ConfigSection& section = sectionIt->second;
    auto itemIt = std::find_if(section.items.begin(), section.items.end(),
        [&itemKey](const Config::ConfigItem& item) { return item.key == itemKey; });

    if (itemIt == section.items.end()) {
        return std::nullopt; // Item not found in section
    }

    // Return the current value stored in memory
    return itemIt->value;
}

bool PreferenceManager::UpdateConfigImpl(const std::string& fullKey, const Config::ConfigValue& value) {
    // Parse dot-separated key into section and item components
    auto [sectionName, itemKey] = ParseConfigKey(fullKey);

    // Find the registered section
    auto sectionIt = registeredSections_.find(sectionName);
    if (sectionIt == registeredSections_.end()) {
        log_w("Section not found for key: %s", fullKey.c_str());
        return false;
    }

    // Find the specific configuration item within the section
    Config::ConfigSection& section = sectionIt->second;
    auto itemIt = std::find_if(section.items.begin(), section.items.end(),
        [&itemKey](Config::ConfigItem& item) { return item.key == itemKey; });

    if (itemIt == section.items.end()) {
        log_w("Item not found for key: %s", fullKey.c_str());
        return false;
    }

    // Store old value for change notifications
    std::optional<Config::ConfigValue> oldValue = itemIt->value;

    // Validate new value against metadata constraints (range, enum options, etc.)
    if (!ValidateConfigValue(fullKey, value)) {
        log_w("Validation failed for key: %s", fullKey.c_str());
        return false;
    }

    // Update in-memory value
    itemIt->value = value;

    // Persist to sectioned NVS storage - each section gets its own namespace
    std::string nsName = GetSectionNamespace(sectionName);
    preferences_.begin(nsName.c_str(), false); // false = read/write mode
    bool success = StoreValueToNVS(preferences_, itemKey, value, itemIt->type);
    preferences_.end();

    if (success) {
        log_i("Updated config %s = %s", fullKey.c_str(),
              Config::ConfigValueHelper::ToString(value).c_str());

        // Notify all registered callbacks about the configuration change
        // Callbacks can watch specific keys or all changes (empty watchedKey)
        for (const auto& [callbackId, keyCallbackPair] : changeCallbacks_) {
            const std::string& watchedKey = keyCallbackPair.first;
            const ConfigChangeCallback& callback = keyCallbackPair.second;

            // Check if callback watches this key specifically or all changes
            if (watchedKey.empty() || watchedKey == fullKey) {
                try {
                    // Call registered callback with old and new values
                    callback(fullKey, oldValue, value);
                    log_t("Notified callback %u for update: %s", callbackId, fullKey.c_str());
                } catch (const std::exception& e) {
                    log_e("Exception in change callback %u: %s", callbackId, e.what());
                    ErrorManager::Instance().ReportError(ErrorLevel::ERROR, "PreferenceManager",
                                                        "Exception in change callback");
                }
            }
        }
    } else {
        // Rollback in-memory value on NVS write failure
        itemIt->value = *oldValue;
        log_e("Failed to save config to NVS for key: %s", fullKey.c_str());
        ErrorManager::Instance().ReportError(ErrorLevel::ERROR, "PreferenceManager",
                                            "Failed to save config value to NVS");
    }

    return success;
}

// ========== Private Helper Methods ==========

std::pair<std::string, std::string> PreferenceManager::ParseConfigKey(const std::string& fullKey) const {
    size_t dotPos = fullKey.find('.');
    if (dotPos == std::string::npos) {
        return {"", fullKey};
    }
    return {fullKey.substr(0, dotPos), fullKey.substr(dotPos + 1)};
}

std::string PreferenceManager::GetSectionNamespace(const std::string& sectionName) const {
    std::string ns = SECTION_PREFIX_ + sectionName;
    if (ns.length() > MAX_NAMESPACE_LEN_) {
        ns = ns.substr(0, MAX_NAMESPACE_LEN_);
    }
    return ns;
}

void PreferenceManager::CreateDefaultSections() {
    using namespace Config;

    // System section
    ConfigSection systemSection("System", "system", "System Settings");
    systemSection.displayOrder = 0;
    systemSection.AddItem(ConfigItem("panel_name", "Panel Name", ConfigValueType::String,
        std::string("OEM Oil"), ConfigMetadata()));
    systemSection.AddItem(ConfigItem("show_splash", "Show Splash Screen", ConfigValueType::Boolean,
        true, ConfigMetadata()));
    systemSection.AddItem(ConfigItem("dynamic_ui_enabled", "Dynamic UI", ConfigValueType::Boolean,
        true, ConfigMetadata()));

    registeredSections_["system"] = systemSection;

    log_i("Created default configuration sections");
}


bool PreferenceManager::ValidateIntRange(int value, const std::string& constraints) const {
    if (constraints.empty()) return true;

    size_t dashPos = constraints.find('-');
    if (dashPos == std::string::npos) return true;

    try {
        int min = std::stoi(constraints.substr(0, dashPos));
        int max = std::stoi(constraints.substr(dashPos + 1));
        return value >= min && value <= max;
    } catch (...) {
        return true; // If parsing fails, allow the value
    }
}

bool PreferenceManager::ValidateFloatRange(float value, const std::string& constraints) const {
    if (constraints.empty()) return true;

    size_t dashPos = constraints.find('-');
    if (dashPos == std::string::npos) return true;

    try {
        float min = std::stof(constraints.substr(0, dashPos));
        float max = std::stof(constraints.substr(dashPos + 1));
        return value >= min && value <= max;
    } catch (...) {
        return true;
    }
}

bool PreferenceManager::ValidateEnumValue(const std::string& value, const std::string& constraints) const {
    if (constraints.empty()) return true;

    auto options = ParseOptions(constraints);
    return std::find(options.begin(), options.end(), value) != options.end();
}

std::vector<std::string> PreferenceManager::ParseOptions(const std::string& str) const {
    std::vector<std::string> options;
    std::stringstream ss(str);
    std::string option;

    while (std::getline(ss, option, ',')) {
        // Trim whitespace
        option.erase(0, option.find_first_not_of(" \t"));
        option.erase(option.find_last_not_of(" \t") + 1);
        if (!option.empty()) {
            options.push_back(option);
        }
    }

    return options;
}

bool PreferenceManager::StoreValueToNVS(Preferences& prefs, const std::string& key,
                                               const Config::ConfigValue& value, Config::ConfigValueType type) {
    switch (type) {
        case Config::ConfigValueType::Integer:
            if (auto val = Config::ConfigValueHelper::GetValue<int>(value)) {
                return prefs.putInt(key.c_str(), *val);
            }
            break;

        case Config::ConfigValueType::Float:
            if (auto val = Config::ConfigValueHelper::GetValue<float>(value)) {
                return prefs.putFloat(key.c_str(), *val);
            }
            break;

        case Config::ConfigValueType::Boolean:
            if (auto val = Config::ConfigValueHelper::GetValue<bool>(value)) {
                return prefs.putBool(key.c_str(), *val);
            }
            break;

        case Config::ConfigValueType::String:
        case Config::ConfigValueType::Enum:
            if (auto val = Config::ConfigValueHelper::GetValue<std::string>(value)) {
                return prefs.putString(key.c_str(), val->c_str());
            }
            break;
    }

    return false;
}

Config::ConfigValue PreferenceManager::LoadValueFromNVS(Preferences& prefs, const std::string& key,
                                                               Config::ConfigValueType type) {
    switch (type) {
        case Config::ConfigValueType::Integer:
            return prefs.getInt(key.c_str(), 0);

        case Config::ConfigValueType::Float:
            return prefs.getFloat(key.c_str(), 0.0f);

        case Config::ConfigValueType::Boolean:
            return prefs.getBool(key.c_str(), false);

        case Config::ConfigValueType::String:
        case Config::ConfigValueType::Enum:
            return std::string(prefs.getString(key.c_str(), "").c_str());
    }

    return std::monostate{};
}