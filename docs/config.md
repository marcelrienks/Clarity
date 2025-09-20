# Dynamic Configuration System

## Overview

The Clarity configuration system enables components to self-register their settings with automatic UI generation, validation, and persistence. All configuration is managed through the `PreferenceManager` and displayed via the `ConfigPanel`. The system uses **type-safe constants** to eliminate hardcoded strings and provide compile-time validation.

## How It Works

1. **Component Registration**: Components register their configuration during initialization using constants
2. **System Registration**: System-wide settings are registered directly in main.cpp setup
3. **Automatic UI**: ConfigPanel dynamically builds menus based on registered configurations
4. **Type Safety**: Strong typing with compile-time validation via configuration constants
5. **Persistence**: Settings saved to ESP32 NVS storage with sectioned organization
6. **Live Updates**: Real-time configuration changes without restart

## Configuration Tree

The following tree shows all configuration sections as they appear in the ConfigPanel, organized by display order:

```
Configuration Menu
├── System (displayOrder: 0) [main.cpp]
│   ├── Default Panel (Enum) → Options: OemOilPanel,ConfigPanel,DiagnosticPanel | Default: OemOilPanel
│   ├── Update Rate (Integer) → Range: 100-2000 ms | Default: 500
│   ├── Show Splash (Boolean) → Default: true
│   └── Panel Name (String) → Default: "OEM Oil" [Legacy]
│
├── Oil Pressure Sensor (displayOrder: 2) [OilPressureSensor]
│   ├── Pressure Unit (Enum) → Options: PSI,Bar,kPa | Default: Bar
│   ├── Update Rate (ms) (Enum) → Options: 250,500,1000,2000 | Default: 500
│   ├── Calibration Offset (Float) → Range: -1.0 to 1.0 | Default: 0.0
│   └── Calibration Scale (Float) → Range: 0.9 to 1.1 | Default: 1.0
│
├── Oil Temperature Sensor (displayOrder: 3) [OilTemperatureSensor]
│   ├── Temperature Unit (Enum) → Options: C,F | Default: C
│   ├── Update Rate (ms) (Enum) → Options: 250,500,1000,2000 | Default: 500
│   ├── Calibration Offset (Float) → Range: -5.0 to 5.0 | Default: 0.0
│   └── Calibration Scale (Float) → Range: 0.9 to 1.1 | Default: 1.0
│
├── Display (displayOrder: 10) [StyleManager]
│   ├── Theme (Enum) → Options: Day,Night | Default: Day
│   └── Brightness (Integer) → Range: 0-100% | Default: 80
│
└── Splash Screen (displayOrder: 20) [SplashPanel]
    └── Duration (Enum) → Options: 1500,1750,2000,2500 ms | Default: 1500
```

## Configuration Constants

All configuration keys are defined as constants in their respective headers to ensure type safety and eliminate hardcoded strings.

### System Configuration (`include/config/system_config.h`)

```cpp
namespace SystemConfig {
    // Configuration section and keys
    static constexpr const char* CONFIG_SECTION = "system";
    static constexpr const char* CONFIG_DEFAULT_PANEL = "system.default_panel";
    static constexpr const char* CONFIG_UPDATE_RATE = "system.update_rate";
    static constexpr const char* CONFIG_SHOW_SPLASH = "system.show_splash";
}
```

### Component Constants Pattern

Each component defines its constants in its header file:

```cpp
// Example: include/sensors/oil_pressure_sensor.h
class OilPressureSensor {
public:
    // Configuration Constants
    static constexpr const char* CONFIG_SECTION = "oil_pressure";
    static constexpr const char* CONFIG_UNIT = "oil_pressure.unit";
    static constexpr const char* CONFIG_UPDATE_RATE = "oil_pressure.update_rate";
    static constexpr const char* CONFIG_OFFSET = "oil_pressure.offset";
    static constexpr const char* CONFIG_SCALE = "oil_pressure.scale";
};
```

## System Configuration (main.cpp)

System-wide settings are registered directly in main.cpp during application startup using constants for all configuration keys.

### Registration Implementation

```cpp
#include "config/system_config.h"

/**
 * @brief Register system-wide configuration settings
 */
void registerSystemConfiguration()
{
    if (!preferenceManager) {
        log_e("Cannot register system configuration - preference manager not available");
        return;
    }

    using namespace Config;

    ConfigSection section("System", SystemConfig::CONFIG_SECTION, "System Settings");
    section.displayOrder = 0; // Highest priority - show first in config menu

    // Default panel selection - controls which panel loads at startup
    section.AddItem(ConfigItem("default_panel", "Default Panel", ConfigValueType::Enum,
        std::string("OemOilPanel"), ConfigMetadata("OemOilPanel,ConfigPanel,DiagnosticPanel")));

    // Global update rate for sensors and components
    section.AddItem(ConfigItem("update_rate", "Update Rate", ConfigValueType::Integer,
        500, ConfigMetadata("100-2000", "ms")));

    // Splash screen control
    section.AddItem(ConfigItem("show_splash", "Show Splash Screen", ConfigValueType::Boolean,
        true, ConfigMetadata()));

    // Legacy panel name setting (kept for backward compatibility)
    section.AddItem(ConfigItem("panel_name", "Panel Name", ConfigValueType::String,
        std::string("OEM Oil"), ConfigMetadata()));

    preferenceManager->RegisterConfigSection(section);
    log_i("System configuration registered");
}

void setup() {
    // ... initialize services ...

    // Register system configuration after services are initialized
    registerSystemConfiguration();

    // Access system configuration using constants
    if (auto nameValue = preferenceManager->QueryConfig<std::string>(SystemConfig::CONFIG_DEFAULT_PANEL)) {
        panelName = *nameValue;
    }

    // ... rest of setup ...
}
```

