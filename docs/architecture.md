# Architecture

## Overview

MVP (Model-View-Presenter) pattern for ESP32 automotive gauges with layered architecture, interface-based design, and handler-based interrupt system using function pointers.

## Visual Diagrams

For detailed architectural diagrams, see:
- **[Architecture Overview](diagrams/architecture-overview.md)** - Complete component relationships and dependencies
- **[Application Flow](diagrams/application-flow.md)** - Startup sequence, runtime processing, and interrupt handling flows

## Pattern Structure

```
InterruptManager → TriggerHandler → Sensors → GPIO
                ↘ ActionHandler  ↗          
                                    
DeviceProvider → Display → Panels → Components
                            ↓ ↑
                         (Display Only)
```

### MVP Mapping
- **Models**: Sensors - hardware data acquisition with change detection
- **Views**: Components - UI rendering (display-only for trigger panels)
- **Presenters**: Panels - orchestration and business logic

## Core Layers

### DeviceProvider
Hardware abstraction for display (GC9A01) and LVGL integration.

### Panels (Presenters)
- Coordinate components for display
- Handle lifecycle: init → load → update
- Trigger panels (Key, Lock) are display-only
- Data panels (Oil) create their own data sensors
- Implement IActionService for button handling

### Components (Views)  
- Render UI elements (gauges, indicators)
- No business logic
- LVGL object management

### Sensors (Models)
- Abstract GPIO/ADC inputs with change detection
- Single ownership model - each GPIO has exactly one sensor
- Split architecture for independent concerns (e.g., KeyPresentSensor, KeyNotPresentSensor)
- Implement proper destructors with GPIO cleanup

## Coordinated Interrupt System Architecture

### Specialized Handler Design
The interrupt system uses InterruptManager to coordinate two specialized handlers that process different interrupt sources:

```
InterruptManager: Central coordination of interrupt processing
├── PolledHandler: Manages POLLED interrupts (GPIO state monitoring)
└── QueuedHandler: Manages QUEUED interrupts (button event processing)
```

### Common Interrupt Data Structure

#### Memory-Optimized Design
Both handlers use a common interrupt structure with static function pointers to prevent heap fragmentation on ESP32:

```cpp
enum class InterruptSource {
    POLLED,     // GPIO state monitoring (managed by PolledHandler)
    QUEUED      // Button event processing (managed by QueuedHandler)
};

enum class InterruptEffect {
    LOAD_PANEL,        // Panel switching with restoration tracking
    SET_THEME,         // Theme changes (non-blocking for restoration)
    SET_PREFERENCE,    // Configuration changes
    CUSTOM_FUNCTION    // Panel-specific custom functions
};

struct Interrupt {
    const char* id;                                    // Static string for memory efficiency
    Priority priority;                                 // Processing priority  
    InterruptSource source;                           // POLLED or QUEUED evaluation
    InterruptEffect effect;                           // What this interrupt does
    bool (*evaluationFunc)(void* context);           // Function pointer - no heap allocation
    void (*executionFunc)(void* context);            // Function pointer - no heap allocation
    void* context;                                    // Sensor or service context
    
    // Effect-specific data (union for memory efficiency)
    union {
        struct { const char* panelName; bool trackForRestore; } panel;     // LOAD_PANEL data
        struct { Theme theme; } theme;                                     // SET_THEME data
        struct { const char* key; void* value; } preference;               // SET_PREFERENCE data
        struct { void (*customFunc)(void* ctx); } custom;                  // CUSTOM_FUNCTION data
    } data;
    
    // Runtime state
    bool active;                                      // Current activation state
    unsigned long lastEvaluation;                    // Performance tracking
};
```

#### Critical Memory Constraint
**ESP32 Memory Limitation**: The ESP32-WROOM-32 has only ~300KB available RAM. Using `std::function` with lambda captures causes:
- Heap fragmentation leading to crashes
- Memory overhead of ~3KB per std::function object
- Unstable system behavior during LVGL operations

**Solution**: Static callback pattern eliminates heap allocations and provides predictable memory usage.

