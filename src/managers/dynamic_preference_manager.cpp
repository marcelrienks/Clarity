#include "managers/dynamic_preference_manager.h"
#include "utilities/logging.h"
#include <algorithm>
#include <sstream>

// ========== PreferenceManager Override ==========

void DynamicPreferenceManager::Init() {
    // Call parent initialization first to set up legacy system
    PreferenceManager::Init();

    std::lock_guard<std::mutex> lock(configMutex_);

    // Check if migration is needed
    preferences_.begin(META_NAMESPACE_, false);
    migrationCompleted_ = preferences_.getBool(MIGRATION_FLAG_, false);
    preferences_.end();

    // Load or migrate configuration
    if (!migrationCompleted_) {
        log_i("Migrating legacy configuration to dynamic system");
        if (!MigrateLegacyConfig()) {
            log_e("Migration failed, using legacy system");
        }
    } else {
        // Load existing dynamic configuration
        LoadAllConfigSections();
        SyncLegacyConfig();
    }

    log_i("DynamicPreferenceManager initialized with live updates enabled");
}

void DynamicPreferenceManager::SaveConfig() {
    std::lock_guard<std::mutex> lock(configMutex_);

    // Sync from legacy config to dynamic sections
    SyncFromLegacyConfig();

    // Save all sections
    SaveAllConfigSections();

    log_d("Configuration saved");
}

void DynamicPreferenceManager::LoadConfig() {
    std::lock_guard<std::mutex> lock(configMutex_);

    // Load all sections
    LoadAllConfigSections();

    // Sync to legacy config
    SyncLegacyConfig();

    log_d("Configuration loaded");
}

void DynamicPreferenceManager::CreateDefaultConfig() {
    std::lock_guard<std::mutex> lock(configMutex_);

    // Create default sections
    CreateDefaultSections();

    // Set legacy config defaults
    legacyConfig_ = Configs();

    // Save everything
    SaveAllConfigSections();

    // Mark migration as complete
    preferences_.begin(META_NAMESPACE_, false);
    preferences_.putBool(MIGRATION_FLAG_, true);
    preferences_.end();

    migrationCompleted_ = true;

    log_i("Default configuration created");
}

Configs& DynamicPreferenceManager::GetConfig() {
    return legacyConfig_;
}

const Configs& DynamicPreferenceManager::GetConfig() const {
    return legacyConfig_;
}

void DynamicPreferenceManager::SetConfig(const Configs& config) {
    std::lock_guard<std::mutex> lock(configMutex_);
    legacyConfig_ = config;
    SyncFromLegacyConfig();
    SaveAllConfigSections();
}

std::string DynamicPreferenceManager::GetPreference(const std::string& key) const {
    // Map legacy keys to new system
    if (key == "panelName") return legacyConfig_.panelName;
    if (key == "showSplash") return legacyConfig_.showSplash ? "true" : "false";
    if (key == "splashDuration") return std::to_string(legacyConfig_.splashDuration);
    if (key == "theme") return legacyConfig_.theme;
    if (key == "updateRate") return std::to_string(legacyConfig_.updateRate);
    if (key == "pressureUnit") return legacyConfig_.pressureUnit;
    if (key == "tempUnit") return legacyConfig_.tempUnit;
    if (key == "pressureOffset") return std::to_string(legacyConfig_.pressureOffset);
    if (key == "pressureScale") return std::to_string(legacyConfig_.pressureScale);
    if (key == "tempOffset") return std::to_string(legacyConfig_.tempOffset);
    if (key == "tempScale") return std::to_string(legacyConfig_.tempScale);

    return "";
}

