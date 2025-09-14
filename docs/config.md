# Dynamic Configuration Management System

## Overview

This document outlines the design and implementation of a dynamic configuration management system for Clarity. This system allows components to register their configuration requirements with the PreferenceManager, which automatically handles persistence, validation, and UI generation.

## Core Principles

1. **Universal Component Self-Registration**: Any component in the application can register its configuration needs during construction
2. **Default-First Architecture**: Default configurations are applied on first startup, persisted configurations used thereafter
3. **Dynamic UI Generation**: Config panel dynamically builds menus based on registered configurations
4. **Type Safety**: Strong typing for configuration values with validation
5. **Extensibility**: Easy addition of new configuration types and any application component

## Architecture Overview

```
┌─────────────────┐    Register    ┌──────────────────┐    Query     ┌─────────────────┐
│ Any Component   │ ───────────────► │ PreferenceManager│ ◄─────────── │   ConfigPanel   │
│                 │                 │                  │              │                 │
│ - Sensors       │                 │ - Config Cache   │              │ - Dynamic Menus │
│ - Panels        │                 │ - ESP32 NVS      │              │ - Validation    │
│ - Managers      │                 │ - Validation     │              │ - Persistence   │
│ - Components    │                 │                  │              │                 │
│ - Providers     │                 │                  │              │                 │
│ - Handlers      │                 │                  │              │                 │
│ - Utilities     │                 │                  │              │                 │
└─────────────────┘                 └──────────────────┘              └─────────────────┘
```

## Configuration Data Structures

### Unified Configuration Structure

The system uses a single ConfigItem structure that contains both runtime values and UI metadata. During NVS persistence, only the essential fields (key, displayName, type, value) are stored, while metadata is preserved in the runtime cache for UI generation.

### ConfigItem (Unified Structure)
Single structure that contains both runtime values and UI metadata. Metadata is excluded during NVS persistence.

```cpp
struct ConfigItem {
    std::string key;                    // Unique identifier within section
    std::string displayName;            // Human-readable name for UI
    ConfigValueType type;               // Data type (int, float, string, bool, enum)
    std::variant<int, float, std::string, bool> value;  // Current/default value
    std::string metadata = "";          // Comma-delimited options for ConfigPanel UI
};

// Examples of metadata usage:
// Boolean: metadata = "true,false" or "On,Off" or "Enabled,Disabled"
// Enum: metadata = "Day,Night,Auto"
// Integer range: metadata = "10,20,30,40,50" or "1-100" (parsed by UI)
// Baud rates: metadata = "9600,57600,115200"
// Empty string: metadata = "" (allows free text input)
```

enum class ConfigValueType {
    Integer,
    Float,
    String,
    Boolean,
    Enum           // String with predefined options
};
```

### ConfigSection (Unified Structure)
Used by components to register their configuration requirements and for runtime caching.

```cpp
struct ConfigSection {
    std::string componentName;          // Component identifier (for logging)
    std::string sectionName;            // Unique section identifier
    std::string displayName;            // Human-readable section name
    std::vector<ConfigItem> items;      // Configuration items (includes metadata)
};
```

## Registration Flow

The system follows a specific registration and initialization flow to ensure metadata is preserved while values are loaded from NVS:

1. **Component Registration**: Component calls `RegisterConfigSection()` with defaults and metadata
2. **Cache Initialization**: PreferenceManager stores the complete section in cache (with metadata)
3. **NVS Override**: PreferenceManager loads any persisted values from NVS and overwrites only the `value` field
4. **Result**: Cache contains original metadata from registration, but current values from NVS

```cpp
// Example registration flow in PreferenceManager::RegisterConfigSection()
bool PreferenceManager::RegisterConfigSection(const ConfigSection& section) {
    std::lock_guard<std::mutex> lock(configMutex_);

    // Step 1: Store defaults with metadata in cache
    configSections_[section.sectionName] = section;

    // Step 2: Load persisted values from NVS (only overwrites 'value' field)
    LoadPersistedValues(configSections_[section.sectionName]);

    return true;
}