### Coordinated Interrupt Processing System
The coordinated system processes interrupts through specialized handlers with central coordination:
- **PolledHandler**: Manages GPIO state changes with sensors tracking previous state internally
- **QueuedHandler**: Processes button events from message queue
- **InterruptManager**: Coordinates both handlers, handles effect-based execution and restoration logic
- **Priority-based coordination**: Highest priority interrupt across both handlers gets processed

#### Coordinated Processing Implementation
**Specialized Architecture**: InterruptManager coordinates PolledHandler and QueuedHandler:

1. **Handler Processing**: Each handler evaluates its interrupts and marks active ones
2. **Priority Coordination**: InterruptManager compares highest priority interrupt from each handler
3. **Effect-Based Execution**: InterruptManager executes based on interrupt effect (LOAD_PANEL, SET_THEME, etc.)
4. **Simplified Restoration**: Only panel-loading effects participate in restoration logic

**Coordination Rule**: Each handler processes its interrupts, InterruptManager coordinates execution.

#### Change Detection for POLLED Interrupts
**PolledHandler Processing**: GPIO state monitoring maintains change detection patterns:
- Sensors still track previous state internally for change detection
- `evaluationFunc` calls sensor's `HasStateChanged()` method exactly once per cycle
- Context parameter provides sensor access without heap allocation
- Same change detection corruption prevention as before

### Sensor Architecture

#### Split Sensor Design (Critical Requirement)
Each GPIO pin must have exactly one dedicated sensor class:
- **KeyPresentSensor** (GPIO 25): Independent class for key present detection
- **KeyNotPresentSensor** (GPIO 26): Independent class for key not present detection
- **LockSensor** (GPIO 27): Lock status monitoring
- **LightsSensor** (GPIO 33): Day/night detection (theme only)
- **ActionButtonSensor** (GPIO 32): Button input with debouncing

#### Sensor Independence Requirements
**Architectural Principle**: KeyPresentSensor and KeyNotPresentSensor must be completely separate classes with:
- Independent initialization flags
- Separate change detection state
- No shared state or initialization conflicts
- Individual GPIO resource management

**Critical**: Using a single KeySensor class for both GPIO pins causes initialization race conditions and change detection failures.

#### Resource Management Requirements
**GPIO Cleanup**: Sensors that attach GPIO interrupts must implement destructors with:
```cpp
~KeyPresentSensor() {
    if (gpioProvider_) {
        gpioProvider_->DetachInterrupt(gpio_pins::KEY_PRESENT);
        log_d("KeyPresentSensor destructor - GPIO interrupt detached");
    }
}
```

**Single Ownership Model**: Each GPIO has exactly one sensor instance to prevent resource conflicts:
- TriggerHandler owns all trigger-related sensors
- Panels are display-only and must not create sensors
- No sensor duplication across components

## Managers

- **InterruptManager**: Creates and coordinates PolledHandler and QueuedHandler during LVGL idle time
- **PanelManager**: Creates panels on demand, manages switching, lifecycle, and restoration tracking
- **PreferenceManager**: Persistent settings
- **StyleManager**: LVGL themes (Day/Night)
- **ErrorManager**: Error collection with coordinated interrupt integration

## Coordinated Interrupt Processing Flow

### Multi-Handler Processing
1. **InterruptManager::Process()** (coordination processing)
   - Call PolledHandler::Process() to evaluate GPIO state changes
   - Call QueuedHandler::Process() to evaluate latest button event
   - Compare highest priority active interrupt from each handler
   - Execute highest priority interrupt across both handlers
   - Handle effect-based execution (LOAD_PANEL, SET_THEME, SET_PREFERENCE, CUSTOM_FUNCTION)
   - Process simplified restoration logic

2. **PolledHandler::Process()** (GPIO monitoring)
   - Evaluate POLLED interrupts for GPIO state changes via `evaluationFunc(context)`
   - Mark active interrupts for InterruptManager coordination

3. **QueuedHandler::Process()** (button events)
   - Evaluate QUEUED interrupts for **single latest button event** (not a queue)
   - **Latest Event Only**: Maintains only the most recent button event, discarding any previous unprocessed events
   - Mark active interrupts for InterruptManager coordination
   - Clear button event after processing

### Priority System
- **CRITICAL (0)**: Errors, key security
- **IMPORTANT (1)**: Lock status  
- **NORMAL (2)**: Theme changes, button actions, preferences

