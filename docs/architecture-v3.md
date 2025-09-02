# Architecture (Current Implementation v3.0)

## Overview

MVP (Model-View-Presenter) pattern for ESP32 automotive gauges with layered architecture, interface-based design, and coordinated interrupt system using PolledHandler/QueuedHandler architecture.

## Current Implementation Status

**IMPORTANT**: This document describes the **current v3.0 implementation**. For proposed future enhancements, see `docs/plans/proposed-interrupt-architecture.md`.

## Visual Diagrams

For detailed architectural diagrams, see:
- **[Architecture Overview](diagrams/architecture-overview.md)** - Complete component relationships and dependencies
- **[Application Flow](diagrams/application-flow.md)** - Startup sequence, runtime processing, and interrupt handling flows

## Pattern Structure

```
InterruptManager → PolledHandler → GPIO Sensors → GPIO
                ↘ QueuedHandler → Button Sensor ↗          
                                    
DeviceProvider → Display → Panels → Components
                            ↓ ↑
                         (Display Only)
```

### MVP Mapping
- **Models**: Sensors - hardware data acquisition with change detection
- **Views**: Components - UI rendering (display-only for interrupt-driven panels)
- **Presenters**: Panels - orchestration and business logic

## Core Layers

### DeviceProvider
Hardware abstraction for display (GC9A01) and LVGL integration.

### Panels (Presenters)
- Coordinate components for display
- Handle lifecycle: init → load → update
- Display-only panels (Key, Lock) create components only
- Data panels (Oil) create their own data sensors
- Implement IActionService for action handling

### Components (Views)  
- Render UI elements (gauges, indicators)
- No business logic
- LVGL object management

### Sensors (Models)
- Abstract GPIO/ADC inputs with change detection
- Single ownership model - each GPIO has exactly one sensor
- Split architecture for independent concerns (KeyPresentSensor, KeyNotPresentSensor)
- Implement proper destructors with GPIO cleanup

## Coordinated Interrupt Architecture (v3.0)

### Handler-Based Design
The interrupt system uses InterruptManager to coordinate specialized handlers:

```
InterruptManager: Central coordination of interrupt processing
├── PolledHandler: Manages GPIO state monitoring (Key, Lock, Lights sensors)
└── QueuedHandler: Manages button event processing (ButtonSensor)
```

### Interrupt Processing Model (v3.0)
The system coordinates interrupt evaluation and execution through handlers:

- **Polled Evaluation**: PolledHandler checks GPIO sensors for state changes during UI IDLE
- **Queued Evaluation**: QueuedHandler monitors button sensor continuously for responsiveness  
- **All Execution**: Only happens during UI IDLE state for LVGL compatibility
- **Priority System**: CRITICAL > IMPORTANT > NORMAL with basic blocking logic

### Current Interrupt Structure (v3.0)

#### Simplified Interrupt Structure
The current implementation uses a unified interrupt structure with single execution function:

```cpp
// Located in include/utilities/types.h
enum class Priority : uint8_t {
    NORMAL = 0,      // Lowest priority (e.g., lights)
    IMPORTANT = 1,   // Medium priority (e.g., lock)
    CRITICAL = 2     // Highest priority (e.g., key, errors)
};

// Current v3.0 Interrupt Structure
struct Interrupt {
    const char* id;                           // Static string identifier
    Priority priority;                        // Processing priority enum
    InterruptSource source;                   // POLLED or QUEUED
    
    // Single execution function - simplified approach
    void (*execute)(void* context);           // Execute the interrupt action
    void* context;                           // Sensor or service context
    
    // Interrupt-specific data (union for memory efficiency)
    union Data {
        const char* panelName;               // Panel to load
        const char* theme;                   // Theme to set
        ButtonAction action;                 // Button action to perform
    } data;
    
    // Control flags
    bool blocking;                           // If true, prevents restoration when active
    InterruptFlags flags;                    // Runtime state flags (int8_t bitfield)
    
    // Helper methods for flag management
    bool IsActive() const { 
        return (flags & InterruptFlags::ACTIVE) != InterruptFlags::NONE; 
    }
    void SetActive(bool active) { 
        if (active) flags |= InterruptFlags::ACTIVE; 
        else flags &= ~InterruptFlags::ACTIVE; 
    }
    bool NeedsExecution() const { 
        return (flags & InterruptFlags::NEEDS_EXECUTION) != InterruptFlags::NONE; 
    }
    void SetNeedsExecution(bool needs) {
        if (needs) flags |= InterruptFlags::NEEDS_EXECUTION;
        else flags &= ~InterruptFlags::NEEDS_EXECUTION;
    }
};
```

**Memory Usage**: Approximately 29 bytes per interrupt (static array of 32 max = ~928 bytes total)

### Handler Architecture (v3.0)

#### PolledHandler (GPIO State Monitoring)
Handles GPIO sensors and manages their state changes:

**Owned Sensors**:
- KeyPresentSensor (GPIO 25)
- KeyNotPresentSensor (GPIO 26)  
- LockSensor (GPIO 27)
- LightsSensor (GPIO 33)
- DebugErrorSensor (GPIO 34, debug builds only)

**Processing Model**:
- Evaluates sensors only during UI IDLE state
- Uses BaseSensor change detection pattern
- Triggers interrupts when sensor states change
- Handles priority-based interrupt execution

#### QueuedHandler (Button Event Processing)
Manages button sensor for user input:

**Owned Sensors**:
- ButtonSensor (GPIO 32)

**Processing Model**:
- Evaluates button state continuously for responsiveness
- Queues button events (short/long press) for later execution
- Executes queued events only during UI IDLE state
- Manages press duration detection (50ms-2000ms short, 2000ms-5000ms long)

### InterruptManager Coordination
Central coordinator that:
- Manages static interrupt array (32 max interrupts)
- Routes interrupts to appropriate handlers based on InterruptSource
- Handles interrupt registration/unregistration
- Coordinates evaluation and execution timing
- Manages button function injection for universal action interrupts

## Sensor Architecture

### Current Sensor Implementation
Each GPIO pin has exactly one dedicated sensor class:

**GPIO Sensors (owned by PolledHandler)**:
- **KeyPresentSensor** (GPIO 25): Independent class for key present detection
- **KeyNotPresentSensor** (GPIO 26): Independent class for key not present detection
- **LockSensor** (GPIO 27): Lock status monitoring
- **LightsSensor** (GPIO 33): Day/night detection

**Button Sensor (owned by QueuedHandler)**:
- **ButtonSensor** (GPIO 32): User input with timing detection, triggers action interrupts

**Data Sensors (owned by panels)**:
- **OilPressureSensor**: Continuous pressure monitoring
- **OilTemperatureSensor**: Continuous temperature monitoring

### BaseSensor Pattern
All sensors inherit from BaseSensor for consistent change detection:

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

### Sensor Independence Requirements
**Architectural Principle**: Split sensor design prevents initialization conflicts:
- Independent initialization flags
- Separate change detection state
- No shared state between sensors
- Individual GPIO resource management

## Managers

- **InterruptManager**: Coordinates PolledHandler and QueuedHandler, manages interrupt registration
- **PanelManager**: Creates panels on demand, manages switching and lifecycle
- **PreferenceManager**: Persistent settings storage
- **StyleManager**: LVGL themes (Day/Night) controlled by LightsSensor
- **ErrorManager**: Error collection (basic implementation, no trigger integration yet)

## Main Loop Processing Flow

### Current Processing Model
```cpp
void loop() {
    // 1. LVGL tasks
    lv_task_handler();
    
    // 2. Process interrupts
    interruptManager->Process();   // Coordinates both handlers
    
    // 3. Process errors
    errorManager->Process();
    
    // 4. Update panels
    panelManager->Update();
}
```

### InterruptManager Processing
1. **Evaluate Interrupts**: 
   - QueuedHandler always evaluates (button responsiveness)
   - PolledHandler evaluates only during UI IDLE
2. **Execute Interrupts**: Both handlers execute only during UI IDLE
3. **Handle Restoration**: Simple restoration logic based on blocking flags

## Available Panels

### Panel Types and Functionality

- **Splash Panel**: Startup animation
  - Short Press: Skip animation, load default panel
  - Long Press: Load config panel
  
- **Oil Panel** (OemOilPanel): Primary monitoring display
  - Creates own OilPressureSensor and OilTemperatureSensor
  - Dual gauge display with continuous animation
  - Short Press: No action
  - Long Press: Load config panel
  
- **Key Panel**: Security key status indicator (Display-only)
  - Visual states: Green icon (present) / Red icon (not present)
  - No sensor creation - receives state via interrupts
  - Short Press: No action
  - Long Press: Load config panel
  
- **Lock Panel**: Vehicle security status (Display-only)
  - Lock engaged/disengaged indication
  - No sensor creation - receives state via interrupts
  - Short Press: No action
  - Long Press: Load config panel
  
- **Error Panel**: System error management
  - Basic error display functionality
  - Short Press: Navigate errors
  - Long Press: Clear errors
  
- **Config Panel**: System configuration
  - Menu navigation for settings
  - Short Press: Navigate options
  - Long Press: Select option

## Hardware Configuration

### Display
- GC9A01 240x240 round display
- SPI2_HOST interface
- Configurable LVGL buffering

### GPIO Mapping (Current Implementation)
| GPIO | Sensor | Purpose | Handler |
|------|--------|---------|---------|
| **GPIO 32** | ButtonSensor | User input button (action interrupts) | QueuedHandler |
| **GPIO 25** | KeyPresentSensor | Key present detection | PolledHandler |
| **GPIO 26** | KeyNotPresentSensor | Key not present detection | PolledHandler |
| **GPIO 27** | LockSensor | Vehicle lock status | PolledHandler |
| **GPIO 33** | LightsSensor | Day/night detection | PolledHandler |
| **GPIO 34** | DebugErrorSensor | Debug error trigger (dev only) | PolledHandler |
| **GPIO 36** | OilPressureSensor | Oil pressure (ADC) | OilPanel |
| **GPIO 39** | OilTemperatureSensor | Oil temperature (ADC) | OilPanel |