void PreferenceManager::LoadPersistedValues(ConfigSection& section) {
    for (auto& item : section.items) {
        // Construct NVS key
        std::string configId = section.sectionName + "." + item.key;

        // Load from NVS if exists, keeping metadata intact
        if (auto persistedValue = LoadFromNVS(configId)) {
            item.value = *persistedValue;  // Only override value, preserve metadata
        }
        // If not in NVS, keep the default value from registration
    }
}
```

## PreferenceManager Enhanced Interface

### Registration Methods

```cpp
class PreferenceManager : public IPreferenceService {
private:
    // Runtime cache (contains both values and metadata)
    std::map<std::string, ConfigSection> configSections_;

public:
    /**
     * @brief Register a configuration section with defaults and metadata
     * @param section Configuration section with defaults and UI metadata
     * @return true if registration successful, false if section already exists
     */
    bool RegisterConfigSection(const ConfigSection& section);

    /**
     * @brief Query a configuration value using constant identifier
     * @param configIdentifier Configuration identifier in format "section_name.item_key"
     * @return Configuration value or std::nullopt if not found
     */
    template<typename T>
    std::optional<T> QueryConfig(const std::string& configIdentifier) const;

    /**
     * @brief Get all registered section names for UI generation
     * @return Vector of section names that have been registered
     */
    std::vector<std::string> GetRegisteredSectionNames() const;

    /**
     * @brief Get configuration section (includes both values and UI metadata)
     * @param sectionName Name of the section to retrieve
     * @return Configuration section or std::nullopt if not found
     */
    std::optional<ConfigSection> GetConfigSection(const std::string& sectionName) const;

    /**
     * @brief Update configuration value using constant identifier
     * @param configIdentifier Configuration identifier in format "section_name.item_key"
     * @param value New value to set (updates both cache and NVS storage)
     * @return true if update successful, false if validation failed
     */
    template<typename T>
    bool UpdateConfig(const std::string& configIdentifier, const T& value);

private:
    std::map<std::string, ConfigSection> configSections_;
    mutable std::mutex configMutex_;  // Thread safety for multi-component registration

    /**
     * @brief Load persisted values from NVS and merge with defaults
     * Only key, displayName, type, and value are persisted (metadata excluded)
     */
    void LoadPersistedValues(ConfigSection& section);

    /**
     * @brief Save configuration values to NVS (excludes metadata)
     */
    bool SaveConfigToNVS(const std::string& sectionName, const std::string& key,
                         const std::variant<int, float, std::string, bool>& value);
};
```

## Component Integration Pattern

This pattern can be applied to **any component** in the Clarity application - sensors, panels, managers, providers, handlers, utilities, or custom components.

### Example 1: OilTemperatureSensor Configuration (Sensor Component)

```cpp
// In oil_temperature_sensor.h - Constant identifiers for configuration queries
class OilTemperatureSensor {
public:
    // Configuration identifiers - used for querying preferences
    static constexpr const char* CONFIG_UNIT = "oil_temperature_sensor.unit";
    static constexpr const char* CONFIG_UPDATE_RATE = "oil_temperature_sensor.update_rate";
    static constexpr const char* CONFIG_CALIBRATION_OFFSET = "oil_temperature_sensor.calibration_offset";

    // Other class members...
};

// In oil_temperature_sensor.cpp constructor
OilTemperatureSensor::OilTemperatureSensor(IGpioProvider* gpio, IPreferenceService* prefs)
    : gpioProvider_(gpio), preferenceService_(prefs)
{
    RegisterConfiguration();
    LoadConfiguration();
}