void DynamicPreferenceManager::SetPreference(const std::string& key, const std::string& value) {
    std::lock_guard<std::mutex> lock(configMutex_);

    // Map legacy keys to new system
    if (key == "panelName") legacyConfig_.panelName = value;
    else if (key == "showSplash") legacyConfig_.showSplash = (value == "true");
    else if (key == "splashDuration") legacyConfig_.splashDuration = std::stoi(value);
    else if (key == "theme") legacyConfig_.theme = value;
    else if (key == "updateRate") legacyConfig_.updateRate = std::stoi(value);
    else if (key == "pressureUnit") legacyConfig_.pressureUnit = value;
    else if (key == "tempUnit") legacyConfig_.tempUnit = value;
    else if (key == "pressureOffset") legacyConfig_.pressureOffset = std::stof(value);
    else if (key == "pressureScale") legacyConfig_.pressureScale = std::stof(value);
    else if (key == "tempOffset") legacyConfig_.tempOffset = std::stof(value);
    else if (key == "tempScale") legacyConfig_.tempScale = std::stof(value);

    SyncFromLegacyConfig();
}

bool DynamicPreferenceManager::HasPreference(const std::string& key) const {
    static const std::vector<std::string> knownKeys = {
        "panelName", "showSplash", "splashDuration", "theme", "updateRate",
        "pressureUnit", "tempUnit", "pressureOffset", "pressureScale",
        "tempOffset", "tempScale"
    };

    return std::find(knownKeys.begin(), knownKeys.end(), key) != knownKeys.end();
}

// ========== IDynamicConfigService Implementation ==========

bool DynamicPreferenceManager::RegisterConfigSection(const Config::ConfigSection& section) {
    std::lock_guard<std::mutex> lock(configMutex_);

    if (registeredSections_.find(section.sectionName) != registeredSections_.end()) {
        log_w("Section %s already registered", section.sectionName.c_str());
        return false;
    }

    registeredSections_[section.sectionName] = section;

    // Load existing values from NVS if available
    LoadConfigSection(section.sectionName);

    log_d("Registered configuration section: %s", section.sectionName.c_str());
    return true;
}

bool DynamicPreferenceManager::UnregisterConfigSection(const std::string& sectionName) {
    std::lock_guard<std::mutex> lock(configMutex_);

    auto it = registeredSections_.find(sectionName);
    if (it == registeredSections_.end()) {
        return false;
    }

    registeredSections_.erase(it);
    log_d("Unregistered configuration section: %s", sectionName.c_str());
    return true;
}

std::vector<std::string> DynamicPreferenceManager::GetRegisteredSectionNames() const {
    std::lock_guard<std::mutex> lock(configMutex_);

    std::vector<std::string> names;
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
    for (const auto& [name, section] : registeredSections_) {
        sections.push_back(section);
    }
    return sections;
}

bool DynamicPreferenceManager::SaveConfigSection(const std::string& sectionName) {
    auto it = registeredSections_.find(sectionName);
    if (it == registeredSections_.end()) {
        return false;
    }

    std::string ns = GetSectionNamespace(sectionName);
    preferences_.begin(ns.c_str(), false);

    for (const auto& item : it->second.items) {
        StoreValueToNVS(preferences_, item.key, item.value, item.type);
    }

    preferences_.end();
    return true;
}

bool DynamicPreferenceManager::LoadConfigSection(const std::string& sectionName) {
    auto it = registeredSections_.find(sectionName);
    if (it == registeredSections_.end()) {
        return false;
    }

    std::string ns = GetSectionNamespace(sectionName);
    preferences_.begin(ns.c_str(), true);

    for (auto& item : it->second.items) {
        Config::ConfigValue loadedValue = LoadValueFromNVS(preferences_, item.key, item.type);
        if (!std::holds_alternative<std::monostate>(loadedValue)) {
            item.value = loadedValue;
        }
    }

    preferences_.end();
    return true;
}

bool DynamicPreferenceManager::SaveAllConfigSections() {
    bool success = true;

    // Save section list
    SaveSectionList();

    // Save each section
    for (const auto& [name, section] : registeredSections_) {
        if (!SaveConfigSection(name)) {
            success = false;
        }
    }

    return success;
}

bool DynamicPreferenceManager::LoadAllConfigSections() {
    bool success = true;

    // Load section list
    LoadSectionList();

    // Load each section
    for (const auto& [name, section] : registeredSections_) {
        if (!LoadConfigSection(name)) {
            success = false;
        }
    }

    return success;
}

