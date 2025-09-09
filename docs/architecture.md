# Architecture

## Overview

MVP (Model-View-Presenter) pattern for ESP32 automotive gauges with layered architecture, interface-based design, and coordinated interrupt system using TriggerHandler/ActionHandler architecture with Trigger/Action separation.

**Related Documentation:**
- **[Requirements](requirements.md)** - Detailed functional and non-functional requirements
- **[Hardware](hardware.md)** - Target hardware and GPIO pin mappings  
- **[Interrupt Architecture](interrupt-architecture.md)** - Detailed Trigger/Action separation design
- **[Standards](standards.md)** - Coding and naming conventions

## Visual Diagrams

For detailed architectural diagrams, see:
- **[Architecture Overview](diagrams/architecture-overview.md)** - Complete component relationships and dependencies
- **[Application Flow](diagrams/application-flow.md)** - Startup sequence, runtime processing, and interrupt handling flows

## Pattern Structure

```
InterruptManager → TriggerHandler → GPIO Sensors → GPIO
                ↘ ActionHandler → Button Sensor ↗          
                                    
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

## Coordinated Interrupt Architecture

### Handler-Based Design
The interrupt system uses InterruptManager to coordinate specialized handlers:

```
InterruptManager: Central coordination of interrupt processing
├── TriggerHandler: Manages GPIO state monitoring (Key, Lock, Lights sensors)
└── ActionHandler: Manages button event processing (ButtonSensor)
```

### Interrupt Processing Model
The system coordinates interrupt evaluation and execution through handlers:

- **Trigger Evaluation**: TriggerHandler checks GPIO sensors for state changes during UI IDLE
- **Action Evaluation**: ActionHandler monitors button sensor every main loop iteration for responsiveness  
- **All Execution**: Only happens during UI IDLE state for LVGL compatibility
- **Priority System**: CRITICAL > IMPORTANT > NORMAL with sophisticated blocking logic

### Interrupt Structure

#### Trigger/Action Separation
The implementation uses separate structures for state-based and event-based interrupts:

```cpp
// Located in include/utilities/types.h
enum class Priority : uint8_t {
    NORMAL = 0,      // Lowest priority (e.g., lights)
    IMPORTANT = 1,   // Medium priority (e.g., lock)
    CRITICAL = 2     // Highest priority (e.g., key, errors)
};

enum class TriggerType : uint8_t {
    PANEL = 0,       // Panel switching triggers
    STYLE = 1,       // Style/theme changing triggers
    FUNCTION = 2     // Function execution triggers
};

// Trigger Structure (State-Based)
struct Trigger {
    const char* id;                          // Unique identifier
    Priority priority;                       // Execution priority
    TriggerType type;                        // Type classification
    
    // Dual-state functions - no context needed
    void (*activateFunc)();                  // Execute when sensor goes ACTIVE
    void (*deactivateFunc)();                // Execute when sensor goes INACTIVE
    
    // State association
    BaseSensor* sensor;                      // Associated sensor (1:1)
    bool isActive;                           // Currently active state
};