void OilTemperatureSensor::RegisterConfiguration() {
    if (!preferenceService_) return;

    ConfigSection section;
    section.componentName = "OilTemperatureSensor";
    section.sectionName = "oil_temperature_sensor";
    section.displayName = "Temperature Sensor";

    // Unit selection with sensor-provided options
    auto supportedUnits = GetSupportedUnits();
    ConfigItem unitItem;
    unitItem.key = "unit";
    unitItem.displayName = "Temperature Unit";
    unitItem.type = ConfigValueType::Enum;
    unitItem.value = std::string("C");  // Default value
    unitItem.metadata = "C,F,K";        // UI options (could be dynamic: supportedUnits)

    // Update rate configuration
    ConfigItem updateRateItem;
    updateRateItem.key = "update_rate";
    updateRateItem.displayName = "Update Rate (ms)";
    updateRateItem.type = ConfigValueType::Integer;
    updateRateItem.value = 500;         // Default value
    updateRateItem.metadata = "100,250,500,1000,2000,5000";  // Common update rates

    // Calibration settings
    ConfigItem offsetItem;
    offsetItem.key = "calibration_offset";
    offsetItem.displayName = "Calibration Offset";
    offsetItem.type = ConfigValueType::Float;
    offsetItem.value = 0.0f;            // Default value
    offsetItem.metadata = "-20.0-20.0"; // Range format for UI parsing

    section.items = {unitItem, updateRateItem, offsetItem};

    preferenceService_->RegisterConfigSection(section);
}

void OilTemperatureSensor::LoadConfiguration() {
    if (!preferenceService_) return;

    // Query individual configuration values using constant identifiers
    if (auto unit = preferenceService_->QueryConfig<std::string>(CONFIG_UNIT)) {
        SetTargetUnit(unit.value());
    }

    if (auto updateRate = preferenceService_->QueryConfig<int>(CONFIG_UPDATE_RATE)) {
        SetUpdateRate(updateRate.value());
    }

    if (auto offset = preferenceService_->QueryConfig<float>(CONFIG_CALIBRATION_OFFSET)) {
        calibrationOffset_ = offset.value();
    }
}
```

### Example 2: StyleManager Configuration (Manager Component)

```cpp
// In style_manager.h - Constant identifiers for configuration queries
class StyleManager {
public:
    // Configuration identifiers - used for querying preferences
    static constexpr const char* CONFIG_THEME = "style_manager.theme";
    static constexpr const char* CONFIG_BRIGHTNESS = "style_manager.brightness";

    // Other class members...
};

// In style_manager.cpp constructor
StyleManager::StyleManager(IPreferenceService* prefs) : preferenceService_(prefs) {
    RegisterConfiguration();
    LoadConfiguration();
}

void StyleManager::RegisterConfiguration() {
    if (!preferenceService_) return;

    ConfigSectionRegistration registration;
    registration.componentName = "StyleManager";
    registration.sectionName = "style_manager";
    registration.displayName = "Display & Theme";

    // Theme selection
    ConfigItemRegistration themeItem;
    themeItem.key = "theme";
    themeItem.displayName = "Theme";
    themeItem.type = ConfigValueType::Enum;
    themeItem.defaultValue = std::string("Day");
    themeItem.enumOptions = {"Day", "Night", "Auto"};  // UI options metadata

    // Brightness control
    ConfigItemRegistration brightnessItem;
    brightnessItem.key = "brightness";
    brightnessItem.displayName = "Brightness";
    brightnessItem.type = ConfigValueType::Integer;
    brightnessItem.defaultValue = 80;
    brightnessItem.minInt = 10;         // UI validation metadata
    brightnessItem.maxInt = 100;        // UI validation metadata

    registration.items = {themeItem, brightnessItem};

    preferenceService_->RegisterConfigSection(registration);
}
```

### Example 3: DisplayProvider Configuration (Provider Component)

```cpp
// In device_provider.h - Constant identifiers for configuration queries
class DeviceProvider {
public:
    // Configuration identifiers - used for querying preferences
    static constexpr const char* CONFIG_ROTATION = "display_provider.rotation";
    static constexpr const char* CONFIG_COLOR_INVERT = "display_provider.color_invert";

    // Other class members...
};

// In device_provider.cpp constructor
DeviceProvider::DeviceProvider(IPreferenceService* prefs) : preferenceService_(prefs) {
    RegisterConfiguration();
    LoadConfiguration();
}

