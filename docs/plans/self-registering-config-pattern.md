# Self-Registering Configuration Pattern

## Problem Statement

The current configuration architecture has a fundamental flaw: configuration sections are only available when their respective components are initialized. This means:

- If the Oil Pressure Sensor isn't loaded (e.g., when viewing the Config Panel), its settings aren't visible
- Users cannot configure components that aren't currently active
- The Config Panel menu changes based on which components happen to be initialized

This breaks the user experience and makes configuration management unpredictable.

## Design Goals

1. **Always Available**: All configuration sections must be available in the Config Panel regardless of component initialization state
2. **Distributed Ownership**: Components maintain responsibility for their own configuration schema
3. **Zero Coupling**: main.cpp should not need to know about specific components
4. **Dynamic Registration**: Adding new components should not require changes to central registration code
5. **Lazy Component Loading**: Components should still only be instantiated when actually needed
6. **Backward Compatible**: Minimize disruption to existing component code

## Proposed Solution: Self-Registering Static Pattern

### Overview

Components register their configuration schemas automatically at program startup using C++ static initialization. This happens before `main()` is called, ensuring all configurations are available immediately.

### Architecture

```
┌─────────────────────────────────────────────────────────┐
│                     Program Startup                      │
├─────────────────────────────────────────────────────────┤
│  1. Static Initializers Run (before main)               │
│     └── Each component registers its schema function     │
│                                                          │
│  2. main() / setup() called                             │
│     └── ConfigRegistry::RegisterAllSchemas()            │
│         └── Executes all registered schema functions     │
│                                                          │
│  3. Components created on-demand                        │
│     └── Load configuration values from registered schema │
└─────────────────────────────────────────────────────────┘
```

### Core Components

#### 1. ConfigRegistry (New)
```cpp
// include/config/config_registry.h
class ConfigRegistry {
    static std::vector<RegistrationFunc>& GetRegistrars();
public:
    static bool RegisterSchema(RegistrationFunc func);
    static void RegisterAllSchemas(IPreferenceService* service);
    static void Reset(); // For testing
};
```

#### 2. Component Pattern
```cpp
class OilPressureSensor {
public:
    // Static method - no instance required
    static void RegisterConfigSchema(IPreferenceService* service);

    // Instance methods - when component is created
    void Init();
    void LoadConfiguration();
};
```

#### 3. Self-Registration Macro
```cpp
#define REGISTER_CONFIG_SCHEMA(ComponentClass) \
    namespace { \
        static bool ComponentClass##_registered = \
            ConfigRegistry::RegisterSchema( \
                [](IPreferenceService* svc) { \
                    ComponentClass::RegisterConfigSchema(svc); \
                } \
            ); \
    }
```

## Implementation Plan

### Phase 1: Create Infrastructure

1. **Create ConfigRegistry class** (`include/config/config_registry.h`)
   - Singleton-like static registry for schema functions
   - Thread-safe initialization using Construct-On-First-Use idiom
   - Methods for registration and batch execution

2. **Update IPreferenceService interface**
   - Add `bool IsSchemaRegistered(const std::string& section)` method
   - Add `void ClearAllSchemas()` for testing

### Phase 2: Refactor Components

For each configurable component:

1. **Split configuration into static and instance parts**

   Before:
   ```cpp
   void OilPressureSensor::RegisterConfig(IPreferenceService* service) {
       // Currently requires instance
   }
   ```

   After:
   ```cpp
   // Static - registers schema only
   static void OilPressureSensor::RegisterConfigSchema(IPreferenceService* service) {
       // No 'this' pointer needed
   }

   // Instance - loads values
   void OilPressureSensor::LoadConfiguration() {
       // Uses 'this' to set member variables
   }
   ```

2. **Add self-registration to each component's .cpp file**
   ```cpp
   REGISTER_CONFIG_SCHEMA(OilPressureSensor)
   ```

### Phase 3: Update Initialization Flow

1. **Update main.cpp**
   ```cpp
   void setup() {
       // Initialize services
       preferenceManager = std::make_unique<PreferenceManager>();

       // Register ALL schemas automatically
       ConfigRegistry::RegisterAllSchemas(preferenceManager.get());

       // Load saved values from NVS
       preferenceManager->LoadAllConfigSections();

       // Continue with normal initialization
       // Components created on-demand later
   }
   ```

2. **Update component initialization**
   - Components call `LoadConfiguration()` in their `Init()` method
   - Configuration values already available from registered schema

### Phase 4: Testing & Validation

1. **Create unit tests**
   - Test registration happens before main
   - Test all configs available without component initialization
   - Test component can load values after registration

2. **Integration testing**
   - Verify Config Panel shows all sections
   - Test changing values for non-initialized components
   - Ensure values persist and load correctly

## Migration Strategy

### Step-by-Step Migration

1. **Start with one component** (e.g., OilPressureSensor)
   - Implement static registration
   - Test thoroughly
   - Validate pattern works

2. **Migrate remaining components**
   - OilTemperatureSensor
   - StyleManager
   - SplashPanel
   - System configuration

3. **Remove old registration calls**
   - Remove `RegisterConfig()` calls from component constructors
   - Clean up any component-specific initialization

