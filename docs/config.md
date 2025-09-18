# Dynamic Configuration Management System

## Overview

The dynamic configuration system in Clarity enables components to self-register their configuration requirements with the PreferenceManager, which automatically handles persistence, validation, and UI generation. This documentation describes how the system currently works in the codebase.

## Core Principles

1. **Component Self-Registration**: Components register their configuration needs during initialization
2. **Default-First Architecture**: Default configurations are applied on first startup, persisted configurations used thereafter
3. **Dynamic UI Generation**: Config panel dynamically builds menus based on registered configurations
4. **Type Safety**: Strong typing for configuration values with validation
5. **Live Updates**: Real-time configuration changes without system restart

## Architecture Overview

```
┌─────────────────┐    Register    ┌──────────────────┐    Query     ┌─────────────────┐
│ Any Component   │ ───────────────► │ PreferenceManager│ ◄─────────── │   ConfigPanel   │
│                 │                 │                  │              │                 │
│ - Sensors       │                 │ - Config Cache   │              │ - Dynamic Menus │
│ - Panels        │                 │ - ESP32 NVS      │              │ - Validation    │
│ - Managers      │                 │ - Validation     │              │ - Persistence   │
│ - Components    │                 │                  │              │                 │
└─────────────────┘                 └──────────────────┘              └─────────────────┘
```

## Configuration Data Structures

### ConfigValue Type System

```cpp
namespace Config {
    enum class ConfigValueType {
        Integer,
        Float,
        String,
        Boolean,
        Enum           // String with predefined options
    };

    using ConfigValue = std::variant<int, float, std::string, bool>;
}
```

### ConfigItem Structure

```cpp
struct ConfigItem {
    std::string key;                    // Unique identifier within section
    std::string displayName;            // Human-readable name for UI
    ConfigValueType type;               // Data type (int, float, string, bool, enum)
    ConfigValue value;                  // Current/default value
    ConfigMetadata metadata;            // UI constraints and options

    ConfigItem(const std::string& k, const std::string& display,
               ConfigValueType t, const ConfigValue& val,
               const ConfigMetadata& meta = ConfigMetadata());
};
```

### ConfigSection Structure

```cpp
struct ConfigSection {
    std::string sectionName;            // Unique section identifier
    std::string storageKey;             // NVS storage key
    std::string displayName;            // Human-readable section name
    std::vector<ConfigItem> items;      // Configuration items in this section
    int displayOrder = 0;               // Order for UI display (lower = earlier)

    void AddItem(const ConfigItem& item);
};
```

## Current Configuration Hierarchy

The config panel displays sections ordered by `displayOrder` (lower numbers first):

```
Config Panel
├── System Settings (displayOrder: 0)
│   ├── Panel Name (String) - Default: "OEM Oil"
│   ├── Show Splash Screen (Boolean) - Default: true
│   └── Dynamic UI (Boolean) - Default: true
│
├── Oil Pressure Sensor (displayOrder: 2)
│   ├── Pressure Unit (Enum) - Options: PSI, Bar, kPa | Default: Bar
│   ├── Update Rate (ms) (Enum) - Options: 250, 500, 1000, 2000 | Default: 500
│   ├── Calibration Offset (Float) - Range: -1.0 to 1.0 | Default: 0.0
│   └── Calibration Scale (Float) - Range: 0.9 to 1.1 | Default: 1.0
│
├── Oil Temperature Sensor (displayOrder: 3)
│   ├── Temperature Unit (Enum) - Options: C, F | Default: C
│   ├── Update Rate (ms) (Enum) - Options: 250, 500, 1000, 2000 | Default: 500
│   ├── Calibration Offset (Float) - Range: -5.0 to 5.0 | Default: 0.0
│   └── Calibration Scale (Float) - Range: 0.9 to 1.1 | Default: 1.0
│
├── System Settings (displayOrder: 5) [SystemManager]
│   ├── Default Panel (Enum) - Options: OemOilPanel, ConfigPanel, DiagnosticPanel | Default: OemOilPanel
│   ├── Update Rate (Integer) - Range: 100-2000 ms | Default: 500
│   └── Show Splash Screen (Boolean) - Default: true
│
├── Theme & Display (displayOrder: 10)
│   ├── Theme (Enum) - Options: Day, Night | Default: Day
│   └── Brightness (Integer) - Range: 0-100% | Default: 80
│
└── Splash Screen (displayOrder: 20)
    ├── Show Splash Screen (Boolean) - Default: true
    └── Splash Duration (Enum) - Options: 1500, 1750, 2000, 2500 ms | Default: 1500
```

