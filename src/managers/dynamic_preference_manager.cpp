#include "managers/dynamic_preference_manager.h"
#include "utilities/logging.h"
#include <algorithm>
#include <sstream>

// ========== IPreferenceService Implementation ==========

void DynamicPreferenceManager::Init() {
    std::lock_guard<std::mutex> lock(configMutex_);

    // Initialize NVS
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        nvs_flash_erase();
        err = nvs_flash_init();
    }

    if (err != ESP_OK) {
        log_e("Failed to initialize NVS: %s", esp_err_to_name(err));
        return;
    }

    // Create default sections
    CreateDefaultSections();

    // Load all registered sections
    LoadAllConfigSections();

    // Initialize Configs struct from dynamic sections
    SyncToLegacyConfig();

    log_i("DynamicPreferenceManager initialized with live updates enabled");
}

void DynamicPreferenceManager::SaveConfig() {
    std::lock_guard<std::mutex> lock(configMutex_);
    SaveAllConfigSections();
    log_d("Configuration saved");
}

void DynamicPreferenceManager::LoadConfig() {
    std::lock_guard<std::mutex> lock(configMutex_);
    LoadAllConfigSections();
    SyncToLegacyConfig();
    log_d("Configuration loaded");
}

void DynamicPreferenceManager::CreateDefaultConfig() {
    std::lock_guard<std::mutex> lock(configMutex_);
    CreateDefaultSections();
    config_ = Configs(); // Set defaults
    SaveAllConfigSections();
    log_i("Default configuration created");
}

Configs& DynamicPreferenceManager::GetConfig() {
    std::lock_guard<std::mutex> lock(configMutex_);
    SyncToLegacyConfig(); // Ensure it's up to date
    return config_;
}

const Configs& DynamicPreferenceManager::GetConfig() const {
    std::lock_guard<std::mutex> lock(configMutex_);
    return config_;
}

void DynamicPreferenceManager::SetConfig(const Configs& config) {
    std::lock_guard<std::mutex> lock(configMutex_);
    config_ = config;
    // Note: In pure dynamic system, this would sync to sections,
    // but for simplicity we'll just store it
}

std::string DynamicPreferenceManager::GetPreference(const std::string& key) const {
    std::lock_guard<std::mutex> lock(configMutex_);
    if (auto value = QueryConfigImpl(key)) {
        return Config::ConfigValueHelper::ToString(*value);
    }
    return "";
}

void DynamicPreferenceManager::SetPreference(const std::string& key, const std::string& value) {
    std::lock_guard<std::mutex> lock(configMutex_);
    UpdateConfigImpl(key, Config::ConfigValue(value));
}

bool DynamicPreferenceManager::HasPreference(const std::string& key) const {
    std::lock_guard<std::mutex> lock(configMutex_);
    return QueryConfigImpl(key).has_value();
}

// ========== IDynamicConfigService Implementation ==========

bool DynamicPreferenceManager::RegisterConfigSection(const Config::ConfigSection& section) {
    std::lock_guard<std::mutex> lock(configMutex_);

    if (registeredSections_.find(section.name) != registeredSections_.end()) {
        log_w("Section '%s' already registered", section.name.c_str());
        return false;
    }

    registeredSections_[section.name] = section;
    log_d("Registered config section: %s", section.name.c_str());

    // Load existing values for this section
    LoadConfigSection(section.name);

    // Notify section callbacks
    if (liveUpdatesEnabled_) {
        for (const auto& [callbackId, sectionCallbackPair] : sectionCallbacks_) {
            const std::string& watchedSection = sectionCallbackPair.first;
            const SectionChangeCallback& callback = sectionCallbackPair.second;

            if (watchedSection.empty() || watchedSection == section.name) {
                try {
                    callback(section.name, "added");
                    log_t("Notified section callback %u for registration: %s", callbackId, section.name.c_str());
                } catch (const std::exception& e) {
                    log_e("Exception in section callback %u: %s", callbackId, e.what());
                }
            }
        }
    }

    return true;
}