void DeviceProvider::RegisterConfiguration() {
    if (!preferenceService_) return;

    ConfigSectionRegistration registration;
    registration.componentName = "DeviceProvider";
    registration.sectionName = "display_provider";
    registration.displayName = "Display Hardware";

    // Rotation setting
    ConfigItemRegistration rotationItem;
    rotationItem.key = "rotation";
    rotationItem.displayName = "Screen Rotation";
    rotationItem.type = ConfigValueType::Enum;
    rotationItem.defaultValue = 0;
    rotationItem.enumOptions = {"0°", "90°", "180°", "270°"};  // UI options metadata

    // Color inversion for different display types
    ConfigItemRegistration invertItem;
    invertItem.key = "color_invert";
    invertItem.displayName = "Invert Colors";
    invertItem.type = ConfigValueType::Boolean;
    invertItem.defaultValue = false;

    registration.items = {rotationItem, invertItem};

    preferenceService_->RegisterConfigSection(registration);
}
```

### Example 4: Custom Component Configuration

```cpp
// Any custom component can use this system
// Example: NetworkManager, DataLogger, AlertSystem, etc.

class CustomAlertComponent {
public:
    // Configuration identifiers - used for querying preferences
    static constexpr const char* CONFIG_ENABLED = "alert_system.enabled";
    static constexpr const char* CONFIG_VOLUME = "alert_system.volume";

private:
    IPreferenceService* preferenceService_;

public:
    CustomAlertComponent(IPreferenceService* prefs) : preferenceService_(prefs) {
        RegisterConfiguration();
        LoadConfiguration();
    }

private:
    void RegisterConfiguration() {
        if (!preferenceService_) return;

        ConfigSectionRegistration registration;
        registration.componentName = "CustomAlertComponent";
        registration.sectionName = "alert_system";
        registration.displayName = "Alert System";

        // Enable/disable alerts
        ConfigItemRegistration enableItem;
        enableItem.key = "enabled";
        enableItem.displayName = "Enable Alerts";
        enableItem.type = ConfigValueType::Boolean;
        enableItem.defaultValue = true;

        // Alert volume
        ConfigItemRegistration volumeItem;
        volumeItem.key = "volume";
        volumeItem.displayName = "Alert Volume";
        volumeItem.type = ConfigValueType::Integer;
        volumeItem.defaultValue = 70;
        volumeItem.minInt = 0;          // UI validation metadata
        volumeItem.maxInt = 100;        // UI validation metadata

        registration.items = {enableItem, volumeItem};

        preferenceService_->RegisterConfigSection(registration);
    }
};
```

### Component Usage Throughout Application

Components can use their constant identifiers anywhere in the application to query and update configuration values:

```cpp
// In any part of the application that needs temperature sensor config
void SomeOtherComponent::UpdateDisplay() {
    // Query the sensor's unit setting using its constant identifier
    if (auto unit = preferenceService_->QueryConfig<std::string>(OilTemperatureSensor::CONFIG_UNIT)) {
        displayUnit_ = unit.value();
        log_i("Using temperature unit: %s", displayUnit_.c_str());
    }

    // Query update rate for synchronization
    if (auto rate = preferenceService_->QueryConfig<int>(OilTemperatureSensor::CONFIG_UPDATE_RATE)) {
        synchronizeWithSensorRate(rate.value());
    }
}

// In oil_temperature_sensor.cpp - component querying and updating its own config
void OilTemperatureSensor::CheckConfigurationChange() {
    // Use own constant identifier to check if unit changed
    if (auto currentUnit = preferenceService_->QueryConfig<std::string>(CONFIG_UNIT)) {
        if (currentUnit.value() != targetUnit_) {
            log_i("Temperature unit changed from %s to %s", targetUnit_.c_str(), currentUnit.value().c_str());
            SetTargetUnit(currentUnit.value());
        }
    }
}

// In style_manager.cpp - using its own identifiers for query and update
void StyleManager::ApplyUserPreferences() {
    if (auto theme = preferenceService_->QueryConfig<std::string>(CONFIG_THEME)) {
        SetTheme(theme.value().c_str());
    }

    if (auto brightness = preferenceService_->QueryConfig<int>(CONFIG_BRIGHTNESS)) {
        SetBrightness(brightness.value());
    }
}

void StyleManager::UpdateBrightness(int newBrightness) {
    // Update configuration using constant identifier (updates cache and NVS)
    if (preferenceService_->UpdateConfig(CONFIG_BRIGHTNESS, newBrightness)) {
        SetBrightness(newBrightness);
        log_i("Brightness updated to %d", newBrightness);
    }
}

