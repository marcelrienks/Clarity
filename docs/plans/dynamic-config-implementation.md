# Dynamic Configuration System Implementation Plan

## Executive Summary

This document outlines a comprehensive plan to migrate Clarity's current monolithic configuration system to a dynamic, component-driven configuration management system. The new architecture will enable any component to self-register configuration requirements, automatic UI generation, and improved maintainability.

## Current System Analysis

### Existing Architecture
- **Single Configuration Structure**: All settings stored in one `Configs` struct
- **Hardcoded Options**: Configuration options defined at compile time
- **Manual UI Management**: ConfigPanel requires manual updates for new settings
- **Centralized Storage**: Single JSON object in ESP32 NVS under "config" key
- **String-based Access**: Key-value access with manual type conversion

### Current Components with Configuration
1. **OemOilPanel**: Uses update rate, pressure/temp units
2. **OilTemperatureSensor**: Uses calibration settings
3. **OilPressureSensor**: Uses calibration settings
4. **StyleManager**: Uses theme setting
5. **SplashPanel**: Uses splash settings

### Current Configuration Options
| Setting | Type | Options | Default |
|---------|------|---------|---------|
| panelName | string | "OilPanel" | "OilPanel" |
| showSplash | bool | true/false | true |
| splashDuration | int | 1500/1750/2000/2500 | 1500 |
| theme | string | "Day"/"Night" | "Day" |
| updateRate | int | 250/500/1000/2000 | 500 |
| pressureUnit | string | "PSI"/"Bar"/"kPa" | "Bar" |
| tempUnit | string | "C"/"F" | "C" |
| pressureOffset | float | -1.0/-0.1/0.0/+0.1/+1.0 | 0.0 |
| pressureScale | float | 0.900/0.950/1.000/1.050/1.100 | 1.0 |
| tempOffset | float | -5.0/-1.0/0.0/+1.0/+5.0 | 0.0 |
| tempScale | float | 0.900/0.950/1.000/1.050/1.100 | 1.0 |

## New Architecture Design

### Core Components
1. **ConfigItem**: Individual configuration item with value and metadata
2. **ConfigSection**: Grouped configuration items for a component
3. **PreferenceManager**: Enhanced with registration and dynamic storage
4. **ConfigPanel**: Automatic UI generation from registered configurations

### Key Architectural Changes
1. **Component Self-Registration**: Components register config needs during construction
2. **Metadata-Driven UI**: UI automatically generated from registration metadata
3. **Sectioned Storage**: NVS organized by component sections instead of single JSON
4. **Type-Safe Access**: Template-based configuration queries with compile-time safety
5. **Constant Identifiers**: Components define static identifiers for configuration access

## Implementation Phases

### Phase 1: Core Infrastructure (Week 1-2)
**Goal**: Establish new configuration data structures and basic PreferenceManager enhancements

#### 1.1 Data Structure Implementation
- [ ] Create `ConfigValueType` enum (Integer, Float, String, Boolean, Enum)
- [ ] Implement `ConfigItem` struct with value, metadata, and type information
- [ ] Implement `ConfigSection` struct for component grouping
- [ ] Add template-based variant handling for type safety

#### 1.2 Enhanced PreferenceManager
- [ ] Add `RegisterConfigSection()` method for component registration
- [ ] Implement template-based `QueryConfig<T>()` with constant identifiers
- [ ] Add `UpdateConfig<T>()` method for type-safe updates
- [ ] Implement sectioned NVS storage (separate namespace per component)
- [ ] Create runtime configuration cache with metadata preservation
- [ ] Add `GetRegisteredSectionNames()` and `GetConfigSection()` for UI queries

#### 1.3 Migration Support
- [ ] Create migration function to convert existing `Configs` struct to new format
- [ ] Implement backward compatibility layer for existing component access patterns
- [ ] Add migration flag and detection logic

### Phase 2: Component Integration (Week 3-4)
**Goal**: Migrate existing components to use new configuration system

#### 2.1 OilTemperatureSensor Migration
- [ ] Define static configuration identifiers (CONFIG_UNIT, CONFIG_UPDATE_RATE, etc.)
- [ ] Implement `RegisterConfiguration()` method with defaults and metadata
- [ ] Update component to use `QueryConfig<T>()` instead of direct Config access
- [ ] Add configuration change detection and response logic

