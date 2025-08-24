# Product Requirements Document - Clarity Digital Gauge System

## 1. Product Overview

### 1.1 Product Name
Clarity - ESP32-based Digital Automotive Gauge System

### 1.2 Product Description
Clarity is a digital gauge system designed for automotive engine monitoring, built on the ESP32 platform with a 1.28" round display (240x240 GC9A01 driver). The system provides real-time monitoring of engine parameters, security status, and system configuration through an intuitive visual interface.

### 1.3 Target Hardware
- **Microcontroller**: ESP32-WROOM-32 (NodeMCU-32S development board)
- **Display**: 1.28" round LCD with GC9A01 driver (240x240 resolution)
- **Interface**: SPI2_HOST for display communication
- **Memory**: 4MB flash with custom partitioning for OTA updates
- **Input**: Single push button on GPIO 32
- **Sensors**: Digital (GPIO) and Analog (ADC) inputs

## 2. Functional Requirements

### 2.1 Architecture Pattern

#### 2.1.1 MVP (Model-View-Presenter) Implementation
- **Models (Sensors)**: Hardware abstraction for data acquisition with change detection
  - Read GPIO and ADC values with state tracking
  - Provide clean data interfaces with unit conversion
  - Report readings in injected units of measure
  - Implement change detection for trigger system integration
  - Single ownership model - each GPIO has exactly one sensor instance
  
- **Views (Components)**: UI element rendering
  - LVGL-based visual components
  - No business logic
  - Responsible for display scales and visual representation
  - Trigger panels (Key, Lock) are display-only without sensors
  
- **Presenters (Panels)**: Business logic and orchestration
  - Coordinate components for display
  - Implement lifecycle: init → load → update
  - Handle panel-specific input behaviors via IActionService
  - Trigger panels receive state from interrupt system, don't read sensors
  - Data panels (Oil) create their own data sensors

#### 2.1.2 Dependency Injection
- All system components use dependency injection for testability and loose coupling
- **Dual factory pattern**: ProviderFactory creates providers, ManagerFactory creates managers
- **Interface-based factories**: IProviderFactory interface enables mock provider injection for testing
- **Generic factory support**: Template-based IFactory<T> for type-safe component creation
- **Dependency resolution**: Managers and panels receive injected dependencies via constructor
- **Service container pattern**: Centralized manager initialization with dependency wiring
- **Comprehensive interface design**: 
  - **Factories**: IProviderFactory, IManagerFactory, IFactory<T>
  - **Providers**: IGpioProvider, IDisplayProvider, IDeviceProvider  
  - **Components**: IPanel, IComponent, ISensor, IHandler, IActionService, IManager
  - **Services**: ILogger, IPreferenceService (future extensions)

### 2.2 Panel System

#### 2.2.1 Available Panels with Button Behaviors

1. **Splash Panel**: Startup animation with user control
   - **Auto-transition**: To default panel after animation completion
   - **Short Press**: Skip animation, load default panel immediately
   - **Long Press**: Load config panel for system settings
   - **Display**: Animated startup sequence with branding
   
2. **Oil Panel** (OemOilPanel): Primary monitoring display
   - **Primary Function**: Dual gauge monitoring (pressure/temperature)
   - **Display**: 240x240px gauges positioned side-by-side with OEM styling
   - **Animation**: Continuous gauge needle animation reflecting sensor data
   - **Units**: Configurable units of measure with delta-based smooth updates
   - **Short Press**: No action (reserved for future functionality)
   - **Long Press**: Load config panel for system configuration
   - **Sensors**: Creates own OilPressureSensor and OilTemperatureSensor internally
   
3. **Key Panel**: Security key status indicator (Display-only)
   - **Visual States**: Green icon (key present) / Red icon (key not present)
   - **Trigger Source**: Activated by KeyPresentSensor/KeyNotPresentSensor via PolledHandler
   - **Priority**: CRITICAL - overrides all other panels when active
   - **Short Press**: No action (status display only)
   - **Long Press**: Load config panel
   - **Sensors**: None - display-only, receives state from interrupt system
   
4. **Lock Panel**: Vehicle security status indicator (Display-only)
   - **Visual States**: Lock engaged/disengaged indication
   - **Trigger Source**: Activated by LockSensor via PolledHandler
   - **Priority**: IMPORTANT - overrides user panels but lower than key panels
   - **Short Press**: No action (status display only)
   - **Long Press**: Load config panel
   - **Sensors**: None - display-only, receives state from interrupt system
   