// ConfigPanel using identifiers constructed from cache for updates
void ConfigPanel::OnThemeSelected(const std::string& themeName) {
    // ConfigPanel constructs identifier from section and item information
    std::string themeIdentifier = "style_manager.theme";

    // Update both cache and NVS storage
    if (preferenceService_->UpdateConfig(themeIdentifier, themeName)) {
        log_i("Theme updated to: %s", themeName.c_str());
        // UI automatically refreshes with new values from cache
    }
}
```

## Configuration Storage Format

### ESP32 NVS Structure
Each configuration section is stored as a separate namespace in NVS for efficient access.

```
NVS Structure:
├── config_meta                    // Metadata namespace
│   ├── sections_list              // JSON array of section names
│   └── schema_version             // Configuration schema version
├── cfg_oil_temp_sensor             // Configuration namespace per section
│   ├── unit                       // Individual config items
│   ├── update_rate
│   └── calibration_offset
├── cfg_oil_pressure_sensor
│   ├── unit
│   ├── update_rate
│   └── calibration_offset
└── cfg_system_settings
    ├── theme
    ├── splash_enabled
    └── splash_duration
```

### Internal Implementation Details

```cpp
// PreferenceManager internal implementation example
template<typename T>
std::optional<T> PreferenceManager::QueryConfig(const std::string& configIdentifier) const {
    std::lock_guard<std::mutex> lock(configMutex_);

    // Parse identifier: "section_name.item_key"
    size_t dotPos = configIdentifier.find('.');
    if (dotPos == std::string::npos) return std::nullopt;

    std::string sectionName = configIdentifier.substr(0, dotPos);
    std::string itemKey = configIdentifier.substr(dotPos + 1);

    // Find section in cache
    auto sectionIt = configSections_.find(sectionName);
    if (sectionIt == configSections_.end()) return std::nullopt;

    // Find item in section
    auto& items = sectionIt->second.items;
    auto itemIt = std::find_if(items.begin(), items.end(),
                              [&itemKey](const ConfigItem& item) {
                                  return item.key == itemKey;
                              });

    if (itemIt == items.end()) return std::nullopt;

    // Return value with type safety
    try {
        return std::get<T>(itemIt->value);
    } catch (const std::bad_variant_access&) {
        return std::nullopt;
    }
}

template<typename T>
bool PreferenceManager::UpdateConfig(const std::string& configIdentifier, const T& value) {
    std::lock_guard<std::mutex> lock(configMutex_);

    // Parse and find item (same as QueryConfig)
    // ... parsing logic ...

    // Update cache
    itemIt->value = value;

    // Update NVS storage
    return WriteConfigToNVS(sectionName, itemKey, value);
}
```

## ConfigPanel UI Generation with Metadata

The ConfigPanel uses registration data (with UI metadata) to build dynamic menus and validation, while actual configuration values are stored separately.

### Menu Structure Generation

```cpp
void ConfigPanel::BuildDynamicMenus() {
    menuSections_.clear();

    // Get all registered configuration sections
    std::vector<std::string> sectionNames = preferenceService_->GetRegisteredSectionNames();

    // Build main menu
    std::vector<MenuItem> mainMenu;
    for (const auto& sectionName : sectionNames) {
        if (auto section = preferenceService_->GetConfigSection(sectionName)) {
            MenuItem sectionItem;
            sectionItem.label = section->displayName;    // From section
            sectionItem.actionType = "enter_section";
            sectionItem.actionParam = section->sectionName;
            mainMenu.push_back(sectionItem);
        }
    }

    mainMenu.push_back({"Exit", "panel_exit", ""});
    menuSections_["main"] = mainMenu;

    // Build section-specific menus
    for (const auto& sectionName : sectionNames) {
        if (auto section = preferenceService_->GetConfigSection(sectionName)) {
            BuildSectionMenu(*section);  // Pass section with metadata
        }
    }
}