### Simplified Panel Restoration
- **Effect-Based Logic**: Only LOAD_PANEL effects participate in restoration
- **Cross-Handler Logic**: InterruptManager checks both handlers for active panel-loading interrupts
- **Theme Independence**: SET_THEME effects never affect panel restoration
- **Reduced Complexity**: No priority-based blocking, just effect-based logic

## Available Panels

- **Splash**: Startup animation with skip capability
- **Oil**: Dual gauge monitoring (pressure/temperature) - creates own data sensors
- **Key**: Key presence/absence indicator - display only, no sensors
- **Lock**: Security status display - display only, no sensors
- **Error**: Scrollable error list with acknowledgment - triggered by ErrorManager
- **Config**: System configuration and preferences

## Hardware Configuration

### Display
- GC9A01 240x240 round
- SPI2_HOST interface
- 57.6KB dual buffer (240×60×2×2 bytes)

### Inputs
- **Digital Sensors** (GPIO with change detection):
  - KeyPresentSensor (GPIO 25) - Key present detection
  - KeyNotPresentSensor (GPIO 26) - Key not present detection
  - LockSensor (GPIO 27) - Vehicle lock status
  - LightsSensor (GPIO 33) - Day/night detection
  - ActionButtonSensor (GPIO 32) - User input button
- **Analog Sensors** (ADC):
  - Oil pressure sensor - Continuous pressure monitoring
  - Oil temperature sensor - Continuous temperature monitoring

## Key Interfaces

### Factory Interfaces
- `IProviderFactory`: Factory interface for creating hardware providers with dependency injection support
- `IManagerFactory`: Factory interface for creating managers (future extension point)

### Provider Interfaces (Hardware Abstraction)
- `IGpioProvider`: Hardware GPIO abstraction for digital/analog I/O
- `IDisplayProvider`: LVGL display abstraction for screen operations
- `IDeviceProvider`: Hardware device driver abstraction for low-level device communication

### Component Interfaces (MVP Pattern)
- `IPanel`: Screen implementation interface with universal button functions
  - `GetShortPressFunction()`: Returns panel's short press static function pointer
  - `GetLongPressFunction()`: Returns panel's long press static function pointer
  - `GetPanelContext()`: Returns panel instance for function context
  - `Init/Load/Update`: Standard panel lifecycle methods
- `IComponent`: UI element rendering interface for LVGL components
- `ISensor`: Data acquisition interface with change detection support

### Service Interfaces
- `IHandler`: Unified interface for interrupt processing (triggers and actions)
- `IActionService`: Panel-specific action handling interface
- `IManager`: Base manager interface for service lifecycle management

### Utility Interfaces
- `ILogger`: Logging abstraction interface (future)
- `IPreferenceService`: Configuration persistence interface (future)

## Coordinated Interrupt Registration

### POLLED Panel Loading Registration (replaces triggers)
```cpp
// Routed to PolledHandler by InterruptManager
interruptManager.RegisterInterrupt({
    .id = "key_present",
    .priority = Priority::CRITICAL,
    .source = InterruptSource::POLLED,
    .effect = InterruptEffect::LOAD_PANEL,
    .evaluationFunc = KeyPresentChanged,
    .executionFunc = LoadKeyPanel,
    .context = keyPresentSensor,
    .data = { .panel = { "KEY", true } }  // trackForRestore = true
});
```

### POLLED Theme Setting Registration
```cpp
// Routed to PolledHandler by InterruptManager
interruptManager.RegisterInterrupt({
    .id = "lights_changed", 
    .priority = Priority::NORMAL,
    .source = InterruptSource::POLLED,
    .effect = InterruptEffect::SET_THEME,
    .evaluationFunc = LightsChanged,
    .executionFunc = SetThemeBasedOnLights,
    .context = lightsSensor,
    .data = { .theme = Theme::NIGHT }  // Will be determined dynamically
});
```