bool DynamicPreferenceManager::ValidateConfigValue(const std::string& fullKey, const Config::ConfigValue& value) const {
    auto [sectionName, itemKey] = ParseConfigKey(fullKey);

    auto sectionOpt = GetConfigSection(sectionName);
    if (!sectionOpt) return false;

    auto item = sectionOpt->FindItem(itemKey);
    if (!item) return false;
    const auto& constraints = item->metadata.constraints;

    switch (item->type) {
        case Config::ConfigValueType::Integer:
            if (auto intVal = Config::ConfigValueHelper::GetValue<int>(value)) {
                return ValidateIntRange(*intVal, constraints);
            }
            break;

        case Config::ConfigValueType::Float:
            if (auto floatVal = Config::ConfigValueHelper::GetValue<float>(value)) {
                return ValidateFloatRange(*floatVal, constraints);
            }
            break;

        case Config::ConfigValueType::Enum:
            if (auto strVal = Config::ConfigValueHelper::GetValue<std::string>(value)) {
                return ValidateEnumValue(*strVal, constraints);
            }
            break;

        case Config::ConfigValueType::Boolean:
        case Config::ConfigValueType::String:
            return true;
    }

    return false;
}

bool DynamicPreferenceManager::ResetToDefault(const std::string& fullKey) {
    std::lock_guard<std::mutex> lock(configMutex_);

    auto [sectionName, itemKey] = ParseConfigKey(fullKey);

    auto it = registeredSections_.find(sectionName);
    if (it == registeredSections_.end()) return false;

    auto item = it->second.FindItem(itemKey);
    if (!item) return false;

    item->value = item->defaultValue;
    return true;
}

bool DynamicPreferenceManager::ResetSectionToDefaults(const std::string& sectionName) {
    std::lock_guard<std::mutex> lock(configMutex_);

    auto it = registeredSections_.find(sectionName);
    if (it == registeredSections_.end()) return false;

    for (auto& item : it->second.items) {
        item.value = item.defaultValue;
    }

    return true;
}

std::optional<Config::ConfigValue> DynamicPreferenceManager::QueryConfigImpl(const std::string& fullKey) const {
    std::lock_guard<std::mutex> lock(configMutex_);

    auto [sectionName, itemKey] = ParseConfigKey(fullKey);

    auto sectionOpt = GetConfigSection(sectionName);
    if (!sectionOpt) return std::nullopt;

    auto item = sectionOpt->FindItem(itemKey);
    if (!item) return std::nullopt;

    return item->value;
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

bool DynamicPreferenceManager::MigrateLegacyConfig() {
    // Check if legacy config exists
    preferences_.begin("clarity", true);
    String jsonStr = preferences_.getString(CONFIG_KEY_, "");
    preferences_.end();

    if (jsonStr.isEmpty()) {
        // No legacy config, create defaults
        CreateDefaultConfig();
        return true;
    }

    // Parse legacy JSON
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, jsonStr);

    if (error) {
        log_e("Failed to parse legacy config: %s", error.c_str());
        return false;
    }

    // Load into legacy struct
    legacyConfig_.panelName = doc["panelName"] | "OilPanel";
    legacyConfig_.showSplash = doc["showSplash"] | true;
    legacyConfig_.splashDuration = doc["splashDuration"] | 1500;
    legacyConfig_.theme = doc["theme"] | "Day";
    legacyConfig_.updateRate = doc["updateRate"] | 500;
    legacyConfig_.pressureUnit = doc["pressureUnit"] | "Bar";
    legacyConfig_.tempUnit = doc["tempUnit"] | "C";
    legacyConfig_.pressureOffset = doc["pressureOffset"] | 0.0f;
    legacyConfig_.pressureScale = doc["pressureScale"] | 1.0f;
    legacyConfig_.tempOffset = doc["tempOffset"] | 0.0f;
    legacyConfig_.tempScale = doc["tempScale"] | 1.0f;

    // Create default sections
    CreateDefaultSections();

    // Sync from legacy to dynamic
    SyncFromLegacyConfig();

    // Save to new format
    SaveAllConfigSections();

    // Mark migration complete
    preferences_.begin(META_NAMESPACE_, false);
    preferences_.putBool(MIGRATION_FLAG_, true);
    preferences_.end();

    // Remove legacy config
    preferences_.begin("clarity", false);
    preferences_.remove(CONFIG_KEY_);
    preferences_.end();

    migrationCompleted_ = true;
    log_i("Legacy configuration migrated successfully");

    return true;
}

