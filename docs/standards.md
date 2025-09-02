# Clarity Project Naming Standards

This document defines the naming conventions used throughout the Clarity automotive gauge system project. These standards are based on the Google C++ Style Guide with one modification for constant naming.

## Overview

Consistent naming conventions improve code readability, maintainability, and team collaboration. All contributors must follow these standards when writing code for the Clarity project.

## File Naming

**Rule**: Use lowercase with underscores to separate words.

**Format**: `lowercase_with_underscores.extension`

**Examples**:
```cpp
// Headers
oil_temperature_sensor.h
trigger_manager.h
panel_manager.h

// Implementation files  
oil_temperature_sensor.cpp
trigger_manager.cpp
panel_manager.cpp
```

**Rationale**: Consistent with Unix/Linux conventions and avoids case sensitivity issues across different filesystems.

## Class Naming

**Rule**: Use PascalCase (CapitalCase) for all class names.

**Format**: `PascalCase`

**Examples**:
```cpp
class OilTemperatureSensor;
class TriggerManager;
class ComponentLocation;
class ErrorManager;
```

**Rationale**: Clear distinction between types and variables, consistent with C++ standard library conventions.

## Function and Method Naming

**Rule**: Use PascalCase for all function and method names.

**Format**: `PascalCase()`

**Examples**:
```cpp
void Init();
Reading GetReading();
void SetTargetUnit(const std::string& unit);
bool IsValidAdcReading(int32_t value);
std::vector<std::string> GetSupportedUnits() const;
```

**Rationale**: Makes functions easily distinguishable from variables and follows established C++ conventions.

## Variable Naming

### Local Variables and Parameters

**Rule**: Use lowercase with underscores to separate words.

**Format**: `lowercase_with_underscores`

**Examples**:
```cpp
int sensor_value;
std::string panel_name;
bool lights_on;
unsigned long update_time;
IGpioProvider* gpio_provider;
```

### Member Variables

**Rule**: Use lowercase with underscores, ending with a trailing underscore.

**Format**: `lowercase_with_underscores_`

**Examples**:
```cpp
class OilTemperatureSensor {
private:
    IGpioProvider* gpio_provider_;
    std::string target_unit_;
    int32_t current_reading_;
    unsigned long last_update_time_;
    unsigned long update_interval_ms_;
};
```

**Rationale**: The trailing underscore clearly identifies member variables and prevents naming conflicts with parameters and local variables.

## Constant Naming

**Rule**: Use ALL_CAPS with underscores to separate words.

**Format**: `ALL_CAPS_WITH_UNDERSCORES`

**Examples**:
```cpp
// Static constexpr constants
static constexpr const char* NIGHT = "Night";
static constexpr const char* DAY = "Day";
static constexpr const char* SPLASH_PANEL = "SplashPanel";
static constexpr int32_t ADC_MAX_VALUE = 4095;
static constexpr float SUPPLY_VOLTAGE = 3.3f;

// Const variables
const int MAX_BUFFER_SIZE = 1024;
const std::string DEFAULT_THEME = "Day";
```

**Project Deviation**: This project uses ALL_CAPS for constants instead of the Google C++ Style Guide's `k` prefix convention (e.g., `kNight`). This provides clear visual distinction and follows traditional C/C++ constant naming.

## Enum Naming

### Enum Classes

**Rule**: Use PascalCase for enum class names.

**Format**: `PascalCase`

**Examples**:
```cpp
enum class LogLevels {
    Verbose,
    Debug,
    Info,
    Warning,
    Error
};

enum class KeyState {
    Inactive,
    Present,
    NotPresent
};
```

### Enum Values

**Rule**: Use PascalCase for enum values.

**Format**: `PascalCase`

**Examples**:
```cpp
enum class TriggerActionType {
    LoadPanel,
    ToggleTheme
};

enum class ErrorLevel {
    WARNING,    // Exception: Legacy constants may use ALL_CAPS
    ERROR,      // Exception: Legacy constants may use ALL_CAPS  
    CRITICAL    // Exception: Legacy constants may use ALL_CAPS
};
```

## Namespace Naming

**Rule**: Use lowercase with underscores for namespace names.

**Format**: `lowercase_with_underscores`

**Examples**:
```cpp
namespace gpio_pins {
    constexpr int OIL_TEMPERATURE = 34;
    constexpr int OIL_PRESSURE = 35;
}

namespace clarity_ui {
    // UI-specific functionality
}
```

## Macro Naming

**Rule**: Use ALL_CAPS with underscores (avoid macros when possible).

**Format**: `ALL_CAPS_WITH_UNDERSCORES`

**Examples**:
```cpp
#define CLARITY_DEBUG 1
#define MAX_RETRY_COUNT 3
```

**Note**: Prefer constexpr constants over macros whenever possible.

## Structure and Union Naming

**Rule**: Use PascalCase, same as classes.

**Format**: `PascalCase`

**Examples**:
```cpp
struct ComponentLocation {
    lv_coord_t x;
    lv_coord_t y;
    lv_align_t align;
};

struct ThemeColors {
    lv_color_t background;
    lv_color_t text;
    lv_color_t primary;
};
```

## Type Alias Naming

**Rule**: Use PascalCase for type aliases.

**Format**: `PascalCase`