void ConfigPanel::BuildSectionMenu(const ConfigSection& section) {
    std::vector<MenuItem> sectionMenu;

    // Build menu items using metadata
    for (const auto& item : section.items) {
        MenuItem menuItem;
        menuItem.label = FormatItemLabel(item);  // Uses current value + display name

        if (item.type == ConfigValueType::Enum && !item.metadata.empty()) {
            menuItem.actionType = "enter_enum_selection";
            menuItem.actionParam = section.sectionName + ":" + item.key;
        } else if (item.type == ConfigValueType::Boolean) {
            menuItem.actionType = "toggle_boolean";
            menuItem.actionParam = section.sectionName + ":" + item.key;
        } else {
            menuItem.actionType = "edit_value";
            menuItem.actionParam = section.sectionName + ":" + item.key;
        }

        sectionMenu.push_back(menuItem);
    }

    sectionMenu.push_back({"Back", "menu_back", ""});
    menuSections_[section.sectionName] = sectionMenu;
}

std::string ConfigPanel::FormatItemLabel(const ConfigItem& item) {
    // Format current value for display
    std::string currentValueStr;
    std::visit([&](const auto& value) {
        using T = std::decay_t<decltype(value)>;
        if constexpr (std::is_same_v<T, bool>) {
            currentValueStr = value ? "On" : "Off";
        } else if constexpr (std::is_same_v<T, std::string>) {
            currentValueStr = value;
        } else {
            currentValueStr = std::to_string(value);
        }
    }, item.value);

    return item.displayName + ": " + currentValueStr;
}
```

### Value Editing Interface with Metadata

```cpp
void ConfigPanel::HandleValueEdit(const std::string& sectionName, const std::string& key) {
    // Get section with metadata
    auto section = preferenceService_->GetConfigSection(sectionName);
    if (!section) return;

    auto itemIt = std::find_if(section->items.begin(), section->items.end(),
                              [&key](const ConfigItem& item) { return item.key == key; });

    if (itemIt == section->items.end()) return;

    // Construct the configuration identifier for value access
    std::string configIdentifier = sectionName + "." + key;

    switch (itemIt->type) {
        case ConfigValueType::Integer:
            ShowIntegerEditor(*itemIt, configIdentifier);
            break;
        case ConfigValueType::Float:
            ShowFloatEditor(*itemIt, configIdentifier);
            break;
        case ConfigValueType::String:
            ShowStringEditor(*itemIt, configIdentifier);
            break;
        case ConfigValueType::Boolean:
            ToggleBooleanValue(*itemIt, configIdentifier);
            break;
        case ConfigValueType::Enum:
            ShowEnumSelector(*itemIt, configIdentifier);   // Uses metadata for options
            break;
    }
}

void ConfigPanel::ShowEnumSelector(const ConfigItem& item, const std::string& configId) {
    // Parse comma-delimited options from metadata
    std::vector<std::string> options = ParseMetadataOptions(item.metadata);

    // Get current value
    std::string current = std::get<std::string>(item.value);

    // Build enum selection menu
    std::vector<MenuItem> enumMenu;
    for (const auto& option : options) {
        MenuItem optionItem;
        optionItem.label = option + (option == current ? " ✓" : "");
        optionItem.actionType = "select_enum_value";
        optionItem.actionParam = configId + ":" + option;
        enumMenu.push_back(optionItem);
    }

    enumMenu.push_back({"Back", "menu_back", ""});
    ShowMenu(enumMenu);
}