**Note**: There's currently a duplicate "System Settings" section (displayOrder 0 & 5) that needs resolution.

## Registration Flow

The system follows this registration and initialization flow:

1. **Component Registration**: Component calls `RegisterConfigSection()` with defaults and metadata
2. **Cache Initialization**: PreferenceManager stores the complete section in cache (with metadata)
3. **NVS Override**: PreferenceManager loads any persisted values from NVS but only if the key exists
4. **Result**: Cache contains original metadata from registration, current values from NVS or defaults

```cpp
bool PreferenceManager::RegisterConfigSection(const ConfigSection& section) {
    registeredSections_[section.sectionName] = section;

    // Load any existing persisted values for this section from NVS
    LoadConfigSection(section.sectionName);
    return true;
}

bool PreferenceManager::LoadConfigSection(const std::string& sectionName) {
    // ... setup preferences namespace ...

    for (auto& item : section.items) {
        // Only overwrite default if value exists in NVS
        if (preferences_.isKey(item.key.c_str())) {
            item.value = LoadValueFromNVS(preferences_, item.key, item.type);
        }
        // Otherwise preserve the default value
    }

    return true;
}
```

## Component Integration Pattern

### Example: Oil Temperature Sensor

```cpp
void OilTemperatureSensor::RegisterConfiguration() {
    if (!preferenceService_) return;

    using namespace Config;
    ConfigSection section("OilTemperatureSensor", "oil_temperature", "Oil Temperature Sensor");
    section.displayOrder = 3;

    // Temperature unit selection
    section.AddItem(ConfigItem("unit", "Temperature Unit", ConfigValueType::Enum,
        std::string("C"), ConfigMetadata("C,F")));

    // Update rate
    section.AddItem(ConfigItem("update_rate", "Update Rate (ms)", ConfigValueType::Enum,
        500, ConfigMetadata("250,500,1000,2000")));

    // Calibration parameters
    section.AddItem(ConfigItem("offset", "Calibration Offset", ConfigValueType::Float,
        0.0f, ConfigMetadata("-5.0,5.0")));
    section.AddItem(ConfigItem("scale", "Calibration Scale", ConfigValueType::Float,
        1.0f, ConfigMetadata("0.9,1.1")));

    preferenceService_->RegisterConfigSection(section);
}
```

### Live Configuration Updates

Components can register callbacks for real-time configuration changes:

```cpp
void OilTemperatureSensor::RegisterLiveUpdateCallbacks() {
    if (!preferenceService_) return;

    auto callback = [this](const std::string& fullKey,
                          const std::optional<Config::ConfigValue>& oldValue,
                          const Config::ConfigValue& newValue) {

        if (fullKey == CONFIG_UNIT) {
            if (auto newUnit = Config::ConfigValueHelper::GetValue<std::string>(newValue)) {
                SetTargetUnit(*newUnit);
                log_i("Temperature unit changed to: %s", newUnit->c_str());
            }
        }
        // Handle other configuration changes...
    };

    configCallbackId_ = preferenceService_->RegisterChangeCallback("oil_temperature", callback);
}
```

## Type-Safe Configuration Access

Components access configuration using constant identifiers:

```cpp
// Configuration constants (in oil_temperature_sensor.h)
static constexpr const char* CONFIG_UNIT = "oil_temperature.unit";
static constexpr const char* CONFIG_UPDATE_RATE = "oil_temperature.update_rate";

// Loading configuration
void OilTemperatureSensor::LoadConfiguration() {
    if (auto unitValue = preferenceService_->QueryConfig<std::string>(CONFIG_UNIT)) {
        SetTargetUnit(*unitValue);
    }

    if (auto rateValue = preferenceService_->QueryConfig<int>(CONFIG_UPDATE_RATE)) {
        SetUpdateRate(*rateValue);
    }
}
```

## ESP32 NVS Storage Structure

Each configuration section is stored as a separate namespace in NVS:

```
NVS Structure:
├── cfg_system                         // System settings namespace
│   ├── panel_name                     // Individual config items
│   ├── show_splash
│   └── dynamic_ui_enabled
├── cfg_oil_pressure                   // Oil pressure sensor namespace
│   ├── unit
│   ├── update_rate
│   ├── offset
│   └── scale
├── cfg_oil_temperature               // Oil temperature sensor namespace
│   ├── unit
│   ├── update_rate
│   ├── offset
│   └── scale
├── cfg_style_manager                 // Style manager namespace
│   ├── theme
│   └── brightness
└── cfg_splash_panel                  // Splash panel namespace
    ├── show_splash
    └── duration
```

## ConfigPanel UI Generation

The ConfigPanel generates its interface dynamically based on registered sections:

```cpp
void ConfigPanel::BuildDynamicMenus() {
    // Get all registered configuration sections
    std::vector<std::string> sectionNames = preferenceService_->GetRegisteredSectionNames();

    // Sort by displayOrder
    std::sort(sectionNames.begin(), sectionNames.end(), [this](const auto& a, const auto& b) {
        auto sectionA = preferenceService_->GetConfigSection(a);
        auto sectionB = preferenceService_->GetConfigSection(b);
        return sectionA->displayOrder < sectionB->displayOrder;
    });

    // Build menus for each section
    for (const auto& sectionName : sectionNames) {
        if (auto section = preferenceService_->GetConfigSection(sectionName)) {
            BuildSectionMenu(*section);
        }
    }
}
```

## Key Features

### Validation and Constraints

- **Enum types**: Options defined in metadata (e.g., "C,F" or "PSI,Bar,kPa")
- **Integer ranges**: Specified in metadata (e.g., "100-2000" or "250,500,1000,2000")
- **Float ranges**: Specified in metadata (e.g., "-5.0,5.0" or "0.9,1.1")
- **Boolean types**: Toggle interface with true/false values

### Persistence

- **Default preservation**: Missing NVS values don't overwrite defaults
- **Sectioned storage**: Each component gets its own NVS namespace
- **Type-safe loading**: Values loaded with proper type conversion
- **Atomic updates**: Individual configuration changes persist immediately

### Extensibility

Any component can add configuration by:
1. Implementing `RegisterConfiguration()` method
2. Calling `preferenceService_->RegisterConfigSection(section)`
3. Using `QueryConfig<T>()` to access values
4. Optionally registering live update callbacks

## Error Handling

### Registration Failures
- Graceful handling of missing preference service
- Logging of registration failures
- Fallback to default values when NVS unavailable

### Runtime Failures
- Type-safe configuration queries with std::optional return
- Validation of configuration values before storage
- Recovery from corrupted NVS data using defaults

## Benefits

### For Developers
- **Simple Integration**: Components just register their needs and query values
- **Automatic UI**: No manual config panel updates needed
- **Type Safety**: Compile-time checking of configuration types
- **Live Updates**: Real-time configuration changes without restart

### For Users
- **Consistent Interface**: All configuration follows the same UI patterns
- **Logical Organization**: Settings grouped by component function
- **Dynamic Options**: Available choices reflect actual capabilities
- **Immediate Feedback**: Changes take effect in real-time

### For System
- **Maintainability**: Configuration logic centralized
- **Reliability**: Strong typing prevents configuration errors
- **Performance**: Efficient sectioned storage and caching
- **Extensibility**: Easy addition of new configurable components