## Memory Architecture

### Current Memory Usage
- **Interrupt Storage**: Static array, ~928 bytes total (32 × 29 bytes)
- **Handler Storage**: Two handler instances with owned sensors
- **Static Callbacks**: Function pointers only, no heap allocation

### Sensor Ownership Model
```
Main
├── Creates ProviderFactory
│   ├── GpioProvider
│   ├── DisplayProvider
│   └── DeviceProvider
└── Creates ManagerFactory (with providers injected)
    ├── Creates InterruptManager
    │   ├── Creates PolledHandler (owns GPIO sensors)
    │   │   ├── KeyPresentSensor (GPIO 25)
    │   │   ├── KeyNotPresentSensor (GPIO 26)
    │   │   ├── LockSensor (GPIO 27)
    │   │   └── LightsSensor (GPIO 33)
    │   └── Creates QueuedHandler (owns button sensor)
    │       └── ButtonSensor (GPIO 32)
    └── Creates PanelManager
        └── Creates panels on demand
            ├── KeyPanel (display-only)
            ├── LockPanel (display-only)
            └── OilPanel (creates own data sensors)
```

## Key Interfaces

### Factory Interfaces
- `IProviderFactory`: Factory interface for creating hardware providers
- `IManagerFactory`: Factory interface for creating managers

### Provider Interfaces (Hardware Abstraction)
- `IGpioProvider`: Hardware GPIO abstraction
- `IDisplayProvider`: LVGL display abstraction
- `IDeviceProvider`: Hardware device driver abstraction

### Component Interfaces
- `IPanel`: Screen implementation interface
- `IActionService`: Universal button function interface
- `IComponent`: UI element rendering interface
- `ISensor`: Data acquisition interface
- `IHandler`: Unified interrupt processing interface

## Service Architecture

### Factory Pattern Design
- **ProviderFactory**: Creates hardware providers (implements IProviderFactory)
- **ManagerFactory**: Creates managers with injected dependencies

### Dependency Injection
```cpp
// Production
auto providerFactory = std::make_unique<ProviderFactory>();
auto managerFactory = std::make_unique<ManagerFactory>(providerFactory.get());

// Testing
auto mockProviderFactory = std::make_unique<MockProviderFactory>();
auto managerFactory = std::make_unique<ManagerFactory>(mockProviderFactory.get());
```

## Testing Limitations

### Platform Constraints
- **PlatformIO Unity**: Build filters don't work, test files must be in root directory
- **Wokwi Emulator**: Square display instead of round, horizontal image inversion
- **ESP32 Memory**: Limited RAM affects test complexity

### Current Testing Status
- **Unit Tests**: Basic framework exists, minimal test coverage
- **Integration Tests**: Wokwi setup available but tests not implemented
- **Memory Profiling**: Not implemented

## Implementation Constraints

### ESP32-Specific
- **Memory**: ~250KB available RAM after OTA partitioning
- **Single-Core**: LVGL operations cannot be interrupted
- **Resource Cleanup**: Proper GPIO interrupt detachment required

### Architecture Constraints
- **Single Ownership**: Each GPIO has exactly one sensor instance
- **Display-Only Panels**: Key/Lock panels cannot create sensors
- **Change Detection**: BaseSensor pattern mandatory
- **Static Callbacks**: Required for memory safety

## Coding Standards

The project follows Google C++ Style Guide with project-specific preferences:

### Naming Conventions
- **Classes**: PascalCase (e.g., `PanelManager`)
- **Functions**: PascalCase (e.g., `SetTheme()`)
- **Variables**: snake_case with trailing underscore for members (e.g., `panel_manager_`)
- **Constants**: ALL_CAPS (e.g., `DAY`, `NIGHT`)
- **Files**: snake_case (e.g., `panel_manager.cpp`)
- **Interfaces**: Prefixed with `I` (e.g., `IPanel`)

## Current Status Summary

**Strengths**:
- ✅ Solid v3.0 interrupt architecture with working handlers
- ✅ Complete sensor ownership model prevents resource conflicts
- ✅ Factory pattern with dependency injection working
- ✅ All core panels implemented and functional
- ✅ Memory-safe static callback system

**Areas for Improvement**:
- ⚠️ Error system only partially integrated with interrupt system
- ⚠️ Button system functional but could be more sophisticated
- ⚠️ Testing coverage minimal
- ⚠️ Memory usage not profiled/validated

**Proposed Future Work** (see `docs/plans/proposed-interrupt-architecture.md`):
- v4.0 Trigger/Action architecture with dual functions
- Enhanced priority override logic
- More sophisticated button handling
- Complete error system integration
- Comprehensive testing framework