### QUEUED Universal Panel Button Registration (every panel has these)
```cpp
// Universal short press - routed to QueuedHandler by InterruptManager
interruptManager.RegisterInterrupt({
    .id = "universal_short_press",
    .priority = Priority::NORMAL,
    .source = InterruptSource::QUEUED,
    .effect = InterruptEffect::BUTTON_ACTION,
    .evaluationFunc = HasShortPressEvent,
    .activateFunc = ExecutePanelShortPress,      // Calls current panel's short press function
    .deactivateFunc = nullptr,
    .sensorContext = actionButtonSensor,
    .serviceContext = panelManager,              // Used to get current panel
    .data = { .buttonActions = { nullptr, nullptr, nullptr } }  // Functions injected at runtime
});

// Universal long press - routed to QueuedHandler by InterruptManager
interruptManager.RegisterInterrupt({
    .id = "universal_long_press",
    .priority = Priority::NORMAL,
    .source = InterruptSource::QUEUED,
    .effect = InterruptEffect::BUTTON_ACTION,
    .evaluationFunc = HasLongPressEvent,
    .activateFunc = ExecutePanelLongPress,       // Calls current panel's long press function
    .deactivateFunc = nullptr,
    .sensorContext = actionButtonSensor,
    .serviceContext = panelManager,
    .data = { .buttonActions = { nullptr, nullptr, nullptr } }  // Functions injected at runtime
});
```

## Memory Architecture

### Sensor Ownership Model
```
 Main
├── Creates ProviderFactory
│   ├── GpioProvider
│   ├── DisplayProvider
│   └── DeviceProvider
└── Creates ManagerFactory (with IProviderFactory injected)
    ├── Creates InterruptManager
    │   ├── Creates PolledHandler (owns GPIO sensors)
    │   │   ├── KeyPresentSensor (GPIO 25)
    │   │   ├── KeyNotPresentSensor (GPIO 26)
    │   │   ├── LockSensor (GPIO 27)
    │   │   └── LightsSensor (GPIO 33)
    │   └── Creates QueuedHandler (owns button sensor)
    │       └── ActionButtonSensor (GPIO 32)
    └── Creates PanelManager
        └── Creates panels on demand (with providers injected)
            ├── KeyPanel (display-only - creates components, no sensors)
            ├── LockPanel (display-only - creates components, no sensors)
            └── OilPanel (creates own data sensors and components)
```

### Resource Management
- Single sensor instance per GPIO pin created by specialized handler (PolledHandler owns GPIO sensors, QueuedHandler owns button sensor)
- Proper destructors with DetachInterrupt() calls
- No sensor duplication across components
- Clear ownership boundaries with coordinated handler architecture

## History

### Evolution of Interrupt Architecture

#### Hardware Interrupts (Abandoned)
Initially attempted hardware GPIO interrupts with message queues on separate cores. This approach failed due to:
- Complex state sharing between cores
- Race conditions with LVGL operations
- Non-stop crashes when interrupting animations
- Excessive complexity for minimal benefit

#### Polling-Based System (Current)
Switched to checking GPIO states during LVGL idle time:
- All interrupt processing occurs in main thread
- No concurrent access issues
- Minimal delay is acceptable for user interactions
- Significantly more reliable architecture

#### Coordinated Interrupt Architecture (Final)
Evolution to specialized handler system with central coordination:
- InterruptManager coordinates PolledHandler and QueuedHandler
- Source-specific evaluation (POLLED vs QUEUED) with effect-based execution
- Simplified restoration logic based on interrupt effects
- Eliminated redundant UnifiedInterruptHandler layer for direct coordination

### Key Architectural Decisions

1. **Coordinated Interrupt Processing**: InterruptManager coordinates specialized PolledHandler and QueuedHandler, eliminating redundant handler layers

2. **Effect-Based Execution**: Interrupts categorized by effect (LOAD_PANEL, SET_THEME, etc.) enabling simplified restoration logic

3. **Simplified Restoration**: Only panel-loading effects participate in restoration, eliminating complex priority-based blocking

4. **Split Sensor Design**: Independent sensor classes for each GPIO prevent initialization conflicts (e.g., KeyPresentSensor vs KeyNotPresentSensor)  

5. **Specialized Ownership**: PolledHandler owns GPIO sensors, QueuedHandler owns button sensor, preventing resource conflicts

6. **Display-Only Panels**: Trigger panels don't create sensors, eliminating duplication and simplifying architecture

## Service Architecture

Dependency injection via dual factory pattern enables loose coupling and testability.

### Factory Pattern Design

The system uses two specialized factories with interface-based dependency injection:

#### ProviderFactory (implements IProviderFactory)
- Creates all hardware abstraction providers (GpioProvider, DisplayProvider, DeviceProvider)
- Implements IProviderFactory interface for testability
- Enables mock provider injection in tests
- Single responsibility: hardware provider creation

**Interface Definition**:
```cpp
class IProviderFactory {
public:
    virtual ~IProviderFactory() = default;
    virtual std::unique_ptr<IGpioProvider> CreateGpioProvider() = 0;
    virtual std::unique_ptr<IDisplayProvider> CreateDisplayProvider() = 0;
    virtual std::unique_ptr<IDeviceProvider> CreateDeviceProvider() = 0;
};
```

#### ManagerFactory (future: implements IManagerFactory)
- Creates all system managers (Interrupt, Panel, Style, Preference, Error)
- Receives IProviderFactory instance for dependency injection
- Passes providers to managers as needed
- Single responsibility: manager creation and wiring

**Generic Factory Pattern**:
```cpp
template<typename T>
class IFactory {
public:
    virtual ~IFactory() = default;
    virtual std::unique_ptr<T> Create() = 0;
};
```

#### Testability Benefits
```cpp
// Production
auto providerFactory = std::make_unique<ProviderFactory>();
auto managerFactory = std::make_unique<ManagerFactory>(providerFactory.get());

// Testing with dependency injection
auto mockProviderFactory = std::make_unique<MockProviderFactory>();
auto managerFactory = std::make_unique<ManagerFactory>(mockProviderFactory.get());

// Generic factory usage
auto panelFactory = std::make_unique<PanelFactory<IPanel>>();
auto oilPanel = panelFactory->Create();
```

## Memory Management Architecture

**Critical Constraint**: ESP32-WROOM-32 has only ~300KB available RAM, making memory management a first-class architectural concern.

### Memory Optimization Requirements

#### 1. Static Callback System (Critical)
**Problem**: `std::function` with lambda captures causes heap fragmentation and system crashes.
**Solution**: All interrupt callbacks must use static function pointers:

```cpp
// Required pattern for all interrupt callbacks
struct InterruptCallbacks {
    static bool KeyPresentChanged(void* context) {
        auto* sensor = static_cast<KeyPresentSensor*>(context);
        return sensor && sensor->HasStateChanged();
    }
    
    static bool KeyPresentState(void* context) {
        auto* sensor = static_cast<KeyPresentSensor*>(context);
        return sensor && sensor->GetKeyPresentState();
    }
};
```

#### 2. LVGL Buffer Optimization
**Memory Usage**: Dual 60-line buffers consume 57.6KB (240×60×2×2 bytes)
**Optimization Options**:
- Single buffer mode: Saves 30KB
- Reduced line count (30 lines): Additional 15KB savings
- Dynamic allocation based on panel complexity

#### 3. Pointer Management Strategy
**Smart Pointer Overhead**: 16-24 bytes per std::shared_ptr instance
**Required Pattern**:
- main.cpp owns all sensors via unique_ptr
- Panels receive raw pointers (non-owning)
- Managers receive raw pointers (non-owning)
- Factory methods return unique_ptr (transfer ownership)

#### 4. String Optimization
**Fixed-Size Strings**: Replace std::string with template-based fixed arrays:
```cpp
template<size_t N>
struct FixedString {
    char data[N];
    // Implementation eliminates heap allocations
};
using InterruptId = FixedString<32>;
using PanelName = FixedString<16>;
```

### Heap Corruption Prevention
**Critical Issue**: `std::function` usage in interrupt system causes heap fragmentation leading to crashes during LVGL operations.
**Prevention Measures**:
1. Use static function pointers exclusively
2. Minimize dynamic allocations during runtime
3. Implement object pools for frequently created/destroyed objects
4. Use stack allocation over heap allocation where possible

### Memory Design Principles
- **Predictable Allocation**: All major allocations occur during initialization
- **Clear Ownership**: Avoid shared_ptr overhead with explicit ownership rules
- **Fragmentation Prevention**: Static callbacks eliminate fragmentation sources
- **ESP32 Optimized**: Architecture designed specifically for ESP32 constraints

## Implementation Constraints