5. **Error Panel**: System error management with navigation
   - **Primary Function**: Display and manage system errors with user interaction
   - **Display**: Scrollable error list with severity color-coding (Critical/Error/Warning)
   - **Navigation**: Cycle through multiple errors for review
   - **Short Press**: Cycle to next error in queue (marks current error as viewed)
   - **Long Press**: Clear all acknowledged/viewed errors
   - **Auto-Restoration**: Automatically restore previous panel when all errors viewed
   - **Priority**: CRITICAL - highest priority, overrides all other panels
   - **State Management**: Tracks viewed errors and auto-restoration conditions
   
6. **Config Panel**: System configuration with hierarchical navigation
   - **Architecture**: Multi-level state machine with menu hierarchy
   - **Navigation**: Main Menu → Submenus → Option Selection → Application
   - **Short Press**: Navigate through current menu level options
   - **Long Press**: Select highlighted option / Enter submenu / Apply setting
   - **Menu Structure**:
     - **Main Menu**: Theme Settings, Default Panel, Exit
     - **Theme Submenu**: Day Theme, Night Theme, Back
     - **Panel Submenu**: Default panel selection options
   - **Integration**: StyleManager (themes), PreferenceManager (persistence), PanelManager (defaults)
   - **State Persistence**: Menu state maintained during navigation

#### 2.2.2 Panel Switching and Universal Button System

**Automatic Panel Switching**:
- **Priority-Based**: CRITICAL (Key/Error) > IMPORTANT (Lock) > User-driven (Oil)
- **Trigger-Driven**: GPIO state changes automatically load appropriate panels
- **Restoration**: Return to user-driven panel when all non-maintaining interrupts inactive

**Universal Button System**:
- **IActionService Interface**: All panels implement universal button functions
- **Function Injection**: Panel's short/long press functions injected into QUEUED interrupts
- **Dynamic Updates**: Button interrupts updated when panels switch
- **Context Management**: Panel context passed to button functions for execution

**Panel Lifecycle Management**:
- **Seamless Transitions**: Proper cleanup and initialization during switching
- **Memory Safety**: Panel creation/destruction managed by PanelManager
- **State Preservation**: Theme and configuration state persists across switches

### 2.3 Interrupt System

#### 2.3.1 Critical Constraint
**INTERRUPTS CAN ONLY BE CHECKED DURING SYSTEM IDLE TIME**
- No interrupt processing during LVGL operations
- InterruptManager coordinates PolledHandler and QueuedHandler
- Priority-based coordination across both handlers
- Panel manager automatically manages idle/busy states
- Individual panels can update state during animations

#### 2.3.1b Critical Memory Constraint
**HEAP CORRUPTION PREVENTION REQUIREMENT**
- `std::function` usage causes heap fragmentation on ESP32
- Lambda captures create heap objects leading to system crashes
- All interrupt callbacks MUST use static function pointers
- No dynamic allocations during interrupt processing
- Memory-efficient design is mandatory for system stability

#### 2.3.2 Architecture Overview
**COORDINATED INTERRUPT SYSTEM**

For detailed architecture, see: **[Architecture Document](architecture.md)**

**Key Components**:
1. **InterruptManager**: Coordinates PolledHandler and QueuedHandler during idle time
2. **PolledHandler**: GPIO state monitoring with change detection
3. **QueuedHandler**: Button event processing with latest event handling
4. **Effect-Based Execution**: LOAD_PANEL, SET_THEME, SET_PREFERENCE, BUTTON_ACTION effects
5. **Memory Safety**: Static function pointers with void* context parameters

#### 2.3.3 POLLED Interrupts (replaces Triggers)

**POLLED Interrupt Architecture**:
POLLED interrupts monitor GPIO state transitions and execute effects exactly once per change:

**Change-Based Evaluation**:
- POLLED interrupts only activate when sensor states actually change
- Each interrupt action executes exactly once per state transition  
- No repeated execution for maintained states
- Eliminates performance issues from redundant operations

**Interrupt Structure**:

For complete interrupt structure details, see: **[Architecture Document](architecture.md#coordinated-interrupt-system-architecture)**

**Memory Safety Requirements**:
- Static function pointers with void* context parameters
- No heap allocation during interrupt processing
- ESP32-optimized memory patterns

**Panel Restoration**:
- Only non-maintaining interrupts block restoration
- Theme changes always maintain and never block restoration
- Restoration occurs when no panel-loading interrupts are active

For detailed restoration logic, see: **[Architecture Document](architecture.md#coordinated-interrupt-processing-system)**

**Coordinated Processing**:
For detailed processing flow, see: **[Application Flow Diagram](diagrams/application-flow.md)**

**Interrupt Registration Examples**:
For complete registration patterns, see: **[Architecture Document](architecture.md#coordinated-interrupt-registration)**

**Registered Interrupts**: See **[Architecture Document](architecture.md)** for complete interrupt list.

**Sensor Change Detection**:
All sensors inherit from BaseSensor for consistent change detection. For implementation details, see: **[Architecture Document](architecture.md#sensor-architecture)**

**Sensor Implementation**: See **[Architecture Document](architecture.md#sensor-architecture)** for complete patterns.

**Sensor Ownership**:
- **PolledHandler**: Creates and owns GPIO sensors for state monitoring
- **QueuedHandler**: Creates and owns button sensor for event processing  
- **Data Panels**: Create own data sensors internally
- **Display-Only Panels**: Create only components, no sensors

For complete ownership model, see: **[Architecture Document](architecture.md#memory-architecture)**

**Resource Management**: See **[Architecture Document](architecture.md#memory-architecture)** for complete ownership model and factory patterns.

**Priority and Restoration**: See **[Architecture Document](architecture.md#coordinated-interrupt-processing-system)** for complete priority system and restoration logic.

**Processing Flow**: See **[Application Flow Diagram](diagrams/application-flow.md)** for detailed interrupt processing steps.

**Architecture Principles**: See **[Architecture Document](architecture.md)** for complete constraints and restoration logic.

**Multi-Trigger Scenarios**: See **[Test Scenarios Document](scenario.md)** for complete behavior examples.

**Key Sensor Architecture**: See **[Architecture Document](architecture.md#sensor-architecture)** for complete split sensor design and implementation requirements.

#### 2.3.4 QUEUED Interrupts

**QUEUED Interrupt Architecture**: Handles button events with single latest event processing.

For complete implementation details, see: **[Architecture Document](architecture.md#queued-interrupts)**

**Universal Panel Button Functions**: All panels implement short/long press functions called via QUEUED interrupts.

**Universal Panel Button Implementation**:
Every panel implements the IActionService interface providing consistent button behavior across the system.

**Button Hardware Configuration**:
- **GPIO**: Single button on GPIO 32 with INPUT_PULLDOWN configuration
- **Timing**: Short press (50ms-2000ms), Long press (2000ms-5000ms)
- **Debouncing**: 50ms debounce time to prevent false triggers
- **Processing**: QUEUED interrupts with latest event processing (not queue-based)

**Button Integration Flow**:
1. **Button Press Detection**: ActionButtonSensor detects press type and duration
2. **Event Queueing**: QueuedHandler receives latest button event (replaces previous)
3. **Interrupt Evaluation**: Universal button interrupts evaluate for pending events
4. **Function Execution**: Current panel's injected function executed with panel context
5. **State Update**: Button event cleared after successful execution

For complete input system details, see: **[Input System Documentation](input.md)**

#### 2.3.5 Errors

**Error Architecture**:
Errors are integrated into the unified interrupt system as POLLED interrupts with CRITICAL priority:

**Error Interrupt Implementation**:
- `error_occurred` interrupt registered as POLLED source with CRITICAL priority
- ErrorManager integration through unified interrupt evaluation function
- Automatic panel loading when errors are present
- Priority precedence over all other interrupts

**Error Interrupt Registration Example**:
```cpp
// Error interrupt routed to PolledHandler by InterruptManager
interruptManager.RegisterInterrupt({
    .id = "error_occurred",
    .priority = Priority::CRITICAL,
    .source = InterruptSource::POLLED,
    .effect = InterruptEffect::LOAD_PANEL,
    .evaluationFunc = HasPendingErrors,
    .activateFunc = LoadErrorPanel,
    .deactivateFunc = nullptr,
    .sensorContext = errorManager,
    .serviceContext = panelManager,
    .data = { .panel = { "ERROR", true, false } }  // Track for restoration, don't maintain
});
```

**Error Levels**:
- **WARNING**: Non-critical issues (auto-dismiss after 10 seconds)
- **ERROR**: Significant issues affecting features
- **CRITICAL**: Issues requiring immediate attention

**Error Service Architecture**:
- **Global ErrorManager**: Singleton service for error collection and management
- **Bounded Error Queue**: Max 10 errors with FIFO replacement when full
- **Error Trigger Integration**: ErrorManager state checked by error trigger evaluation
- **No Try-Catch**: Preserve Arduino crash reporting in main loops

**Error Lifecycle**:
1. **Error Occurrence**: Component reports error to ErrorManager
2. **Trigger Evaluation**: Error trigger evaluates true when errors pending
3. **Panel Loading**: Error trigger executes, loads ErrorPanel with CRITICAL priority
4. **User Interaction**: User acknowledges/dismisses errors via panel actions
5. **Trigger Deactivation**: When no errors remain, trigger evaluates false
6. **Panel Restoration**: System returns to previous user-driven panel

#### 2.3.6 Handler Architecture

**IHandler Interface**:
```cpp
class IHandler {
public:
    virtual ~IHandler() = default;
    virtual void Process() = 0;
};
```

**Base Interrupt Constructs**:
All interrupts are structs with function pointers for evaluation and execution.

**Trigger Struct Implementation**:
```cpp
struct TriggerInterrupt {
    const char* id;  // Static string for memory efficiency
    TriggerPriority priority;
    bool (*hasChangedFunc)();      // Function pointer - no heap allocation
    bool (*getCurrentStateFunc)();  // Function pointer - no heap allocation
    void (*executionFunc)();       // Function pointer - no heap allocation
};
```

**Action Struct Implementation**:
```cpp
struct ActionInterrupt {
    const char* id;  // Static string for memory efficiency
    Priority priority;
    bool (*evaluationFunc)();      // Function pointer - no heap allocation
    void (*executionFunc)();       // Function pointer - no heap allocation
    ActionType actionType;                     // SHORT_PRESS, LONG_PRESS
};
```

**Handler Implementations**:
- **PolledHandler**: Implements IHandler, manages POLLED interrupts for GPIO state monitoring
- **QueuedHandler**: Implements IHandler, manages QUEUED interrupts for button event processing
- **Registration**: Unified Interrupt struct registration
- **Coordinated Processing**: InterruptManager coordinates both handlers by priority

For complete implementation details, see: **[Architecture Document](architecture.md#coordinated-interrupt-system-architecture)**

#### 2.3.7 Registration Pattern

POLLED and QUEUED interrupts are registered at startup as struct instances:

```
// Register POLLED interrupt with PolledHandler (via InterruptManager)
interruptManager.RegisterInterrupt({
    .id = "key_present",
    .priority = Priority::CRITICAL,
    .source = InterruptSource::POLLED,
    .effect = InterruptEffect::LOAD_PANEL,
    .evaluationFunc = IsKeyPresent,
    .activateFunc = LoadKeyPanel,
    .sensorContext = keyPresentSensor,
    .serviceContext = panelManager
});

// Register QUEUED interrupt with QueuedHandler (via InterruptManager)
interruptManager.RegisterInterrupt({
    .id = "short_press",
    .priority = Priority::NORMAL,
    .source = InterruptSource::QUEUED,
    .effect = InterruptEffect::BUTTON_ACTION,
    .evaluationFunc = HasShortPressEvent,
    .activateFunc = ExecutePanelShortPress,
    .sensorContext = actionButtonSensor,
    .serviceContext = panelManager
});
```

**Registered Interrupts**:
- **Triggers**: key_present, key_not_present, lock_state, lights_state, error_occurred
- **Actions**: short_press_action, long_press_action

This approach provides:
- Specialized handlers per interrupt source (POLLED vs QUEUED)
- Direct struct registration without intermediate handler objects
- Clear separation between trigger and action processing logic

#### 2.3.8 Evaluation Process

**Polymorphic Handler Processing**:
InterruptManager coordinates both handlers through the unified IHandler interface:

```cpp
void InterruptManager::CheckAllInterrupts() {
    polledHandler_->Process();   // IHandler* - processes POLLED interrupts
    queuedHandler_->Process();   // IHandler* - processes QUEUED interrupts  
}
```

**Handler-Specific Processing**:
1. **PolledHandler::Process()** (GPIO state monitoring):
   - Evaluates POLLED interrupts for state changes
   - Marks active interrupts for InterruptManager coordination
   
2. **QueuedHandler::Process()** (button event processing):
   - Evaluates QUEUED interrupts for latest button event
   - Marks active interrupts for InterruptManager coordination
   
3. **InterruptManager coordination**:
   - Compares highest priority interrupt from each handler
   - Executes highest priority interrupt across both handlers

**Architecture Benefits**:
- **Polymorphic Processing**: InterruptManager treats both handlers identically
- **Type Safety**: Registration methods remain type-specific outside interface
- **Precedence**: Trigger processing always occurs before action processing
- **Testability**: Single IHandler interface simplifies mocking

#### 2.3.9 Benefits of Change-Based Architecture
- **Performance Optimization**: Actions execute once per change instead of repeatedly
- **CPU Efficiency**: Eliminates unnecessary repeated operations during steady states
- **Memory Stability**: No fragmentation from repeated object creation/destruction
- **Predictable Behavior**: Clear semantics for when actions execute
- **System Stability**: Eliminates hanging caused by inconsistent trigger types
- **Consistent Implementation**: All POLLED interrupts follow identical change-detection pattern
- **Easier Debugging**: State changes are explicit and trackable
- **Reduced Power Consumption**: Less CPU usage during idle periods

#### 2.3.10 Performance Requirements
**Target Performance Metrics**:
- **Theme Setting Frequency**: Maximum 2 executions per second (only on actual changes)
- **Interrupt Processing Time**: Consistent regardless of maintained trigger states
- **CPU Usage During Idle**: Minimal processing overhead when no state changes occur
- **Memory Usage**: Stable without fragmentation from repeated operations

**Change Detection Implementation Requirements**:

**BaseSensor Pattern (Mandatory)**:
All sensors must inherit from BaseSensor base class for consistent change detection:
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

**Change Detection Rules**:
1. **First Read Initialization**: No change detected on first sensor read
2. **State Persistence**: Previous state maintained between calls
3. **Atomic Updates**: State update and change detection must be atomic
4. **Thread Safety**: Change detection must be safe for single-threaded ESP32

**Change Detection Performance**:
- **Sensor State Reads**: Execute only during interrupt evaluation cycles
- **Change Comparison**: Simple boolean comparison per sensor
- **Initialization Overhead**: One-time cost during sensor initialization
- **Scalability**: Performance scales linearly with number of sensors, not trigger frequency
- **Phase-Based Processing**: Single evaluation per trigger per cycle prevents corruption

#### 2.3.11 Priority-Based Restoration Implementation
**Blocking Trigger Detection**:
```cpp
// Check if any blocking panel-loading interrupts are active
bool HasBlockingPanelInterruptActive() const {
    for (const auto& interrupt : polledInterrupts_) {
        if (interrupt.active &&
            (interrupt.priority == CRITICAL || interrupt.priority == IMPORTANT) &&
            interrupt.effect == InterruptEffect::LOAD_PANEL &&
            !interrupt.data.panel.maintainWhenInactive) {
            return true;
        }
    }
    return false;
}
```

**Restoration Decision Logic**:
When a panel-loading trigger deactivates:
1. Check if any blocking panel-loading interrupts (CRITICAL/IMPORTANT) are active
2. If yes: Execute the highest priority blocking interrupt's activation function
3. If no: Restore the user-driven panel
4. SET_THEME interrupts (lights) never participate in blocking logic

**Example Flow**:
- Key trigger deactivates while lock trigger is active
- System detects lock (IMPORTANT) blocks restoration
- Lock trigger's execution function is called
- Lock panel is loaded instead of restoring user panel

### 2.4 Development and Debug System

#### 2.4.1 Debug Error Trigger System
**Purpose**: Enable testing and validation of error handling system during development.

**Hardware Configuration**:
- **GPIO 34**: Input-only pin for debug error trigger (development builds only)
- **External Circuit**: Requires external pull-down resistor (GPIO 34 has no internal pull resistors)
- **Activation**: HIGH signal triggers debug error generation

**Debug Error Integration**:
- **Conditional Compilation**: Only active in CLARITY_DEBUG builds
- **Error Generation**: Creates test errors for system validation and error panel testing
- **Interrupt Registration**: debug_error_trigger interrupt with CRITICAL priority
- **Error Types**: Configurable error levels (WARNING/ERROR/CRITICAL) for comprehensive testing

**Development Features**:
- **Error Panel Testing**: Validate error cycling, acknowledgment, and auto-restoration
- **Priority Testing**: Verify error interrupts override other panels correctly
- **System Integration**: Test complete error workflow from detection to restoration
- **Hardware Validation**: Verify GPIO interrupt handling and debouncing

**Production Builds**: Debug error system completely excluded from release builds for memory optimization.

### 2.5 Sensor System

#### 2.5.1 Sensor Architecture

**Sensor Class Hierarchy**:
All sensors inherit from `ISensor` interface and `BaseSensor` for change detection:

```cpp
class ISensor {
public:
    virtual ~ISensor() = default;
    virtual void Init() = 0;
    virtual Reading GetReading() = 0;
};

class BaseSensor {
protected:
    bool initialized_ = false;
    template<typename T> bool DetectChange(T currentValue, T& previousValue);
};
```

#### 2.5.2 Sensor Types and Implementations

**Digital Sensors**:
- **KeyPresentSensor** (GPIO 25): Key present detection
  - Independent class with isolated change tracking
  - Methods: `GetKeyPresentState()`, `HasStateChanged()`
  - Destructor with DetachInterrupt() for cleanup
  
- **KeyNotPresentSensor** (GPIO 26): Key not present detection  
  - Independent class with isolated change tracking
  - Methods: `GetKeyNotPresentState()`, `HasStateChanged()`
  - Destructor with DetachInterrupt() for cleanup
  
- **LockSensor** (GPIO 27): Vehicle lock status
  - Change detection via BaseSensor inheritance
  - Methods: `GetLockState()`, `HasLockStateChanged()`
  - No interrupt attachment - no destructor needed
  
- **LightsSensor** (GPIO 33): Day/night theme switching
  - Change detection for NORMAL priority triggers
  - Methods: `GetLightsState()`, `HasStateChanged()`
  - No interrupt attachment - no destructor needed

- **ActionButtonSensor** (GPIO 32): User input button
  - Debouncing and timing logic
  - Owned by QueuedHandler

**Analog Sensors**: ADC-based continuous measurements
- **OilPressureSensor**: Pressure monitoring with unit conversion
- **OilTemperatureSensor**: Temperature monitoring

#### 2.5.3 Unit Management
- Units of measure injected via SetTargetUnit() method
- Sensors implement GetSupportedUnits() for validation
- Sensors report readings in requested units via ConvertReading()
- Panels control visual scales and ranges independently
- OilPressureSensor supports Bar, PSI, kPa units

### 2.6 Theme System

#### 2.6.1 Available Themes
- **Day Theme**: White/bright colors for daylight visibility
- **Night Theme**: Red/dark colors for night vision

#### 2.6.2 Theme Switching
- Automatic based on light sensor
- Instant switching without panel reload
- Theme persists across panel switches

## 3. Non-Functional Requirements

### 3.1 Memory Management Requirements

#### 3.1.1 ESP32 Memory Constraints
**Critical Limitation**: ESP32-WROOM-32 has 320KB total RAM, with ~250KB available after system overhead and OTA partitioning
- All architecture decisions must account for memory constraints
- Heap fragmentation prevention is mandatory
- Dynamic allocation minimization required
- Memory-efficient patterns must be used throughout

#### 3.1.2 Static Callback Requirements
**Mandatory Pattern**: All interrupt system callbacks must use static function pointers
- Eliminates `std::function` heap allocation overhead
- Prevents lambda capture heap objects
- Provides predictable memory usage
- Prevents system crashes from heap fragmentation

#### 3.1.3 LVGL Buffer Optimization
**Memory Usage**: Dual 60-line buffers consume 57.6KB (240×60×2×2 bytes)
**Optimization Requirements**:
- Single buffer mode capability for 30KB savings
- Reduced line count options for additional 15KB savings
- Dynamic buffer allocation based on panel complexity
- Memory-efficient rendering modes

#### 3.1.4 Pointer Management Strategy
**Required Ownership Pattern**:
- main.cpp owns all sensors via unique_ptr (transfer ownership)
- Panels receive raw pointers (non-owning references)
- Managers receive raw pointers (non-owning references)
- Factory methods return unique_ptr (clear ownership transfer)
- Avoid shared_ptr overhead (16-24 bytes per instance)

### 3.2 Error Prevention Requirements

#### 3.2.1 Heap Corruption Prevention
**Critical Requirement**: System must prevent heap corruption that causes crashes
- **Root Cause**: `std::function` with lambda captures fragments ESP32 heap
- **Solution**: Static function pointers with context parameters exclusively
- **Enforcement**: Build-time verification of callback patterns
- **Testing**: Memory stress testing during LVGL operations

#### 3.2.2 Change Detection Corruption Prevention
**Problem**: Multiple calls to `HasStateChanged()` corrupt sensor state
**Requirements**:
- Separate change detection from state retrieval functions
- Single evaluation rule: each trigger evaluated once per cycle
- Phase-based processing to prevent multiple evaluations
- BaseSensor pattern for consistent change tracking

#### 3.2.3 Resource Conflict Prevention
**GPIO Resource Management**:
- Each GPIO pin has exactly one sensor instance
- Proper DetachInterrupt() calls in sensor destructors
- No sensor duplication between panels and handlers
- Build-time enforcement of single ownership model

### 3.3 Performance
- Smooth animations at 60 FPS
- Responsive button input (<100ms response time)
- Efficient memory usage within ESP32 constraints
- LVGL buffer: 57.6KB for 60-line dual buffering (240×60×2×2 bytes)

### 3.4 Reliability
- Graceful error handling without system crashes
- Automatic recovery from error states
- Hardware watchdog integration
- Core dump partition for debugging
- Heap corruption prevention through static callbacks
- Memory leak prevention through proper resource cleanup

### 3.5 Implementation Constraints

#### 3.5.1 ESP32-Specific Constraints
- **Single-Core Operation**: LVGL operations must not be interrupted by GPIO processing
- **Memory Fragmentation**: Minimize dynamic allocations during runtime
- **Resource Cleanup**: Proper GPIO interrupt detachment in destructors required
- **Timing Constraints**: Interrupt processing during idle time only

#### 3.5.2 Architectural Constraints
- **Sensor Independence**: Split sensor classes prevent initialization conflicts
- **Display-Only Panels**: Trigger panels must not create sensors
- **Single Ownership**: Each GPIO managed by exactly one sensor
- **Change Detection**: BaseSensor pattern mandatory for all sensors

### 3.6 Maintainability
- Modular architecture with clear separation of concerns
- Comprehensive logging system:
  - log_v: Method entry only
  - log_d: Complex activities within methods
  - log_i: Major functional operations
- Consistent coding standards (Google C++ Style Guide adapted)
- Memory-efficient patterns throughout codebase

### 3.7 Testability
- Unit tests for all major components
- Integration tests for panel transitions
- Mock implementations for hardware dependencies
- Test execution via PlatformIO Unity framework
- Memory stress testing for heap corruption prevention
- Change detection testing for sensor state management

### 3.8 Extensibility
- Easy addition of new panels
- Pluggable sensor architecture
- Configurable trigger mappings
- Theme system extensibility
- Memory-efficient extension patterns

## 4. Technical Constraints

### 4.1 Platform Limitations
- ESP32 memory constraints
- Single-core operation for reliability
- No dynamic memory allocation in critical paths
- LVGL integration requirements

### 4.2 Development Environment
- PlatformIO build system
- Wokwi emulator for local testing (square display limitation)
- Custom partition scheme for OTA updates
- Build environments: debug-local, debug-upload, release

### 4.3 Testing Limitations
- PlatformIO Unity framework constraints
- Test files must be in root test directory
- Build filters not applied to Unity tests
- Wokwi displays image inverted horizontally

## 5. User Interface Requirements

### 5.1 Display Specifications
- 240x240 pixel round display
- Full-color RGB support
- SPI interface for fast updates
- Hardware rotation support

### 5.2 Visual Design
- Clean, automotive-inspired interface
- High contrast for visibility
- Smooth gauge animations
- Clear status indicators

### 5.3 User Feedback
- Visual confirmation for all inputs
- Error states clearly indicated
- Loading/transition animations
- Status badges for pending errors

## 6. System Integration

### 6.1 Manager Services
- **PanelManager**: Panel lifecycle, switching, and restoration tracking
- **InterruptManager**: Orchestrates interrupt checking during LVGL idle time
  - **PolledHandler**: GPIO state monitoring with change detection
  - **QueuedHandler**: Button event processing with latest event handling
- **StyleManager**: Theme management (Day/Night)
- **PreferenceManager**: Persistent settings storage
- **ErrorManager**: Error collection with change detection integration
- **ProviderFactory**: Implements IProviderFactory, creates all hardware providers
- **ManagerFactory**: Creates all managers with injected IProviderFactory dependency

### 6.2 Service Initialization
- Ordered initialization in main setup
- Dependency resolution
- Graceful degradation on initialization failure

## 7. Security and Safety

### 7.1 Security Features
- Key presence detection
- Lock status monitoring
- No storage of sensitive data
- No network connectivity (air-gapped)

### 7.2 Safety Considerations
- Fail-safe defaults for sensor failures
- Clear error indication
- No critical operations during animations
- Interrupt safety during LVGL operations

## 8. Future Enhancements

### 8.1 Planned Features
- Additional sensor support
- Network connectivity for telemetry
- Mobile app integration
- Advanced configuration options

### 8.2 Extensibility Points
- Plugin architecture for custom panels
- Sensor calibration system
- Theme marketplace
- OTA update mechanism

## 9. Acceptance Criteria

### 9.1 Core Functionality
- All panels load and display correctly
- Trigger system responds within 100ms
- Button input works reliably
- Error handling prevents crashes

### 9.2 User Experience
- Smooth transitions between panels
- Responsive to user input
- Clear visual feedback
- Intuitive navigation

### 9.3 System Stability
- 24-hour continuous operation without crashes
- Graceful handling of sensor disconnection
- Memory usage remains stable
- No LVGL conflicts with interrupt system

## 10. Document History

- **Version 1.0**: Initial comprehensive PRD incorporating architecture, error handling, input system, scenarios, and sensor guidelines
- **Version 1.1**: Updated after codebase review - corrected input timing, updated ErrorManager implementation status, corrected panel names and configurations
- **Version 1.2**: Major architectural update - transition to change-based trigger system
  - Updated trigger architecture to use change-based evaluation instead of state-based
  - Added sensor change detection requirements and implementation patterns
  - Updated trigger registration examples to show change detection patterns
  - Added performance requirements and benefits of change-based architecture
  - Updated multi-trigger scenarios to reflect change-based behaviors
  - Added comprehensive performance metrics and optimization targets
- **Version 1.3**: Key sensor architecture refinement - split sensor design
  - Updated key detection system to use separate KeyPresentSensor and KeyNotPresentSensor classes
  - Eliminated shared initialization conflicts through independent sensor inheritance
  - Updated sensor system documentation to reflect split architecture
  - Modified trigger registration examples to use dedicated sensor classes
  - Enhanced sensor class hierarchy documentation with inheritance patterns
  - Improved architectural alignment with independent trigger design principle
- **Version 1.4**: Coordinated interrupt architecture design
  - Separated PolledHandler and QueuedHandler with central coordination
  - Implemented effect-based execution with activate/deactivate functions
  - Added maintenance-based restoration logic
  - Enforced single evaluation rule: each interrupt processed exactly once per cycle
  - Updated interrupt struct to support unified processing
  - Added comprehensive restoration logic for multi-interrupt scenarios
- **Version 1.5**: Sensor ownership and resource management clarification
  - Added critical architecture constraint for single sensor ownership
  - Specified PolledHandler as owner of GPIO sensors, QueuedHandler as owner of button sensor
  - Eliminated sensor duplication in panels (display-only requirement)
  - Added destructor requirements for proper GPIO resource cleanup
  - Clarified panel responsibilities (trigger panels are display-only)
  - Added architecture enforcement rules to prevent resource conflicts
- **Version 2.0**: Interrupt system architecture alignment
  - Updated all sections to reflect handler-based architecture
  - Documented IHandler interface and struct-based interrupt design
  - Updated sensor descriptions to show split KeyPresentSensor/KeyNotPresentSensor
  - Added code examples showing implementation patterns
  - Reflected removal of TriggerManager and cleanup phases
  - Updated panel descriptions to clarify display-only nature
  - Added performance metrics targets
  - Documented sensor ownership model with ManagerFactory
- **Last Updated**: December 2024