bool DynamicPreferenceManager::UnregisterConfigSection(const std::string& sectionName) {
    std::lock_guard<std::mutex> lock(configMutex_);

    auto it = registeredSections_.find(sectionName);
    if (it == registeredSections_.end()) {
        return false;
    }

    registeredSections_.erase(it);
    log_d("Unregistered config section: %s", sectionName.c_str());

    // Notify section callbacks
    if (liveUpdatesEnabled_) {
        for (const auto& [callbackId, sectionCallbackPair] : sectionCallbacks_) {
            const std::string& watchedSection = sectionCallbackPair.first;
            const SectionChangeCallback& callback = sectionCallbackPair.second;

            if (watchedSection.empty() || watchedSection == sectionName) {
                try {
                    callback(sectionName, "removed");
                    log_t("Notified section callback %u for unregistration: %s", callbackId, sectionName.c_str());
                } catch (const std::exception& e) {
                    log_e("Exception in section callback %u: %s", callbackId, e.what());
                }
            }
        }
    }

    return true;
}

std::vector<std::string> DynamicPreferenceManager::GetRegisteredSectionNames() const {
    std::lock_guard<std::mutex> lock(configMutex_);
    std::vector<std::string> names;
    names.reserve(registeredSections_.size());

    for (const auto& [name, section] : registeredSections_) {
        names.push_back(name);
    }

    return names;
}

std::optional<Config::ConfigSection> DynamicPreferenceManager::GetConfigSection(const std::string& sectionName) const {
    std::lock_guard<std::mutex> lock(configMutex_);

    auto it = registeredSections_.find(sectionName);
    if (it != registeredSections_.end()) {
        return it->second;
    }

    return std::nullopt;
}

std::vector<Config::ConfigSection> DynamicPreferenceManager::GetAllConfigSections() const {
    std::lock_guard<std::mutex> lock(configMutex_);
    std::vector<Config::ConfigSection> sections;
    sections.reserve(registeredSections_.size());

    for (const auto& [name, section] : registeredSections_) {
        sections.push_back(section);
    }

    return sections;
}

bool DynamicPreferenceManager::SaveConfigSection(const std::string& sectionName) {
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
            success = false;
        }
    }

    preferences_.end();

    if (success) {
        log_d("Saved config section: %s", sectionName.c_str());
    }

    return success;
}

bool DynamicPreferenceManager::LoadConfigSection(const std::string& sectionName) {
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
    log_d("Loaded config section: %s", sectionName.c_str());
    return true;
}

bool DynamicPreferenceManager::SaveAllConfigSections() {
    bool allSuccess = true;
    for (const auto& [name, section] : registeredSections_) {
        if (!SaveConfigSection(name)) {
            allSuccess = false;
        }
    }
    return allSuccess;
}

bool DynamicPreferenceManager::LoadAllConfigSections() {
    bool allSuccess = true;
    for (const auto& [name, section] : registeredSections_) {
        if (!LoadConfigSection(name)) {
            allSuccess = false;
        }
    }
    return allSuccess;
}