void DynamicPreferenceManager::SyncLegacyConfig() {
    // Update legacy config struct from dynamic sections

    // System settings
    if (auto val = QueryConfig<std::string>("system.panel_name")) {
        legacyConfig_.panelName = *val;
    }
    if (auto val = QueryConfig<bool>("system.show_splash")) {
        legacyConfig_.showSplash = *val;
    }
    if (auto val = QueryConfig<int>("system.splash_duration")) {
        legacyConfig_.splashDuration = *val;
    }
    if (auto val = QueryConfig<int>("system.update_rate")) {
        legacyConfig_.updateRate = *val;
    }

    // Style settings
    if (auto val = QueryConfig<std::string>("style.theme")) {
        legacyConfig_.theme = *val;
    }

    // Oil pressure sensor
    if (auto val = QueryConfig<std::string>("oil_pressure.unit")) {
        legacyConfig_.pressureUnit = *val;
    }
    if (auto val = QueryConfig<float>("oil_pressure.offset")) {
        legacyConfig_.pressureOffset = *val;
    }
    if (auto val = QueryConfig<float>("oil_pressure.scale")) {
        legacyConfig_.pressureScale = *val;
    }

    // Oil temperature sensor
    if (auto val = QueryConfig<std::string>("oil_temperature.unit")) {
        legacyConfig_.tempUnit = *val;
    }
    if (auto val = QueryConfig<float>("oil_temperature.offset")) {
        legacyConfig_.tempOffset = *val;
    }
    if (auto val = QueryConfig<float>("oil_temperature.scale")) {
        legacyConfig_.tempScale = *val;
    }
}

void DynamicPreferenceManager::SyncFromLegacyConfig() {
    // Update dynamic sections from legacy config struct

    UpdateConfig("system.panel_name", legacyConfig_.panelName);
    UpdateConfig("system.show_splash", legacyConfig_.showSplash);
    UpdateConfig("system.splash_duration", legacyConfig_.splashDuration);
    UpdateConfig("system.update_rate", legacyConfig_.updateRate);
    UpdateConfig("style.theme", legacyConfig_.theme);
    UpdateConfig("oil_pressure.unit", legacyConfig_.pressureUnit);
    UpdateConfig("oil_pressure.offset", legacyConfig_.pressureOffset);
    UpdateConfig("oil_pressure.scale", legacyConfig_.pressureScale);
    UpdateConfig("oil_temperature.unit", legacyConfig_.tempUnit);
    UpdateConfig("oil_temperature.offset", legacyConfig_.tempOffset);
    UpdateConfig("oil_temperature.scale", legacyConfig_.tempScale);
}