// Action Structure (Event-Based)
struct Action {
    const char* id;                          // Unique identifier
    void (*executeFunc)();                   // Execute when action triggered
    bool hasTriggered;                       // Action pending execution
    ActionPress pressType;                   // SHORT or LONG press
};
```

**Memory Efficiency**: Static structures with minimal overhead for ESP32 memory constraints

### Handler Architecture

#### TriggerHandler (GPIO State Monitoring)
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
- Executes dual functions based on state changes
- Implements priority-based override logic

#### ActionHandler (Button Event Processing)
Manages button sensor for user input:

**Owned Sensors**:
- ButtonSensor (GPIO 32)

**Processing Model**:
- Evaluates button state every main loop iteration for responsiveness
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

### Sensor Implementation
Each GPIO pin has exactly one dedicated sensor class:

**GPIO Sensors (owned by TriggerHandler)**:
- **KeyPresentSensor** (GPIO 25): Independent class for key present detection
- **KeyNotPresentSensor** (GPIO 26): Independent class for key not present detection
- **LockSensor** (GPIO 27): Lock status monitoring
- **LightsSensor** (GPIO 33): Day/night detection

**Button Sensor (owned by ActionHandler)**:
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

- **InterruptManager**: Coordinates TriggerHandler and ActionHandler, manages interrupt registration
- **PanelManager**: Creates panels on demand, manages switching and lifecycle
- **PreferenceManager**: Persistent settings storage
- **StyleManager**: LVGL themes (Day/Night) controlled by LightsSensor
- **ErrorManager**: Error collection (basic implementation, no trigger integration yet)

## Main Loop Processing Flow

### Processing Model
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
   - ActionHandler always evaluates (button responsiveness)
   - TriggerHandler evaluates only during UI IDLE
2. **Execute Interrupts**: Both handlers execute only during UI IDLE
3. **Handle Restoration**: Smart restoration logic with priority override

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
| **Digital GPIO** | | | |
| **GPIO 32** | ButtonSensor | User input button (action interrupts) | ActionHandler |
| **GPIO 25** | KeyPresentSensor | Key present detection | TriggerHandler |
| **GPIO 26** | KeyNotPresentSensor | Key not present detection | TriggerHandler |
| **GPIO 27** | LockSensor | Vehicle lock status | TriggerHandler |
| **GPIO 33** | LightsSensor | Day/night detection | TriggerHandler |
| **GPIO 34** | DebugErrorSensor | Debug error trigger (dev only) | TriggerHandler |
| **ADC Channels** | | | |
| **GPIO 36** | OilPressureSensor | Oil pressure (ADC1_0) | OilPanel |
| **GPIO 39** | OilTemperatureSensor | Oil temperature (ADC1_3) | OilPanel |

## Memory Architecture

### Memory Efficiency
- **Trigger Storage**: Static array with minimal overhead
- **Action Storage**: Compact static array structure
- **Handler Storage**: Two handler instances with owned sensors
- **Memory Efficiency**: Optimized for ESP32 constraints
- **Static Callbacks**: Function pointers only, no heap allocation

### Multi-Factory Architecture & Sensor Ownership Model
```
Main
├── Creates ProviderFactory (implements IProviderFactory)
│   ├── Creates DeviceProvider (hardware driver)
│   ├── Creates GpioProvider (GPIO abstraction)
│   └── Creates DisplayProvider (LVGL abstraction)
└── Creates ManagerFactory (receives IProviderFactory)
    ├── Uses IProviderFactory to get providers
    ├── Creates PreferenceManager
    ├── Creates StyleManager (with theme)
    ├── Creates InterruptManager (with GpioProvider)
    │   ├── Creates TriggerHandler (owns GPIO sensors)
    │   │   ├── KeyPresentSensor (GPIO 25)
    │   │   ├── KeyNotPresentSensor (GPIO 26)
    │   │   ├── LockSensor (GPIO 27)
    │   │   └── LightsSensor (GPIO 33)
    │   └── Creates ActionHandler (owns button sensor)
    │       └── ButtonSensor (GPIO 32)
    ├── Creates PanelManager (with all dependencies)
    │   └── Creates panels on demand
    │       ├── KeyPanel (display-only)
    │       ├── LockPanel (display-only)
    │       └── OilPanel (creates own data sensors)
    └── Creates ErrorManager
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

### Multi-Factory Architecture Design

The system uses a multi-factory architecture with 4 specialized factories:

#### Core Dependency Factories (Provider → Manager Pattern):
The primary architectural pattern involves two interdependent factories:

1. **ProviderFactory** (implements IProviderFactory):
   - Creates hardware abstraction providers
   - Manages DeviceProvider, GpioProvider, DisplayProvider
   - Ensures proper initialization order (Device → GPIO → Display)
   - Handles hardware-specific setup and configuration

2. **ManagerFactory** (implements IManagerFactory):
   - Accepts IProviderFactory for provider creation
   - Creates all manager instances with dependency injection
   - Coordinates manager initialization order
   - Enables testability through provider factory injection

#### UI Creation Factories (Singleton Pattern):
Two additional singleton factories handle UI component creation:

3. **PanelFactory** (implements IPanelFactory):
   - Creates panel instances on demand
   - Singleton pattern for global access from managers
   - Registered panel types created dynamically

4. **ComponentFactory** (implements IComponentFactory):
   - Creates UI components for panels
   - Singleton pattern for global access from panels
   - Provides typed component creation methods

### Dependency Injection Flow
```cpp

### Factory Usage Patterns
```cpp
// Core Dependency Pattern (Provider → Manager)
auto providerFactory = std::make_unique<ProviderFactory>();
auto managerFactory = std::make_unique<ManagerFactory>(std::move(providerFactory));

// ManagerFactory internally uses providerFactory to create providers
auto interruptManager = managerFactory->CreateInterruptManager(nullptr); // Gets GPIO from factory
auto panelManager = managerFactory->CreatePanelManager(nullptr, nullptr, ...); // Gets providers from factory

// UI Singleton Pattern (Independent Factories)
auto& panelFactory = PanelFactory::Instance();
auto& componentFactory = ComponentFactory::Instance();
auto panel = panelFactory.CreatePanel("OilPanel", dependencies...);
auto component = componentFactory.CreateGaugeComponent(styleService);

// Test usage with mock factories
auto mockProviderFactory = std::make_unique<MockProviderFactory>();
auto managerFactory = std::make_unique<ManagerFactory>(std::move(mockProviderFactory));
```

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

## System Capabilities

**Core Features**:
- Sophisticated interrupt architecture with Trigger/Action separation
- Complete sensor ownership model prevents resource conflicts
- Multi-factory architecture with specialized concerns
- Core dependency pattern: Provider → Manager factory chain
- Hardware abstraction via ProviderFactory
- Business logic management via ManagerFactory with provider injection
- UI creation via singleton PanelFactory and ComponentFactory
- All core panels implemented and functional
- Memory-safe static callback system
- Priority-based override logic for complex scenarios
- Smart restoration with last user panel tracking
- Memory-optimized design for ESP32 constraints
- Advanced error handling integration
- Extended sensor support architecture