### ESP32-Specific Requirements
1. **Memory Constraints**: All design decisions must account for 300KB RAM limit
2. **Single-Core Operation**: LVGL operations must not be interrupted by GPIO processing
3. **Heap Management**: Minimize dynamic allocations to prevent fragmentation
4. **Resource Cleanup**: Proper GPIO interrupt detachment in destructors

### Sensor Duplication Prevention
**Critical Rule**: Each GPIO pin must have exactly one sensor instance
**Enforcement**:
- Handlers create and own their sensors internally
- Data panels create their own data sensors
- Trigger panels are display-only (no sensor creation)
- Build-time verification of single ownership

### Change Detection Implementation
**BaseSensor Pattern**: All sensors must inherit from BaseSensor for consistent change detection:
```cpp
class BaseSensor {
protected:
    bool initialized_ = false;
    
    template<typename T>
    bool DetectChange(T currentValue, T& previousValue) {
        if (!initialized_) {
            previousValue = currentValue;
            initialized_ = true;
            return false; // No change on first read
        }
        
        bool changed = (currentValue != previousValue);
        previousValue = currentValue;
        return changed;
    }
};
```

### Error Prevention Requirements
1. **Initialization Race Conditions**: Split sensors prevent shared state conflicts
2. **Change Detection Corruption**: Separate change detection from state retrieval
3. **Resource Conflicts**: Single sensor ownership per GPIO
4. **Memory Leaks**: Proper destructor implementation for interrupt cleanup

## Implemented Architecture Enhancements

### Coordinated Interrupt System
The system has been designed with a coordinated interrupt architecture that uses specialized handlers with central coordination. This provides:

- **Specialized Processing**: PolledHandler for GPIO monitoring, QueuedHandler for button events
- **Central Coordination**: InterruptManager coordinates both handlers and manages execution
- **Effect-Based Execution**: LOAD_PANEL, SET_THEME, SET_PREFERENCE, CUSTOM_FUNCTION effects
- **Simplified Restoration**: Effect-based logic eliminates complex priority blocking
- **Eliminated Redundancy**: Direct coordination removes unnecessary UnifiedInterruptHandler layer

This coordinated approach is documented in detail in the plans/interrupt-consolidation-plan.md.

## Testing Limitations

The ESP32 development environment has several testing constraints that affect architecture decisions:

### PlatformIO Unity Framework Limitations
- **Build Filter Issues**: Build filters are not applied to Unity tests, causing all test files to link together
- **Directory Structure**: Test files must be in root test directory; nested directories are not discovered
- **Memory Constraints**: ESP32's limited RAM affects test complexity and mocking capabilities
- **Hardware Dependencies**: Many components require actual GPIO/SPI hardware for meaningful testing

### Wokwi Emulator Constraints
- **Display Limitations**: Emulator only supports square displays, not the target round GC9A01
- **Image Inversion**: Display renders horizontally inverted compared to actual hardware
- **Performance Differences**: Timing behaviors may differ from real ESP32 hardware

These limitations influence the architecture toward:
- Interface-based design for easier mocking
- Separation of hardware-dependent and business logic layers  
- Comprehensive logging for production debugging
- Simple, focused test cases that work within framework constraints

## Coding Standards

The project follows a consistent coding format based on Google C++ Style Guide with project-specific preferences:

### Naming Conventions
- **Classes**: `PascalCase` (e.g., `PanelManager`, `StyleManager`)
- **Functions/Methods**: `PascalCase` for public methods (e.g., `SetTheme()`, `ApplyThemeToScreen()`)
- **Variables**: `snake_case` with trailing underscore for members (e.g., `panel_manager_`, `gpio_provider_`)
- **Constants**: `ALL_CAPS` (e.g., `NIGHT`, `DAY`, `OIL`) - project preference over kCamelCase
- **Files**: `snake_case.h/.cpp` (e.g., `panel_manager.h`, `style_manager.cpp`)
- **Interfaces**: Prefixed with `I` (e.g., `IComponent`, `IPanel`, `ISensor`)

### Code Organization
- **Headers**: Comprehensive Doxygen-style documentation blocks
- **Includes**: System includes before project includes
- **Namespaces**: `snake_case` (e.g., `gpio_pins`)
- **Enums**: `PascalCase` with descriptive values