## Component Registration Pattern

Individual components follow this pattern to register their configuration using constants:

```cpp
// Example: OilPressureSensor::RegisterConfiguration()
void OilPressureSensor::RegisterConfiguration() {
    if (!preferenceService_) return;

    using namespace Config;

    // Use constant for section name
    ConfigSection section("OilPressureSensor", CONFIG_SECTION, "Oil Pressure Sensor");
    section.displayOrder = 2; // Controls menu order

    // Add configuration items (key names only, full paths are constants)
    section.AddItem(ConfigItem("unit", "Pressure Unit", ConfigValueType::Enum,
        std::string("Bar"), ConfigMetadata("PSI,Bar,kPa")));

    section.AddItem(ConfigItem("update_rate", "Update Rate (ms)", ConfigValueType::Enum,
        500, ConfigMetadata("250,500,1000,2000")));

    preferenceService_->RegisterConfigSection(section);
}
```

## Type-Safe Configuration Access

Components access their configuration using constant identifiers defined in their headers:

```cpp
// System configuration access (any component can access these)
#include "config/system_config.h"

if (auto panel = preferenceManager->QueryConfig<std::string>(SystemConfig::CONFIG_DEFAULT_PANEL)) {
    panelManager->CreateAndLoadPanel(panel->c_str());
}

if (auto rate = preferenceManager->QueryConfig<int>(SystemConfig::CONFIG_UPDATE_RATE)) {
    // Use global update rate for component timing
}

// Component-specific configuration access
if (auto unit = preferenceService_->QueryConfig<std::string>(OilTemperatureSensor::CONFIG_UNIT)) {
    SetTargetUnit(*unit);
}

if (auto offset = preferenceService_->QueryConfig<float>(OilPressureSensor::CONFIG_OFFSET)) {
    SetCalibrationOffset(*offset);
}
```

## Cross-Component Configuration Access

Components can access other components' configuration by including their headers:

```cpp
// In oem_oil_panel.cpp
#include "sensors/oil_pressure_sensor.h"
#include "sensors/oil_temperature_sensor.h"

// Access sensor configuration for display purposes
if (auto unit = preferenceService_->QueryConfig<std::string>(OilPressureSensor::CONFIG_UNIT)) {
    updatePressureUnitDisplay(*unit);
}

if (auto unit = preferenceService_->QueryConfig<std::string>(OilTemperatureSensor::CONFIG_UNIT)) {
    updateTemperatureUnitDisplay(*unit);
}
```

## Storage Organization

Configuration is stored in ESP32 NVS with separate namespaces per component:

```
NVS Structure:
├── cfg_system                    // System settings (main.cpp)
├── cfg_oil_pressure             // Oil pressure sensor
├── cfg_oil_temperature          // Oil temperature sensor
├── cfg_style_manager            // Theme & display
└── cfg_splash_panel             // Splash screen
```

## Value Types and Validation

- **Enum**: Dropdown selection with comma-separated options in metadata
- **Integer**: Numeric input with range validation (e.g., "100-2000")
- **Float**: Decimal input with range validation (e.g., "-5.0,5.0")
- **Boolean**: Toggle switch (true/false)
- **String**: Text input (no validation by default)

## Registration Flow

1. **Application Startup** (main.cpp):
   - Initialize all services and managers
   - Register system configuration via `registerSystemConfiguration()`
   - System settings available immediately

2. **Component Initialization**:
   - Each component registers its configuration during initialization
   - Components can access system settings using constants
   - All configuration sections available to ConfigPanel

3. **Dynamic UI Generation**:
   - ConfigPanel automatically discovers all registered sections
   - Builds menus in display order
   - Provides type-appropriate input controls

## Adding New Configuration

### For System-Wide Settings:

1. **Add constant to `include/config/system_config.h`**:
```cpp
static constexpr const char* CONFIG_NEW_SETTING = "system.new_setting";
```

2. **Add item to `registerSystemConfiguration()` in main.cpp**:
```cpp
section.AddItem(ConfigItem("new_setting", "New Setting", ConfigValueType::Boolean,
    true, ConfigMetadata()));
```

3. **Access using constant**:
```cpp
if (auto value = preferenceManager->QueryConfig<bool>(SystemConfig::CONFIG_NEW_SETTING)) {
    // Use the value
}
```

### For Component Settings:

1. **Add constants to component header**:
```cpp
static constexpr const char* CONFIG_NEW_SETTING = "component_section.new_setting";
```

2. **Register in component's `RegisterConfiguration()` method**:
```cpp
section.AddItem(ConfigItem("new_setting", "New Setting", ConfigValueType::Boolean,
    true, ConfigMetadata()));
```

3. **Access using constant in component code**:
```cpp
if (auto value = preferenceService_->QueryConfig<bool>(CONFIG_NEW_SETTING)) {
    // Use the value
}
```

The ConfigPanel will automatically discover and display new configuration without any additional code changes.

## Architecture Benefits

- **Type Safety**: Compile-time validation of configuration keys via constants
- **Refactoring Safety**: IDE can track all usages when constants are renamed
- **No Magic Strings**: All configuration keys defined as named constants
- **Single Source of Truth**: Constants serve as configuration API documentation
- **Centralized System Settings**: Global configuration managed at application entry point
- **Component Autonomy**: Each component manages its own configuration needs
- **Cross-Component Access**: Type-safe access to other components' configuration
- **Automatic UI**: Zero-configuration UI generation based on registered sections
- **Persistent Storage**: Automatic NVS persistence with proper namespace isolation
- **Error Prevention**: Eliminates typos and mismatches in configuration key strings