// Helper function to parse comma-delimited metadata
std::vector<std::string> ConfigPanel::ParseMetadataOptions(const std::string& metadata) {
    std::vector<std::string> options;
    if (metadata.empty()) return options;

    std::stringstream ss(metadata);
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

void ConfigPanel::ShowIntegerEditor(const ConfigItem& item, const std::string& configId) {
    // Parse metadata for integer constraints/options
    if (!item.metadata.empty()) {
        // Check if metadata contains predefined options (comma-separated)
        if (item.metadata.find(',') != std::string::npos) {
            ShowIntegerSelector(item, configId);  // Show selection from predefined values
            return;
        }

        // Check if metadata contains a range (e.g., "1-100")
        auto rangePos = item.metadata.find('-');
        if (rangePos != std::string::npos) {
            int minValue = std::stoi(item.metadata.substr(0, rangePos));
            int maxValue = std::stoi(item.metadata.substr(rangePos + 1));

            int current = std::get<int>(item.value);
            ShowIntegerInput(item.displayName, current, minValue, maxValue,
                            [this, configId](int newValue) {
                                OnValueChanged(configId, newValue);
                            });
            return;
        }
    }

    // Default: free input
    int current = std::get<int>(item.value);
    ShowIntegerInput(item.displayName, current, INT_MIN, INT_MAX,
                    [this, configId](int newValue) {
                        OnValueChanged(configId, newValue);
                    });
}

void ConfigPanel::ShowFloatEditor(const ConfigItem& item, const std::string& configId) {
    // Parse metadata for float constraints/options
    if (!item.metadata.empty()) {
        // Check for range format (e.g., "-20.0-20.0")
        auto rangePos = item.metadata.find('-', 1);  // Start from 1 to handle negative values
        if (rangePos != std::string::npos) {
            float minValue = std::stof(item.metadata.substr(0, rangePos));
            float maxValue = std::stof(item.metadata.substr(rangePos + 1));

            float current = std::get<float>(item.value);
            ShowFloatInput(item.displayName, current, minValue, maxValue,
                          [this, configId](float newValue) {
                              OnValueChanged(configId, newValue);
                          });
            return;
        }
    }

    // Default: free input
    float current = std::get<float>(item.value);
    ShowFloatInput(item.displayName, current, -FLT_MAX, FLT_MAX,
                  [this, configId](float newValue) {
                      OnValueChanged(configId, newValue);
                  });
}

void ConfigPanel::OnValueChanged(const std::string& configIdentifier, const std::variant<int, float, std::string, bool>& newValue) {
    // Update configuration using the identifier (updates cache and NVS)
    std::visit([&](const auto& value) {
        preferenceService_->UpdateConfig(configIdentifier, value);
    }, newValue);

    // Refresh the current menu to show updated values
    RefreshCurrentMenu();
}
```

## Registration with Options: Key Benefits

### 1. Clean Separation of Concerns
- **Storage**: Only actual configuration values stored in cache/NVS
- **UI Metadata**: Options, validation ranges, and display info kept separately
- **Runtime Queries**: Components access values efficiently without UI overhead

### 2. Dynamic Options Support
```cpp
// Components can provide dynamic options during registration
auto supportedUnits = GetSupportedUnits();  // Could return {"C", "F"} or {"C", "F", "K"}
unitItem.enumOptions = supportedUnits;      // UI gets actual sensor capabilities
```

### 3. No Storage Overhead
- Enum options like `{"Day", "Night", "Auto"}` stored once in registration
- Validation ranges stored once in registration
- Only selected values (`"Day"`, `80`, `true`) stored in cache/NVS

### 4. Type-Safe Validation
```cpp
// Registration provides validation metadata
updateRateItem.minInt = 100;
updateRateItem.maxInt = 5000;

// ConfigPanel uses metadata for validation
ShowIntegerInput(item.displayName, current, item.minInt, item.maxInt, callback);
```

### 5. Simplified Initialization Process
```cpp
bool PreferenceManager::RegisterConfigSection(const ConfigSection& section) {
    std::lock_guard<std::mutex> lock(configMutex_);

    // Store complete section (defaults + metadata) in cache
    configSections_[section.sectionName] = section;

    // Load persisted values and override only the 'value' field
    LoadPersistedValues(configSections_[section.sectionName]);

    return true;
}

void PreferenceManager::LoadPersistedValues(ConfigSection& section) {
    for (auto& item : section.items) {
        std::string configId = section.sectionName + "." + item.key;

        // Only override value if found in NVS, preserve metadata
        if (auto persistedValue = LoadFromNVS(configId)) {
            item.value = *persistedValue;
        }
        // Default value and metadata remain from registration
    }
}

## Implementation Phases

### Phase 1: Core Infrastructure
1. **ConfigItemRegistration and ConfigSectionRegistration structures** (with UI metadata)
2. **ConfigItem and ConfigSection structures** (values only for storage)
3. **PreferenceManager dual storage system** (registration data + runtime cache)
4. **NVS storage layer with section-based namespaces**
5. **Query and update methods using constant identifiers**

### Phase 2: Component Integration
1. **Update OilTemperatureSensor to use new system**
2. **Update OilPressureSensor to use new system**
3. **Update any other configurable components (panels, managers, providers, etc.)**
4. **Migrate existing system preferences**
5. **Backward compatibility layer**

### Phase 3: Dynamic UI Generation
1. **ConfigPanel menu generation from registration metadata**
2. **Type-specific value editors using validation ranges**
3. **Enum selectors using registration enumOptions**
4. **Type-appropriate input controls with metadata-driven validation**

### Phase 4: Advanced Features
1. **Configuration import/export**
2. **Configuration presets**
3. **Runtime configuration change notifications**
4. **Configuration dependency management**

## Migration Strategy

### Existing Configuration Compatibility
Current `Configs` struct will be migrated to the new system:

```cpp
// Migration function to convert existing preferences
void MigrateExistingConfiguration() {
    // Read current config
    Configs oldConfig = LoadLegacyConfig();

    // Convert to new section-based format
    ConfigSection systemSection = ConvertSystemConfig(oldConfig);
    ConfigSection sensorSection = ConvertSensorConfig(oldConfig);

    // Register converted sections
    RegisterConfigSection({"SystemMigration", systemSection});
    RegisterConfigSection({"SensorMigration", sensorSection});

    // Mark migration complete
    SetMigrationFlag();
}
```

## Benefits

### For Developers
- **Universal Component Support**: Any application component can easily add configuration
- **Simplified Configuration**: Components just register their needs and query values
- **Automatic UI Generation**: No manual config panel updates needed
- **Type Safety**: Compile-time checking of configuration types
- **Constant Identifiers**: Simple string-based configuration access

### For Users
- **Consistent Interface**: All configuration follows the same UI patterns
- **Better Organization**: Logical grouping of related settings
- **Dynamic Options**: Available choices reflect actual component capabilities
- **Type-safe Input**: Appropriate input controls for each configuration type

### For System
- **Universal Extensibility**: Adding any type of component automatically extends configuration
- **Maintainability**: Configuration logic centralized in PreferenceManager
- **Reliability**: Strong typing prevents configuration type errors
- **Performance**: Section-based storage enables efficient partial updates

## Error Handling

### Registration Failures
- Duplicate section registration (component restart scenario)
- NVS storage failures (space exhaustion)
- Invalid configuration values during registration

### Runtime Failures
- Configuration corruption in NVS
- Component configuration query failures
- UI generation errors for unknown types

### Recovery Strategies
- Automatic fallback to default values
- Section-by-section recovery (don't fail entire system)
- Re-registration of corrupted sections with defaults
- Logging and error reporting through ErrorManager

## Testing Strategy

### Unit Tests
- ConfigItem structure and type handling
- PreferenceManager registration and retrieval
- NVS storage and recovery
- Menu generation algorithms

### Integration Tests
- Component registration during startup
- End-to-end configuration change flow
- Migration from legacy configuration
- Configuration persistence across reboots

### System Tests
- Multi-component registration scenarios
- Configuration panel navigation
- Error recovery and fallback behavior
- Performance with large configuration sets

## Future Enhancements

### Advanced Configuration Types
- **Arrays**: Lists of similar items (e.g., sensor calibration points)
- **Objects**: Nested configuration structures
- **References**: Configuration values that reference other sections

### UI Improvements
- **Search**: Find configuration items across all sections
- **Favorites**: Quick access to frequently changed settings
- **Groups**: Visual grouping of related items within sections
- **Wizards**: Guided configuration for complex setups

### System Integration
- **Remote Configuration**: Update settings via web interface
- **Configuration Profiles**: Save/restore complete system configurations
- **Conditional Configuration**: Settings that depend on other settings
- **Live Updates**: Some settings take effect immediately without restart

This dynamic configuration system transforms Clarity from having hardcoded configuration options to a flexible, extensible system where components define their own configuration needs and the UI adapts automatically.