#### 2.2 OilPressureSensor Migration
- [ ] Apply same pattern as OilTemperatureSensor
- [ ] Define pressure-specific calibration constants
- [ ] Register pressure unit options and calibration ranges

#### 2.3 StyleManager Migration
- [ ] Register theme configuration with "Day"/"Night" enum options
- [ ] Update theme application to use new configuration queries
- [ ] Plan for future brightness and display settings

#### 2.4 SplashPanel Migration
- [ ] Register splash enable/disable and duration settings
- [ ] Define duration options metadata for UI generation
- [ ] Update splash logic to query new configuration system

#### 2.5 System Settings Migration
- [ ] Create SystemManager component for panel selection and general settings
- [ ] Register update rate configuration (used by multiple components)
- [ ] Handle cross-component configuration dependencies

### Phase 3: Dynamic UI Generation (Week 5-6)
**Goal**: Replace hardcoded ConfigPanel menus with dynamic generation

#### 3.1 ConfigPanel Enhancement
- [ ] Implement `BuildDynamicMenus()` using registered section information
- [ ] Create type-specific value editors (integer, float, string, boolean, enum)
- [ ] Add metadata parsing for validation ranges and enum options
- [ ] Implement enum selector with comma-delimited metadata parsing

#### 3.2 Input Controls
- [ ] Create `ShowIntegerEditor()` with range validation from metadata
- [ ] Create `ShowFloatEditor()` with range constraints
- [ ] Create `ShowEnumSelector()` with dynamic option lists
- [ ] Create `ShowStringEditor()` for free-text configuration
- [ ] Implement boolean toggle with custom labels from metadata

#### 3.3 Menu Navigation
- [ ] Replace hardcoded menu states with dynamic section-based navigation
- [ ] Implement breadcrumb navigation for deep menu structures
- [ ] Add menu refresh logic when configuration values change

### Phase 4: Advanced Features (Week 7-8)
**Goal**: Add advanced configuration management capabilities

#### 4.1 Validation and Constraints
- [ ] Add range validation for integer and float types
- [ ] Implement enum validation against registered options
- [ ] Add cross-configuration dependency validation
- [ ] Create validation error reporting and user feedback

#### 4.2 Configuration Profiles
- [ ] Implement configuration import/export functionality
- [ ] Add predefined configuration presets (Racing, Street, Economy)
- [ ] Create configuration backup and restore capabilities

#### 4.3 Runtime Configuration Updates
- [ ] Add configuration change notification system
- [ ] Implement live configuration updates without restart
- [ ] Add configuration change logging and audit trail

## Migration Strategy

### Existing Configuration Compatibility

#### Phase 1: Dual System Support
```cpp
// Legacy access still works
Configs& config = preferenceService_->GetConfig();
int updateRate = config.updateRate;

// New access becomes available
auto updateRate = preferenceService_->QueryConfig<int>("system_settings.update_rate");
```

#### Phase 2: Component-by-Component Migration
Each component migrates independently:
1. Component registers its configuration section
2. Component updates internal logic to use new queries
3. Legacy Config struct field remains available during transition
4. Configuration values automatically synchronized between systems

#### Phase 3: Legacy Removal
After all components migrated:
1. Remove legacy GetConfig/SetConfig methods
2. Clean up old JSON key mappings
3. Remove monolithic Configs struct
4. Update documentation and examples

### NVS Storage Evolution

#### Current Format
```
NVS:
└── clarity (namespace)
    └── config (key) → JSON string with all settings
```

#### New Format
```
NVS:
├── config_meta (namespace)
│   ├── sections_list → ["oil_temp_sensor", "oil_pressure_sensor", "style_manager"]
│   └── schema_version → "1.0"
├── cfg_oil_temp_sensor (namespace)
│   ├── unit → "C"
│   ├── update_rate → 500
│   └── calibration_offset → 0.0
├── cfg_oil_pressure_sensor (namespace)
│   ├── unit → "Bar"
│   ├── update_rate → 500
│   └── calibration_offset → 0.0
└── cfg_style_manager (namespace)
    ├── theme → "Day"
    └── brightness → 80
```

### Data Migration Process
```cpp
void MigrateExistingConfiguration() {
    // 1. Check if migration needed
    if (preferences_.getString("config", "").length() == 0) return;

    // 2. Load legacy configuration
    Configs legacyConfig = LoadLegacyConfig();

    // 3. Register component sections with current values
    MigrateOilTemperatureSensorConfig(legacyConfig);
    MigrateOilPressureSensorConfig(legacyConfig);
    MigrateStyleManagerConfig(legacyConfig);
    MigrateSplashPanelConfig(legacyConfig);

    // 4. Remove legacy config and mark migration complete
    preferences_.remove("config");
    preferences_.putBool("migration_completed", true);
}
```