void DynamicPreferenceManager::CreateDefaultSections() {
    using namespace Config;

    // System settings section
    ConfigSection systemSection("System", "system", "System Settings");
    systemSection.displayOrder = 0;
    systemSection.AddItem(ConfigItem("panel_name", "Default Panel", ConfigValueType::Enum,
        std::string("OilPanel"), ConfigMetadata("OilPanel")));
    systemSection.AddItem(ConfigItem("show_splash", "Show Splash Screen", ConfigValueType::Boolean,
        true));
    systemSection.AddItem(ConfigItem("splash_duration", "Splash Duration (ms)", ConfigValueType::Enum,
        1500, ConfigMetadata("1500,1750,2000,2500")));
    systemSection.AddItem(ConfigItem("update_rate", "Update Rate (ms)", ConfigValueType::Enum,
        500, ConfigMetadata("250,500,1000,2000")));
    RegisterConfigSection(systemSection);

    // Style settings section
    ConfigSection styleSection("StyleManager", "style", "Display Settings");
    styleSection.displayOrder = 1;
    styleSection.AddItem(ConfigItem("theme", "Theme", ConfigValueType::Enum,
        std::string("Day"), ConfigMetadata("Day,Night")));
    RegisterConfigSection(styleSection);

    // Oil pressure sensor section
    ConfigSection pressureSection("OilPressureSensor", "oil_pressure", "Oil Pressure Sensor");
    pressureSection.displayOrder = 2;
    pressureSection.AddItem(ConfigItem("unit", "Pressure Unit", ConfigValueType::Enum,
        std::string("Bar"), ConfigMetadata("PSI,Bar,kPa")));
    pressureSection.AddItem(ConfigItem("offset", "Calibration Offset", ConfigValueType::Float,
        0.0f, ConfigMetadata("-1.0,1.0")));
    pressureSection.AddItem(ConfigItem("scale", "Calibration Scale", ConfigValueType::Float,
        1.0f, ConfigMetadata("0.9,1.1")));
    RegisterConfigSection(pressureSection);

    // Oil temperature sensor section
    ConfigSection tempSection("OilTemperatureSensor", "oil_temperature", "Oil Temperature Sensor");
    tempSection.displayOrder = 3;
    tempSection.AddItem(ConfigItem("unit", "Temperature Unit", ConfigValueType::Enum,
        std::string("C"), ConfigMetadata("C,F")));
    tempSection.AddItem(ConfigItem("offset", "Calibration Offset", ConfigValueType::Float,
        0.0f, ConfigMetadata("-5.0,5.0")));
    tempSection.AddItem(ConfigItem("scale", "Calibration Scale", ConfigValueType::Float,
        1.0f, ConfigMetadata("0.9,1.1")));
    RegisterConfigSection(tempSection);
}

bool DynamicPreferenceManager::ValidateIntRange(int value, const std::string& constraints) const {
    if (constraints.empty()) return true;

    // Check if it's a list of options
    if (constraints.find(',') != std::string::npos) {
        auto options = ParseOptions(constraints);
        for (const auto& opt : options) {
            try {
                if (std::stoi(opt) == value) return true;
            } catch (...) {}
        }
        return false;
    }

    // Check if it's a range
    size_t dashPos = constraints.find('-', 1); // Start from 1 to handle negative numbers
    if (dashPos != std::string::npos) {
        try {
            int min = std::stoi(constraints.substr(0, dashPos));
            int max = std::stoi(constraints.substr(dashPos + 1));
            return value >= min && value <= max;
        } catch (...) {}
    }

    return true;
}

bool DynamicPreferenceManager::ValidateFloatRange(float value, const std::string& constraints) const {
    if (constraints.empty()) return true;

    // Check if it's a list of options
    if (constraints.find(',') != std::string::npos) {
        auto options = ParseOptions(constraints);
        for (const auto& opt : options) {
            try {
                if (std::stof(opt) == value) return true;
            } catch (...) {}
        }
        return false;
    }

    // Check if it's a range
    size_t dashPos = constraints.find('-', 1); // Start from 1 to handle negative numbers
    if (dashPos != std::string::npos) {
        try {
            float min = std::stof(constraints.substr(0, dashPos));
            float max = std::stof(constraints.substr(dashPos + 1));
            return value >= min && value <= max;
        } catch (...) {}
    }

    return true;
}

bool DynamicPreferenceManager::ValidateEnumValue(const std::string& value, const std::string& constraints) const {
    if (constraints.empty()) return true;

    auto options = ParseOptions(constraints);
    return std::find(options.begin(), options.end(), value) != options.end();
}

std::vector<std::string> DynamicPreferenceManager::ParseOptions(const std::string& str) const {
    std::vector<std::string> options;
    std::stringstream ss(str);
    std::string item;

    while (std::getline(ss, item, ',')) {
        // Trim whitespace
        item.erase(0, item.find_first_not_of(" \t"));
        item.erase(item.find_last_not_of(" \t") + 1);
        if (!item.empty()) {
            options.push_back(item);
        }
    }

    return options;
}