bool DynamicPreferenceManager::ValidateConfigValue(const std::string& fullKey, const Config::ConfigValue& value) const {
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

bool DynamicPreferenceManager::ResetToDefault(const std::string& fullKey) {
    auto [sectionName, itemKey] = ParseConfigKey(fullKey);

    auto sectionIt = registeredSections_.find(sectionName);
    if (sectionIt == registeredSections_.end()) {
        return false;
    }

    Config::ConfigSection& section = sectionIt->second;
    auto itemIt = std::find_if(section.items.begin(), section.items.end(),
        [&itemKey](Config::ConfigItem& item) { return item.key == itemKey; });

    if (itemIt == section.items.end()) {
        return false;
    }

    Config::ConfigValue oldValue = itemIt->value;
    itemIt->value = itemIt->defaultValue;

    // Save to NVS
    SaveConfigSection(sectionName);

    // Notify callbacks
    if (liveUpdatesEnabled_) {
        for (const auto& [callbackId, keyCallbackPair] : changeCallbacks_) {
            const std::string& watchedKey = keyCallbackPair.first;
            const ConfigChangeCallback& callback = keyCallbackPair.second;

            if (watchedKey.empty() || watchedKey == fullKey) {
                try {
                    callback(fullKey, oldValue, itemIt->defaultValue);
                    log_t("Notified callback %u for reset: %s", callbackId, fullKey.c_str());
                } catch (const std::exception& e) {
                    log_e("Exception in change callback %u: %s", callbackId, e.what());
                }
            }
        }
    }

    log_d("Reset to default: %s", fullKey.c_str());
    return true;
}

bool DynamicPreferenceManager::ResetSectionToDefaults(const std::string& sectionName) {
    auto sectionIt = registeredSections_.find(sectionName);
    if (sectionIt == registeredSections_.end()) {
        return false;
    }

    Config::ConfigSection& section = sectionIt->second;

    for (auto& item : section.items) {
        Config::ConfigValue oldValue = item.value;
        item.value = item.defaultValue;

        // Notify individual item callbacks
        std::string fullKey = sectionName + "." + item.key;
        if (liveUpdatesEnabled_) {
            for (const auto& [callbackId, keyCallbackPair] : changeCallbacks_) {
                const std::string& watchedKey = keyCallbackPair.first;
                const ConfigChangeCallback& callback = keyCallbackPair.second;

                if (watchedKey.empty() || watchedKey == fullKey) {
                    try {
                        callback(fullKey, oldValue, item.defaultValue);
                        log_t("Notified callback %u for section reset: %s", callbackId, fullKey.c_str());
                    } catch (const std::exception& e) {
                        log_e("Exception in change callback %u: %s", callbackId, e.what());
                    }
                }
            }
        }
    }

    // Save entire section
    SaveConfigSection(sectionName);

    log_d("Reset section to defaults: %s", sectionName.c_str());
    return true;
}

// ========== Live Update Implementation ==========

uint32_t DynamicPreferenceManager::RegisterChangeCallback(const std::string& fullKey, ConfigChangeCallback callback) {
    std::lock_guard<std::mutex> lock(configMutex_);
    uint32_t callbackId = nextCallbackId_++;
    changeCallbacks_[callbackId] = std::make_pair(fullKey, callback);
    log_d("Registered change callback %u for key: %s", callbackId, fullKey.c_str());
    return callbackId;
}

uint32_t DynamicPreferenceManager::RegisterSectionCallback(const std::string& sectionName, SectionChangeCallback callback) {
    std::lock_guard<std::mutex> lock(configMutex_);
    uint32_t callbackId = nextCallbackId_++;
    sectionCallbacks_[callbackId] = std::make_pair(sectionName, callback);
    log_d("Registered section callback %u for section: %s", callbackId, sectionName.c_str());
    return callbackId;
}

bool DynamicPreferenceManager::UnregisterChangeCallback(uint32_t callbackId) {
    std::lock_guard<std::mutex> lock(configMutex_);
    auto it = changeCallbacks_.find(callbackId);
    if (it != changeCallbacks_.end()) {
        log_d("Unregistered change callback %u", callbackId);
        changeCallbacks_.erase(it);
        return true;
    }
    return false;
}

bool DynamicPreferenceManager::UnregisterSectionCallback(uint32_t callbackId) {
    std::lock_guard<std::mutex> lock(configMutex_);
    auto it = sectionCallbacks_.find(callbackId);
    if (it != sectionCallbacks_.end()) {
        log_d("Unregistered section callback %u", callbackId);
        sectionCallbacks_.erase(it);
        return true;
    }
    return false;
}

bool DynamicPreferenceManager::NotifyConfigChange(const std::string& fullKey) {
    if (!liveUpdatesEnabled_) {
        return false;
    }

    std::lock_guard<std::mutex> lock(configMutex_);

    auto currentValue = QueryConfigImpl(fullKey);
    if (!currentValue) {
        log_w("Cannot notify change for non-existent key: %s", fullKey.c_str());
        return false;
    }

    bool notificationSent = false;
    for (const auto& [callbackId, keyCallbackPair] : changeCallbacks_) {
        const std::string& watchedKey = keyCallbackPair.first;
        const ConfigChangeCallback& callback = keyCallbackPair.second;

        if (watchedKey.empty() || watchedKey == fullKey) {
            try {
                callback(fullKey, std::nullopt, *currentValue);
                notificationSent = true;
                log_t("Notified callback %u for key change: %s", callbackId, fullKey.c_str());
            } catch (const std::exception& e) {
                log_e("Exception in change callback %u: %s", callbackId, e.what());
            }
        }
    }

    return notificationSent;
}

void DynamicPreferenceManager::SetLiveUpdatesEnabled(bool enabled) {
    std::lock_guard<std::mutex> lock(configMutex_);
    liveUpdatesEnabled_ = enabled;
    log_i("Live updates %s", enabled ? "enabled" : "disabled");
}

bool DynamicPreferenceManager::AreLiveUpdatesEnabled() const {
    std::lock_guard<std::mutex> lock(configMutex_);
    return liveUpdatesEnabled_;
}

// ========== Protected Implementation Methods ==========

std::optional<Config::ConfigValue> DynamicPreferenceManager::QueryConfigImpl(const std::string& fullKey) const {
    auto [sectionName, itemKey] = ParseConfigKey(fullKey);

    auto sectionIt = registeredSections_.find(sectionName);
    if (sectionIt == registeredSections_.end()) {
        return std::nullopt;
    }

    const Config::ConfigSection& section = sectionIt->second;
    auto itemIt = std::find_if(section.items.begin(), section.items.end(),
        [&itemKey](const Config::ConfigItem& item) { return item.key == itemKey; });

    if (itemIt == section.items.end()) {
        return std::nullopt;
    }

    return itemIt->value;
}

bool DynamicPreferenceManager::UpdateConfigImpl(const std::string& fullKey, const Config::ConfigValue& value) {
    auto [sectionName, itemKey] = ParseConfigKey(fullKey);

    auto sectionIt = registeredSections_.find(sectionName);
    if (sectionIt == registeredSections_.end()) {
        log_w("Section not found for key: %s", fullKey.c_str());
        return false;
    }

    Config::ConfigSection& section = sectionIt->second;
    auto itemIt = std::find_if(section.items.begin(), section.items.end(),
        [&itemKey](Config::ConfigItem& item) { return item.key == itemKey; });

    if (itemIt == section.items.end()) {
        log_w("Item not found for key: %s", fullKey.c_str());
        return false;
    }

    std::optional<Config::ConfigValue> oldValue = itemIt->value;

    if (!ValidateConfigValue(fullKey, value)) {
        log_w("Validation failed for key: %s", fullKey.c_str());
        return false;
    }

    itemIt->value = value;

    std::string nsName = GetSectionNamespace(sectionName);
    preferences_.begin(nsName.c_str(), false);
    bool success = StoreValueToNVS(preferences_, itemKey, value, itemIt->type);
    preferences_.end();

    if (success) {
        log_d("Updated config %s = %s", fullKey.c_str(),
              Config::ConfigValueHelper::ToString(value).c_str());

        if (liveUpdatesEnabled_) {
            for (const auto& [callbackId, keyCallbackPair] : changeCallbacks_) {
                const std::string& watchedKey = keyCallbackPair.first;
                const ConfigChangeCallback& callback = keyCallbackPair.second;

                if (watchedKey.empty() || watchedKey == fullKey) {
                    try {
                        callback(fullKey, oldValue, value);
                        log_t("Notified callback %u for update: %s", callbackId, fullKey.c_str());
                    } catch (const std::exception& e) {
                        log_e("Exception in change callback %u: %s", callbackId, e.what());
                    }
                }
            }
        }
    } else {
        itemIt->value = *oldValue;
        log_e("Failed to save config to NVS for key: %s", fullKey.c_str());
    }

    return success;
}

// ========== Private Helper Methods ==========

std::pair<std::string, std::string> DynamicPreferenceManager::ParseConfigKey(const std::string& fullKey) const {
    size_t dotPos = fullKey.find('.');
    if (dotPos == std::string::npos) {
        return {"", fullKey};
    }
    return {fullKey.substr(0, dotPos), fullKey.substr(dotPos + 1)};
}

std::string DynamicPreferenceManager::GetSectionNamespace(const std::string& sectionName) const {
    std::string ns = SECTION_PREFIX_ + sectionName;
    if (ns.length() > MAX_NAMESPACE_LEN_) {
        ns = ns.substr(0, MAX_NAMESPACE_LEN_);
    }
    return ns;
}

void DynamicPreferenceManager::CreateDefaultSections() {
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

    log_d("Created default configuration sections");
}

void DynamicPreferenceManager::SyncToLegacyConfig() {
    // Update legacy config struct from dynamic sections
    if (auto systemSection = GetConfigSection("system")) {
        for (const auto& item : systemSection->items) {
            if (item.key == "panel_name") {
                if (auto val = Config::ConfigValueHelper::GetValue<std::string>(item.value)) {
                    config_.panelName = *val;
                }
            } else if (item.key == "show_splash") {
                if (auto val = Config::ConfigValueHelper::GetValue<bool>(item.value)) {
                    config_.showSplash = *val;
                }
            }
        }
    }

    // Sync sensor configurations if available
    if (auto tempSection = GetConfigSection("oil_temperature")) {
        for (const auto& item : tempSection->items) {
            if (item.key == "unit") {
                if (auto val = Config::ConfigValueHelper::GetValue<std::string>(item.value)) {
                    config_.tempUnit = *val;
                }
            } else if (item.key == "update_rate") {
                if (auto val = Config::ConfigValueHelper::GetValue<int>(item.value)) {
                    config_.updateRate = *val;
                }
            } else if (item.key == "offset") {
                if (auto val = Config::ConfigValueHelper::GetValue<float>(item.value)) {
                    config_.tempOffset = *val;
                }
            } else if (item.key == "scale") {
                if (auto val = Config::ConfigValueHelper::GetValue<float>(item.value)) {
                    config_.tempScale = *val;
                }
            }
        }
    }

    if (auto pressureSection = GetConfigSection("oil_pressure")) {
        for (const auto& item : pressureSection->items) {
            if (item.key == "unit") {
                if (auto val = Config::ConfigValueHelper::GetValue<std::string>(item.value)) {
                    config_.pressureUnit = *val;
                }
            } else if (item.key == "offset") {
                if (auto val = Config::ConfigValueHelper::GetValue<float>(item.value)) {
                    config_.pressureOffset = *val;
                }
            } else if (item.key == "scale") {
                if (auto val = Config::ConfigValueHelper::GetValue<float>(item.value)) {
                    config_.pressureScale = *val;
                }
            }
        }
    }
}

bool DynamicPreferenceManager::ValidateIntRange(int value, const std::string& constraints) const {
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

bool DynamicPreferenceManager::ValidateFloatRange(float value, const std::string& constraints) const {
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

bool DynamicPreferenceManager::ValidateEnumValue(const std::string& value, const std::string& constraints) const {
    if (constraints.empty()) return true;

    auto options = ParseOptions(constraints);
    return std::find(options.begin(), options.end(), value) != options.end();
}

std::vector<std::string> DynamicPreferenceManager::ParseOptions(const std::string& str) const {
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

bool DynamicPreferenceManager::StoreValueToNVS(Preferences& prefs, const std::string& key,
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

Config::ConfigValue DynamicPreferenceManager::LoadValueFromNVS(Preferences& prefs, const std::string& key,
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