## Component Integration Examples

### OilTemperatureSensor Implementation
```cpp
// oil_temperature_sensor.h
class OilTemperatureSensor {
public:
    static constexpr const char* CONFIG_UNIT = "oil_temperature_sensor.unit";
    static constexpr const char* CONFIG_UPDATE_RATE = "oil_temperature_sensor.update_rate";
    static constexpr const char* CONFIG_CALIBRATION_OFFSET = "oil_temperature_sensor.calibration_offset";
    static constexpr const char* CONFIG_CALIBRATION_SCALE = "oil_temperature_sensor.calibration_scale";

private:
    void RegisterConfiguration();
    void LoadConfiguration();
};

// oil_temperature_sensor.cpp
void OilTemperatureSensor::RegisterConfiguration() {
    ConfigSection section;
    section.componentName = "OilTemperatureSensor";
    section.sectionName = "oil_temperature_sensor";
    section.displayName = "Temperature Sensor";

    // Unit selection
    ConfigItem unitItem;
    unitItem.key = "unit";
    unitItem.displayName = "Temperature Unit";
    unitItem.type = ConfigValueType::Enum;
    unitItem.value = std::string("C");
    unitItem.metadata = "C,F,K";

    // Update rate
    ConfigItem updateRateItem;
    updateRateItem.key = "update_rate";
    updateRateItem.displayName = "Update Rate (ms)";
    updateRateItem.type = ConfigValueType::Integer;
    updateRateItem.value = 500;
    unitItem.metadata = "100,250,500,1000,2000,5000";

    // Calibration offset
    ConfigItem offsetItem;
    offsetItem.key = "calibration_offset";
    offsetItem.displayName = "Calibration Offset";
    offsetItem.type = ConfigValueType::Float;
    offsetItem.value = 0.0f;
    offsetItem.metadata = "-20.0-20.0"; // Range format

    section.items = {unitItem, updateRateItem, offsetItem};
    preferenceService_->RegisterConfigSection(section);
}

void OilTemperatureSensor::LoadConfiguration() {
    if (auto unit = preferenceService_->QueryConfig<std::string>(CONFIG_UNIT)) {
        SetTargetUnit(unit.value());
    }
    if (auto rate = preferenceService_->QueryConfig<int>(CONFIG_UPDATE_RATE)) {
        SetUpdateRate(rate.value());
    }
    if (auto offset = preferenceService_->QueryConfig<float>(CONFIG_CALIBRATION_OFFSET)) {
        calibrationOffset_ = offset.value();
    }
}
```

### Dynamic UI Generation Example
```cpp
// ConfigPanel automatic menu generation
void ConfigPanel::BuildDynamicMenus() {
    menuSections_.clear();

    // Get all registered sections
    auto sectionNames = preferenceService_->GetRegisteredSectionNames();

    // Build main menu dynamically
    std::vector<MenuItem> mainMenu;
    for (const auto& sectionName : sectionNames) {
        if (auto section = preferenceService_->GetConfigSection(sectionName)) {
            MenuItem item;
            item.label = section->displayName;
            item.actionType = "enter_section";
            item.actionParam = sectionName;
            mainMenu.push_back(item);
        }
    }

    // Build section-specific menus
    for (const auto& sectionName : sectionNames) {
        BuildSectionMenu(sectionName);
    }
}

void ConfigPanel::BuildSectionMenu(const std::string& sectionName) {
    auto section = preferenceService_->GetConfigSection(sectionName);
    if (!section) return;

    std::vector<MenuItem> sectionMenu;
    for (const auto& item : section->items) {
        MenuItem menuItem;
        menuItem.label = FormatItemLabel(item); // Shows current value

        switch (item.type) {
            case ConfigValueType::Enum:
                menuItem.actionType = "show_enum_selector";
                break;
            case ConfigValueType::Boolean:
                menuItem.actionType = "toggle_boolean";
                break;
            case ConfigValueType::Integer:
            case ConfigValueType::Float:
                menuItem.actionType = "show_numeric_editor";
                break;
            case ConfigValueType::String:
                menuItem.actionType = "show_string_editor";
                break;
        }

        menuItem.actionParam = sectionName + ":" + item.key;
        sectionMenu.push_back(menuItem);
    }

    menuSections_[sectionName] = sectionMenu;
}
```