bool DynamicPreferenceManager::SaveSectionList() {
    preferences_.begin(META_NAMESPACE_, false);

    // Create comma-separated list of section names
    std::string sectionList;
    for (const auto& [name, section] : registeredSections_) {
        if (!sectionList.empty()) sectionList += ",";
        sectionList += name;
    }

    preferences_.putString("sections", sectionList.c_str());
    preferences_.end();

    return true;
}

bool DynamicPreferenceManager::LoadSectionList() {
    preferences_.begin(META_NAMESPACE_, true);
    String sectionListStr = preferences_.getString("sections", "");
    std::string sectionList(sectionListStr.c_str());
    preferences_.end();

    if (sectionList.empty()) {
        return false;
    }

    // Parse section list and ensure they're registered
    auto sections = ParseOptions(sectionList);
    for (const auto& sectionName : sections) {
        // Sections should already be registered by components
        // This is mainly for validation
        if (registeredSections_.find(sectionName) == registeredSections_.end()) {
            log_w("Section %s in storage but not registered", sectionName.c_str());
        }
    }

    return true;
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

    // Get current value
    auto currentValue = QueryConfigImpl(fullKey);
    if (!currentValue) {
        log_w("Cannot notify change for non-existent key: %s", fullKey.c_str());
        return false;
    }

    // Notify relevant callbacks
    bool notificationSent = false;
    for (const auto& [callbackId, keyCallbackPair] : changeCallbacks_) {
        const std::string& watchedKey = keyCallbackPair.first;
        const ConfigChangeCallback& callback = keyCallbackPair.second;

        // Empty string means watch all keys
        if (watchedKey.empty() || watchedKey == fullKey) {
            try {
                // We don't have old value in this context, so pass nullopt
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

bool DynamicPreferenceManager::UpdateConfigImpl(const std::string& fullKey, const Config::ConfigValue& value) {
    auto [sectionName, itemKey] = ParseConfigKey(fullKey);

    // Find the section
    auto sectionIt = registeredSections_.find(sectionName);
    if (sectionIt == registeredSections_.end()) {
        log_w("Section not found for key: %s", fullKey.c_str());
        return false;
    }

    Config::ConfigSection& section = sectionIt->second;

    // Find the item
    auto itemIt = std::find_if(section.items.begin(), section.items.end(),
        [&itemKey](const Config::ConfigItem& item) { return item.key == itemKey; });

    if (itemIt == section.items.end()) {
        log_w("Item not found for key: %s", fullKey.c_str());
        return false;
    }

    // Store old value for callback notification
    std::optional<Config::ConfigValue> oldValue = itemIt->value;

    // Validate the new value
    if (!ValidateConfigValue(fullKey, value)) {
        log_w("Validation failed for key: %s", fullKey.c_str());
        return false;
    }

    // Update the value
    itemIt->value = value;

    // Save to NVS
    std::string nsName = GetSectionNamespace(sectionName);
    preferences_.begin(nsName.c_str(), false);
    bool success = StoreValueToNVS(preferences_, itemKey, value, itemIt->type);
    preferences_.end();

    if (success) {
        log_d("Updated config %s = %s", fullKey.c_str(),
              Config::ConfigValueHelper::ToString(value).c_str());

        // Notify callbacks if live updates are enabled
        if (liveUpdatesEnabled_) {
            for (const auto& [callbackId, keyCallbackPair] : changeCallbacks_) {
                const std::string& watchedKey = keyCallbackPair.first;
                const ConfigChangeCallback& callback = keyCallbackPair.second;

                // Empty string means watch all keys
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

        // Sync legacy config if needed
        SyncLegacyConfig();
    } else {
        // Revert the in-memory change if NVS save failed
        itemIt->value = *oldValue;
        log_e("Failed to save config to NVS for key: %s", fullKey.c_str());
    }

    return success;
}