**Examples**:
```cpp
using Reading = std::variant<std::monostate, int32_t, double, std::string, bool>;
using GpioProviderPtr = std::shared_ptr<IGpioProvider>;
using ConfigCallback = std::function<void(const Configs&)>;
```

## Interface Naming

**Rule**: Use PascalCase with `I` prefix for interfaces.

**Format**: `IPascalCase`

**Examples**:
```cpp
class ISensor;
class IPanel;
class IGpioProvider;
class IDisplayProvider;
```

**Rationale**: The `I` prefix clearly identifies interfaces and abstract base classes.

## Boolean Variable and Function Naming

**Rule**: Use descriptive names that read like questions.

**Examples**:
```cpp
// Variables
bool is_initialized;
bool has_error;
bool lights_on;

// Functions
bool IsReady();
bool HasChanged();
bool ShouldUpdate();
```

## Naming Best Practices

### General Guidelines

1. **Be Descriptive**: Names should clearly indicate purpose and avoid ambiguity
   ```cpp
   // Good - Clear purpose and context
   int oil_temperature_celsius;
   bool sensor_initialization_complete;
   TriggerManager trigger_manager;
   std::string configuration_file_path;
   
   // Bad - Unclear or overly abbreviated
   int temp;
   bool flag;
   std::string panel_name_string;  // Redundant type info
   int sensor_count_integer;       // Redundant type info
   ```

2. **Use Consistent Terminology**: Maintain the same terms throughout the codebase
   ```cpp
   // Consistent sensor naming
   OilTemperatureSensor, OilPressureSensor, LightsSensor
   
   // Consistent manager naming  
   TriggerManager, PanelManager, StyleManager
   
   // Acceptable abbreviations (widely understood)
   int adc_value;  // ADC is universally recognized
   lv_obj_t* ui_object;  // LVGL library convention
   ```

### Context-Specific Guidelines

#### Architecture-Based Naming Patterns
```cpp
// Sensor classes - indicate measured quantity
class OilTemperatureSensor;
class OilPressureSensor;
int temperature_celsius;
int pressure_bar;

// Manager classes - coordination and lifecycle
class TriggerManager;
class PanelManager;
class StyleManager;

// Component classes - UI element rendering
class OilTemperatureComponent;
class ClarityComponent;
class ErrorListComponent;

// Factory classes - object creation
class UIFactory;
class ManagerFactory;
class ProviderFactory;
```

## Documentation Standards

**Rule**: All public APIs must have Doxygen-style comments following naming conventions.

**Examples**:
```cpp
/**
 * @class OilTemperatureSensor
 * @brief Oil temperature monitoring sensor with unit-aware conversions
 */
class OilTemperatureSensor {
public:
    /**
     * @brief Set the target unit for temperature readings
     * @param unit Target unit ("C" for Celsius, "F" for Fahrenheit)
     */
    void SetTargetUnit(const std::string& unit);
    
    /**
     * @brief Get the current temperature reading
     * @return Temperature value in the configured unit
     */
    Reading GetReading();
};
```

## Tools and Enforcement

### Comprehensive Code Review Checklist
**File and Type Naming:**
- [ ] File names use lowercase_with_underscores
- [ ] Class names use PascalCase  
- [ ] Interface names use PascalCase with `I` prefix
- [ ] Enum classes and values use PascalCase
- [ ] Struct names use PascalCase

**Function and Variable Naming:**
- [ ] Function names use PascalCase
- [ ] Local variables use lowercase_with_underscores
- [ ] Member variables end with trailing underscore
- [ ] Constants use ALL_CAPS
- [ ] Boolean names read like questions (is_ready, has_error)

**Architecture Compliance:**
- [ ] Names are descriptive and unambiguous
- [ ] Consistent terminology used throughout codebase
- [ ] Architecture-specific suffixes applied correctly
- [ ] No redundant type information in names

## Examples from Codebase

### Complete Class Example
```cpp
// File: oil_temperature_sensor.h
class OilTemperatureSensor : public ISensor {
public:
    OilTemperatureSensor(IGpioProvider* gpio_provider, int update_rate_ms = 500);
    
    void Init() override;
    void SetTargetUnit(const std::string& unit) override;
    Reading GetReading() override;
    std::vector<std::string> GetSupportedUnits() const override;
    
private:
    static constexpr int32_t TEMPERATURE_MAX_CELSIUS = 120;
    static constexpr int32_t TEMPERATURE_MIN_FAHRENHEIT = 32;
    
    IGpioProvider* gpio_provider_;
    std::string target_unit_;
    int32_t current_reading_;
    unsigned long last_update_time_;
    unsigned long update_interval_ms_;
};
```

### Constants Structure Example
```cpp
struct PanelNames {
    static constexpr const char* SPLASH = "SplashPanel";
    static constexpr const char* OIL = "OemOilPanel";
    static constexpr const char* KEY = "KeyPanel";
    static constexpr const char* LOCK = "LockPanel";
    static constexpr const char* ERROR = "ErrorPanel";
    static constexpr const char* CONFIG = "ConfigPanel";
};
```

## Conclusion

Following these naming standards ensures code consistency, improves readability, and facilitates maintenance and collaboration. When in doubt, prioritize clarity and consistency with existing code patterns.

For questions or suggestions regarding these standards, please discuss in code reviews or team meetings.