## Testing Strategy

### Unit Tests
- [ ] ConfigItem structure serialization/deserialization
- [ ] PreferenceManager registration and query functionality
- [ ] Template type safety and variant handling
- [ ] NVS storage and retrieval for sectioned configuration
- [ ] Migration logic for legacy configurations

### Integration Tests
- [ ] Component registration during system startup
- [ ] End-to-end configuration change flow (UI → Storage → Component)
- [ ] Cross-component configuration dependencies
- [ ] Configuration persistence across ESP32 reboots

### System Tests
- [ ] Multi-component registration and conflict resolution
- [ ] ConfigPanel dynamic menu generation and navigation
- [ ] Error recovery and fallback to defaults
- [ ] Performance impact of new configuration system
- [ ] Memory usage optimization for ESP32 constraints

## Risk Assessment and Mitigation

### High Risk Items
1. **Configuration Data Loss During Migration**
   - **Risk**: Legacy configuration lost during migration
   - **Mitigation**: Backup existing NVS data, comprehensive migration testing, rollback capability

2. **Component Startup Dependencies**
   - **Risk**: Components register before PreferenceManager ready
   - **Mitigation**: Dependency injection order, deferred registration queue, startup sequence documentation

3. **Memory Constraints on ESP32**
   - **Risk**: New system uses too much RAM/Flash
   - **Mitigation**: Memory profiling, metadata optimization, lazy loading strategies

### Medium Risk Items
1. **UI Generation Performance**
   - **Risk**: Dynamic menu generation too slow
   - **Mitigation**: Menu caching, progressive loading, performance benchmarks

2. **Configuration Validation Complexity**
   - **Risk**: Complex validation rules cause issues
   - **Mitigation**: Simple validation initially, gradual enhancement, comprehensive testing

## Success Criteria

### Functional Requirements Met
- [ ] Any component can register configuration requirements
- [ ] ConfigPanel automatically generates UI for all registered configurations
- [ ] Type-safe configuration access with compile-time checking
- [ ] Backward compatibility maintained during migration
- [ ] Configuration persistence across ESP32 reboots

### Performance Requirements Met
- [ ] Configuration queries <1ms response time
- [ ] UI generation <500ms for complete menu rebuild
- [ ] Memory usage increase <10% compared to current system
- [ ] NVS write operations complete <100ms

### Quality Requirements Met
- [ ] Zero configuration data loss during migration
- [ ] 100% unit test coverage for core configuration classes
- [ ] Integration tests pass for all existing use cases
- [ ] Documentation updated for new development patterns

## Timeline and Milestones

| Phase | Duration | Key Deliverables | Success Metrics |
|-------|----------|------------------|----------------|
| Phase 1 | Week 1-2 | Core infrastructure, data structures | Unit tests pass, basic registration works |
| Phase 2 | Week 3-4 | Component migration, backward compatibility | All components use new system, legacy still works |
| Phase 3 | Week 5-6 | Dynamic UI generation | ConfigPanel auto-generates all menus |
| Phase 4 | Week 7-8 | Advanced features, optimization | Performance targets met, documentation complete |

## Future Enhancements

### Advanced Configuration Types
- **Arrays**: Lists of calibration points, multiple sensor configurations
- **Objects**: Nested configuration structures for complex components
- **References**: Configuration values that reference other sections

### Remote Configuration
- **Web Interface**: Configure device remotely via WiFi
- **Configuration Sync**: Synchronize settings across multiple devices
- **Cloud Profiles**: Store configuration profiles in cloud storage

### Intelligent Configuration
- **Auto-Calibration**: Automatic sensor calibration based on operating conditions
- **Adaptive Settings**: Configuration adjusts based on usage patterns
- **Predictive Configuration**: Suggest optimal settings based on vehicle type

## Conclusion

This implementation plan provides a structured approach to modernizing Clarity's configuration system while maintaining backward compatibility and minimizing risk. The phased approach ensures each milestone can be validated before proceeding, and the component-driven architecture will enable easy addition of new features and sensors in the future.

The new system will transform Clarity from a hardcoded configuration approach to a flexible, self-documenting, and maintainable configuration architecture that scales with the project's growth and complexity.