### Backward Compatibility

During migration, support both patterns:
```cpp
void Component::Init() {
    if (!ConfigRegistry::IsSchemaRegistered(CONFIG_SECTION)) {
        // Fallback to old pattern
        RegisterConfig(preferenceService_);
    }
    LoadConfiguration();
}
```

## Components to Migrate

1. **System Configuration** (main.cpp)
   - Move `registerSystemConfiguration()` to static registration

2. **Sensors**
   - OilPressureSensor
   - OilTemperatureSensor

3. **Managers**
   - StyleManager

4. **Panels**
   - SplashPanel

5. **Future Components**
   - Any new components automatically follow this pattern

## Benefits

1. **User Experience**
   - Config Panel always shows complete menu
   - Users can configure any component anytime
   - Predictable, consistent interface

2. **Developer Experience**
   - Add new components without touching main.cpp
   - Clear separation: schema vs. values
   - Self-documenting registration

3. **Architecture**
   - True dynamic registration
   - Zero coupling between main and components
   - Components still lazy-loaded

## Potential Issues & Mitigations

### Static Initialization Order Fiasco
**Issue**: Order of static initialization across translation units is undefined
**Mitigation**: Use Construct-On-First-Use idiom for the registry vector

### Linker Optimization
**Issue**: Linker might strip unused components, preventing registration
**Mitigation**:
- Use `--whole-archive` flag for component libraries
- Or create explicit reference file that mentions all components

### Testing Challenges
**Issue**: Static registration happens once per program run
**Mitigation**: Add `ConfigRegistry::Reset()` method for test isolation

### Memory Usage
**Issue**: All schemas loaded even if component never used
**Mitigation**: Schemas are lightweight (just metadata), actual components still lazy-loaded

## Success Criteria

1. ✅ Config Panel shows all sections regardless of loaded components
2. ✅ No changes required to main.cpp when adding new components
3. ✅ Components maintain ownership of their configuration
4. ✅ Existing configuration values preserved during migration
5. ✅ No performance degradation
6. ✅ Clean, maintainable code structure

## Timeline

- **Day 1**: Create ConfigRegistry infrastructure
- **Day 2**: Migrate first component (OilPressureSensor) as proof of concept
- **Day 3**: Migrate remaining components
- **Day 4**: Testing and validation
- **Day 5**: Documentation and cleanup

## Code Examples

### Complete Component Example

```cpp
// oil_pressure_sensor.h
class OilPressureSensor : public BaseSensor {
public:
    // Static schema registration - no instance needed
    static void RegisterConfigSchema(IPreferenceService* service);

    // Instance methods
    void Init() override;
    void LoadConfiguration();

private:
    // Config items remain as inline static
    inline static Config::ConfigItem unitConfig_{...};
    inline static Config::ConfigItem updateRateConfig_{...};

    // Instance variables
    IPreferenceService* preferenceService_;
    std::string targetUnit_;
    int updateIntervalMs_;
};

// oil_pressure_sensor.cpp
#include "sensors/oil_pressure_sensor.h"
#include "config/config_registry.h"

// Self-registration at program start
REGISTER_CONFIG_SCHEMA(OilPressureSensor)

// Static method - registers schema without instance
void OilPressureSensor::RegisterConfigSchema(IPreferenceService* service) {
    Config::ConfigSection section(
        CONFIG_SECTION,
        CONFIG_SECTION,
        "Oil Pressure Sensor"
    );

    section.AddItem(unitConfig_);
    section.AddItem(updateRateConfig_);
    section.AddItem(offsetConfig_);
    section.AddItem(scaleConfig_);

    service->RegisterConfigSection(section);
}

// Instance method - loads values when component created
void OilPressureSensor::LoadConfiguration() {
    if (!preferenceService_) return;

    if (auto unit = preferenceService_->QueryConfig<std::string>(CONFIG_UNIT)) {
        targetUnit_ = *unit;
    }

    if (auto rate = preferenceService_->QueryConfig<int>(CONFIG_UPDATE_RATE)) {
        updateIntervalMs_ = *rate;
    }
}

void OilPressureSensor::Init() {
    // Load configuration values (schema already registered)
    LoadConfiguration();

    // Continue with normal initialization
    RegisterLiveUpdateCallbacks();
}
```

## Next Steps

1. Review and approve this plan
2. Create feature branch: `feature/self-registering-config`
3. Implement ConfigRegistry
4. Migrate one component as proof of concept
5. Complete migration of all components
6. Test thoroughly
7. Merge to main

## Questions to Resolve

1. Should we use a macro or template for registration?
2. How to handle system configuration (currently in main.cpp)?
3. Should we add component versioning to schemas?
4. Do we need a migration path for existing NVS data?

## References

- [Static Initialization Order Fiasco](https://en.cppreference.com/w/cpp/language/siof)
- [Construct On First Use Idiom](https://en.wikibooks.org/wiki/More_C%2B%2B_Idioms/Construct_On_First_Use)
- [Self-Registering Classes Pattern](https://www.bfilipek.com/2018/02/factory-selfregister.html)
- [Google Test Registration Pattern](https://github.com/google/googletest)