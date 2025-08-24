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

#### 2.2.1 Available Panels
1. **Splash Panel**: Startup animation and branding
   - Auto-transitions to default panel after animation
   - Skippable via button press
   
2. **Oil Panel** (OemOilPanel): Primary monitoring display
   - Dual gauge display (pressure and temperature) positioned side-by-side
   - 240x240px gauges with OEM styling
   - Configurable units of measure with delta-based updates
   - Smart caching and animation-based smooth transitions
   
3. **Key Panel**: Security key status indicator (Display-only)
   - Green icon when key present (triggered by KeyPresentSensor)
   - Red icon when key not present (triggered by KeyNotPresentSensor)
   - No sensor creation - receives state from TriggerHandler
   
4. **Lock Panel**: Vehicle security status (Display-only)
   - Visual indication of lock state
   - No sensor creation - receives state from TriggerHandler
   
5. **Error Panel**: System error display
   - Scrollable list of active errors
   - Color-coded by severity (Critical/Error/Warning)
   - Error acknowledgment functionality
   
6. **Config Panel**: System configuration
   - Basic ConfigPanel with dummy implementation
   - DynamicConfigPanel for full functionality
   - Default panel selection
   - Splash screen toggle
   - Unit preferences (future)

#### 2.2.2 Panel Switching
- Automatic switching based on trigger priority
- Manual switching via button input
- Seamless transitions with proper cleanup

### 2.3 Interrupt System

#### 2.3.1 Critical Constraint
**INTERRUPTS CAN ONLY BE CHECKED DURING SYSTEM IDLE TIME**
- No interrupt processing during LVGL operations
- InterruptManager coordinates all interrupt sources
- Ordered evaluation: Triggers first, then Actions if no triggers active
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
**COORDINATED INTERRUPT SYSTEM WITH FUNCTION POINTERS**

The interrupt system uses InterruptManager to coordinate specialized handlers for different interrupt sources:

- **Common Structure**: Single Interrupt struct handles all interrupt types with effect-based execution
- **Function Pointer Architecture**: Static evaluation and execution function pointers with void* context
- **Specialized Processing**: PolledHandler (GPIO state monitoring) and QueuedHandler (button event processing)
- **Effect-Based Execution**: LOAD_PANEL, SET_THEME, SET_PREFERENCE, CUSTOM_FUNCTION effects
- **Coordinated Logic**: InterruptManager coordinates both handlers and eliminates redundant layers
- **Change-Based Evaluation**: POLLED interrupts fire only on state transitions, not continuously

**Key Components**:
1. **InterruptManager**: Coordinates PolledHandler and QueuedHandler during idle time
2. **PolledHandler**: Implements IHandler, manages POLLED interrupts (GPIO state monitoring)
3. **QueuedHandler**: Implements IHandler, manages QUEUED interrupts (button event processing)
4. **InterruptSource**: POLLED vs QUEUED evaluation types routed to appropriate handlers
5. **InterruptEffect**: Effect-based execution categories for simplified restoration logic

#### 2.3.3 POLLED Interrupts (replaces Triggers)

**POLLED Interrupt Architecture**:
POLLED interrupts monitor GPIO state transitions and execute effects exactly once per change:

**Change-Based Evaluation**:
- POLLED interrupts only activate when sensor states actually change
- Each interrupt action executes exactly once per state transition  
- No repeated execution for maintained states
- Eliminates performance issues from redundant operations

**Interrupt with POLLED Source**:
```cpp
enum class InterruptEffect {
    LOAD_PANEL,        // Load panel, participates in restoration (unless maintains)
    SET_THEME,         // Set theme, always maintains (doesn't participate in restoration blocking)
    SET_PREFERENCE,    // Set configuration value
    BUTTON_ACTION      // Execute panel-specific button function (short/long press)
};

struct Interrupt {
    const char* id;                                    // Static string for memory efficiency
    Priority priority;                                 // Processing priority
    InterruptSource source;                           // POLLED or QUEUED evaluation
    InterruptEffect effect;                           // LOAD_PANEL, SET_THEME, BUTTON_ACTION, etc.
    bool (*evaluationFunc)(void* context);           // Checks current state (POLLED) or events (QUEUED)
    void (*activateFunc)(void* context);             // Executes on interrupt activation
    void (*deactivateFunc)(void* context);           // Executes on interrupt deactivation (optional)
    void* sensorContext;                              // Sensor context for evaluation
    void* serviceContext;                             // Service context for execution (PanelManager, etc.)
    union {
        struct { 
            const char* panelName; 
            bool trackForRestore; 
            bool maintainWhenInactive;               // Theme interrupts maintain, panel interrupts don't
        } panel;
        struct { 
            Theme theme; 
            bool maintainWhenInactive;               // Always true for themes
        } theme;
        struct { 
            const char* key; 
            void* value; 
        } preference;
        struct {
            void (*shortPressFunc)(void* panelContext); // Panel's short press function
            void (*longPressFunc)(void* panelContext);  // Panel's long press function
            void* panelContext;                          // Current panel instance
        } buttonActions;
    } data;                                           // Effect-specific data union
    bool active;                                      // Current activation state
    bool previouslyActive;                           // For state change detection
    unsigned long lastEvaluation;                    // Performance tracking
};
```
- State tracking handled internally by sensors using BaseSensor pattern
- Context parameter provides sensor access without heap allocation
- Effect-based execution enables simplified restoration logic

**Memory Safety Requirements**:
- All callback functions must be static (no lambda captures)
- Context passed via void* parameter to eliminate heap allocation
- Static string literals for IDs to prevent string fragmentation
- Function pointer arrays over std::vector to avoid dynamic allocation

**Panel Restoration (Simplified)**:
- PanelManager tracks the last user-driven panel (loaded by user action)
- When ALL panel-loading interrupts become inactive, system returns to the tracked user-driven panel
- Only LOAD_PANEL effect interrupts participate in restoration logic
- SET_THEME effects never block or affect panel restoration
- Restoration occurs when no LOAD_PANEL interrupts are active

**Coordinated Processing Implementation**:
- **InterruptManager**: Coordinates PolledHandler and QueuedHandler
- **PolledHandler**: Manages POLLED interrupts for GPIO state monitoring
- **QueuedHandler**: Manages QUEUED interrupts for button event processing
- **Coordinated Processing**:
  1. **Handler Evaluation**: Each handler evaluates its interrupts independently
  2. **Priority Coordination**: InterruptManager compares highest priority interrupt from each handler
  3. **Effect-Based Execution**: InterruptManager executes based on interrupt effect type
  4. **Simplified Restoration**: Only panel-loading effects affect restoration
- **Single Evaluation Rule**: Each handler evaluates its interrupts exactly once per processing cycle
- **Cross-Handler Priority**: Highest priority interrupt across both handlers gets processed
- **Effect-Based Logic**: Restoration based on interrupt effects rather than complex priority blocking

**POLLED Interrupt Registration Examples** (COORDINATED PATTERN):
```cpp
// POLLED panel-loading interrupt using KeyPresentSensor - routed to PolledHandler
interruptManager.RegisterInterrupt({
    .id = "key_present",
    .priority = Priority::CRITICAL,
    .source = InterruptSource::POLLED,
    .effect = InterruptEffect::LOAD_PANEL,
    .evaluationFunc = IsKeyPresent,              // Checks current key state (not change)
    .activateFunc = LoadKeyPanel,                // Executes when key becomes present
    .deactivateFunc = nullptr,                   // No deactivation needed
    .sensorContext = keyPresentSensor,
    .serviceContext = panelManager,
    .data = { .panel = { "KEY", true, false } }  // Track restore, don't maintain when inactive
});

// POLLED theme-setting interrupt (doesn't affect restoration) - routed to PolledHandler
interruptManager.RegisterInterrupt({
    .id = "lights_changed",
    .priority = Priority::NORMAL,
    .source = InterruptSource::POLLED,
    .effect = InterruptEffect::SET_THEME,
    .evaluationFunc = AreLightsOn,               // Checks current light state
    .activateFunc = SetNightTheme,               // Executes when lights turn on
    .deactivateFunc = nullptr,                   // No deactivation (other interrupt handles day)
    .sensorContext = lightsSensor,
    .serviceContext = styleManager,
    .data = { .theme = { Theme::NIGHT, true } }  // Always maintain theme when inactive
});
```

**Registered POLLED Interrupts**:
- **key_present**: POLLED panel-loading interrupt using KeyPresentSensor (GPIO 25)
- **key_not_present**: POLLED panel-loading interrupt using KeyNotPresentSensor (GPIO 26)
- **lock_active**: POLLED panel-loading interrupt using LockSensor for lock engaged state
- **lights_changed**: POLLED theme-setting interrupt using LightsSensor (theme change only)
- **error_occurred**: POLLED panel-loading interrupt using ErrorManager state changes

**Sensor Change Detection Requirements**:
All sensors implement change detection through the BaseSensor base class:

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

**Implementation Notes**:
- BaseSensor class provides change detection template
- All trigger sensors inherit from BaseSensor
- Independent initialization for each sensor instance

**Sensor Implementation Pattern**:
- **State Tracking**: Each sensor tracks its previous state internally
- **Change Detection**: Sensors provide HasStateChanged() methods
- **Initialization**: Sensors set baseline state during Init() without triggering
- **Consistency**: All sensors follow the same change detection pattern

**Sensor Ownership and Resource Management**:

**CRITICAL ARCHITECTURE CONSTRAINT: PROPER FACTORY PATTERN WITH SINGLE SENSOR OWNERSHIP**
- **Handler Sensor Creation**: TriggerHandler and ActionHandler create and own their respective sensors internally
- **Panel Sensor Creation**: Data panels (OilPanel) create their own data sensors internally
- **Display-Only Trigger Panels**: KeyPanel, LockPanel create components but NO sensors
- **GPIO Resource Safety**: Each GPIO pin has exactly ONE sensor instance to prevent conflicts

**Sensor Creation and Ownership Requirements**:
1. **Build-Time Verification**: System must prevent multiple sensor instances for same GPIO
2. **Resource Conflict Detection**: GPIO interrupt attachment conflicts must be avoided
3. **Trigger Panel Restrictions**: Trigger panels forbidden from creating sensor instances
4. **Specialized Handler Ownership**: PolledHandler owns GPIO sensors, QueuedHandler owns button sensor
5. **Panel Ownership**: Data panels create and own their data sensors
6. **Cleanup Requirements**: Sensors with GPIO interrupts must implement proper destructors

**Sensor Resource Management Implementation**:
- **Destructors**: KeyPresentSensor and KeyNotPresentSensor implement destructors with DetachInterrupt()
- **DetachInterrupt**: Proper GPIO cleanup in sensor destructors
- **Safe Cleanup**: Null pointer checks prevent crashes during cleanup
- **No Resource Leaks**: Sensors properly release GPIO resources on destruction

**Panel Implementation**:
- **Trigger Panels**: KeyPanel, LockPanel are display-only - create components, use direct GPIO reads for initial state
- **Data Panels**: OilPanel creates own OilPressure/Temperature sensors and components
- **State Display**: Trigger panels read GPIO state directly via gpioProvider_->DigitalRead()
- **No Trigger Panel Sensors**: No sensor instances created in trigger panels

**Factory Pattern Architecture**:
```
 Main
├── Creates ProviderFactory (implements IProviderFactory)
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

**Priority Levels**:
- **CRITICAL (0)**: Highest priority (errors, key security)
- **IMPORTANT (1)**: Security-related (lock status)
- **NORMAL (2)**: Standard operations (theme changes, button actions, preferences)

**Effect-Based Restoration Logic**:
- Only LOAD_PANEL effect interrupts participate in restoration logic
- SET_THEME effects never block or affect panel restoration
- SET_PREFERENCE effects never block or affect panel restoration
- CUSTOM_FUNCTION effects never block or affect panel restoration
- When no LOAD_PANEL interrupts are active, restore the user-driven panel
- Simplified logic eliminates complex priority-based blocking scenarios

**Coordinated Interrupt Processing**:
1. InterruptManager calls PolledHandler::Process() and QueuedHandler::Process()
2. Each handler evaluates its interrupts:
   - PolledHandler: Calls evaluationFunc() to detect GPIO state changes
   - QueuedHandler: Checks button event queue for pending actions
3. InterruptManager coordinates highest priority interrupt across both handlers
4. Effect-based execution based on interrupt effect type via InterruptManager
5. Track panel-loading interrupt activity across both handlers for simplified restoration
6. Return to user-driven panel when no LOAD_PANEL interrupts are active in either handler

**Change Detection Flow**:
1. **Sensor Read**: Each trigger evaluation reads current sensor state
2. **Change Detection**: Sensor compares current state to previous state
3. **Condition Check**: If change detected, check if target condition is met
4. **Single Execution**: If conditions met, execute action exactly once
5. **State Update**: Sensor updates its previous state for next evaluation

**Critical Architecture Constraints**:

**INDEPENDENT TRIGGERS** - All triggers are completely independent entities:
- Multiple triggers can be active simultaneously
- Triggers with same priority are processed in registration order
- Last trigger to activate "wins" for panel display
- Each trigger manages its own state independently

**TRIGGER INDEPENDENCE** - Key architectural principles:
1. No coupling between triggers (e.g., key_present and key_not_present are unrelated)
2. Multiple triggers can share the same priority level
3. When multiple same-priority triggers are active, all are processed
4. Panel restoration occurs only when ALL triggers become inactive

**RESTORATION LOGIC** - Critical requirement:
- Uses priority-based blocking: CRITICAL and IMPORTANT triggers block restoration
- NORMAL priority triggers (e.g., lights/theme) do NOT block restoration
- When restoration is blocked, the blocking trigger's action is executed instead
- Individual trigger deactivation does NOT immediately trigger restoration
- This prevents panel flickering and ensures proper panel precedence

**MULTI-TRIGGER SCENARIOS** - Change-based behaviors:

Scenario 1 - Blocking trigger prevents restoration:
1. Key inserted: key_present trigger fires once (loads key panel)
2. Lock engaged: lock_active trigger fires once (loads lock panel)
3. Key removed: key_present state changes but lock still active (lock blocks restoration)
4. Lock disengaged: lock_active state changes, no blocking triggers (user panel restores)

Scenario 2 - Non-blocking trigger allows restoration:
1. Key inserted: key_present trigger fires once (loads key panel)
2. Lights turned on: lights_state trigger fires once (changes theme only)
3. Key removed: key_present state changes, lights don't block (user panel restores)
4. Lights turned off: lights_state trigger fires once (changes theme back)

Scenario 3 - Rapid state changes:
1. Key inserted then immediately removed: Both triggers fire in sequence
2. Lock engaged then immediately disengaged: Single trigger fires twice
3. Lights rapidly toggled: Each state change fires trigger once
4. System handles all changes without repeated actions

**Performance Benefits**:
- Theme setting: Reduced from ~60/sec to ~2/sec (only on actual changes)
- Panel loading: Once per state transition instead of continuously
- CPU usage: Consistent regardless of maintained trigger states
- Memory usage: No fragmentation from repeated operations

**Key Sensor Architecture - Split Sensor Design**:
The key detection system implements **two completely independent sensor classes**, reflecting the hardware reality of separate GPIO detection mechanisms:

**Sensor Class Architecture**:
- **KeyPresentSensor**: Dedicated class managing GPIO 25 (KEY_PRESENT pin)
- **KeyNotPresentSensor**: Dedicated class managing GPIO 26 (KEY_NOT_PRESENT pin)
- **Independent Inheritance**: Each sensor inherits from `ISensor` and `BaseSensor` separately
- **Isolated State Management**: Each sensor maintains its own change detection state
- **Proper Cleanup**: Both sensors implement destructors with DetachInterrupt()

**Hardware Interface**:
- **GPIO 25 (KEY_PRESENT)**: KeyPresentSensor monitors for HIGH state indicating key presence
- **GPIO 26 (KEY_NOT_PRESENT)**: KeyNotPresentSensor monitors for HIGH state indicating key absence
- **GPIO State Logic**: Both pins LOW = no detection active, either pin HIGH = valid detection
- **Pull-down Configuration**: Both pins use INPUT_PULLDOWN for stable readings

**Critical Architecture Principle**: 
**SENSOR INDEPENDENCE REQUIREMENT** - KeyPresentSensor and KeyNotPresentSensor are completely separate classes with no shared state, eliminating initialization conflicts and ensuring proper change detection for each GPIO pin independently.

**Split Sensor Implementation Requirements**:
1. **Independent Classes**: Each GPIO must have dedicated sensor class
2. **Separate Initialization**: Each sensor has its own initialized_ flag
3. **Individual Change Detection**: No shared state between sensor change tracking
4. **Resource Isolation**: GPIO cleanup handled independently per sensor
5. **Architectural Alignment**: Split design matches independent trigger system

**Sensor Implementation Pattern**:
```cpp
// Independent sensor classes
class KeyPresentSensor : public ISensor, public BaseSensor {
    bool HasStateChanged();      // Independent change detection
    bool GetKeyPresentState();   // GPIO 25 only
private:
    bool initialized_ = false;   // Independent initialization
};

class KeyNotPresentSensor : public ISensor, public BaseSensor {
    bool HasStateChanged();         // Independent change detection  
    bool GetKeyNotPresentState();   // GPIO 26 only
private:
    bool initialized_ = false;      // Independent initialization
};
```

**State Validation Rules**:
1. **Valid Detection States**: 
   - Both sensors inactive: No key detection active
   - KeyPresentSensor active: Key detected as present
   - KeyNotPresentSensor active: Key detected as not present

2. **Conflict Resolution**: 
   - Both sensors active simultaneously: Hardware fault condition
   - System prioritizes most recently activated sensor
   - Logging captures conflicting states for diagnostics

**Independent Trigger Implementation**:
Each sensor integrates with completely separate triggers:
- **key_present trigger**: Uses KeyPresentSensor change detection
- **key_not_present trigger**: Uses KeyNotPresentSensor change detection
- **Evaluation Pattern**: `sensor->HasStateChanged() && sensor->GetCurrentState()`
- **Execution**: Each trigger loads KeyPanel with appropriate state
- **Priority**: Both triggers have CRITICAL priority for immediate response

#### 2.3.4 QUEUED Interrupts (replaces Actions)

**QUEUED Interrupt Architecture**:
QUEUED interrupts handle button events from a message queue and execute panel-specific behaviors:

**Interrupt with QUEUED Source**:
```cpp
struct Interrupt {
    const char* id;                                    // Static string for memory efficiency
    Priority priority;                                 // Processing priority
    InterruptSource source;                           // QUEUED for button event processing
    InterruptEffect effect;                           // LOAD_PANEL, SET_PREFERENCE, BUTTON_ACTION
    bool (*evaluationFunc)(void* context);           // Function pointer - no heap allocation
    void (*executionFunc)(void* context);            // Function pointer - no heap allocation
    void* context;                                    // Panel or handler context
    union { /* effect-specific data */ } data;       // Union for memory efficiency
};
```

**QueuedHandler Implementation**:
- Implements IHandler interface
- Manages list of QUEUED interrupts only
- Owns ActionButtonSensor for GPIO 32 monitoring
- Processes QUEUED interrupts by checking **single latest button event** (not a queue)
- **Single Event Policy**: Maintains only the most recent button event, automatically discarding any previous unprocessed events
- Marks available actions for InterruptManager coordination
- Clears action state after execution

```cpp
class QueuedHandler {
private:
    std::optional<ButtonEvent> latestButtonEvent_;  // Only latest event, replaces previous
    
public:
    void QueueButtonEvent(ButtonEvent event);       // Replaces any existing event
    bool HasPendingButtonEvent() const;
    void ClearButtonEvent();                        // Clears after processing
};
```

**Input Detection**:
The QueuedHandler monitors button hardware and updates action availability:
- Monitors button state continuously via ActionButtonSensor
- Detects press types (short/long) with debouncing
- Queues button events for QUEUED interrupt evaluation
- Event queue processed by QUEUED interrupt evaluation functions

**QUEUED Interrupt Registration Examples - Universal Panel Functions**:
```cpp
// Universal short press button action - routed to QueuedHandler
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

// Universal long press button action - routed to QueuedHandler
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

**IActionService Interface**:
All panels must implement `IActionService` which provides:
- `GetShortPressAction()`: Returns Action struct with function pointer for short press handling
- `GetLongPressAction()`: Returns Action struct with function pointer for long press handling
- Panel-specific logic executed when actions are triggered

**Hardware Configuration**:
- Single push button on GPIO 32
- Rising edge detection with debouncing
- 3.3V pull-up configuration

**Input Timing**:
- **Short Press**: 50ms - 2000ms
- **Long Press**: 2000ms - 5000ms
- **Maximum Press**: 5100ms (timeout)
- **Debounce Time**: 50ms

**Panel-Specific Action Behaviors**:
- **Oil Panel**: Short press: No action, Long press: Load config panel
- **Splash Panel**: Short press: Skip animation, Long press: Load config panel
- **Error Panel**: Short press: Cycle through errors, Long press: Clear all errors
- **Config Panel**: Short press: Cycle through options, Long press: Select/edit option

**QUEUED Interrupt Processing Rules**:
- QUEUED interrupts processed by priority alongside POLLED interrupts
- Highest priority interrupt (regardless of source) gets processed first
- Button events queued and processed when their interrupt evaluates true
- Panel context validated before executing panel-specific custom functions

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
- **TriggerHandler**: Implements IHandler, manages trigger structs with phase-based processing
- **ActionHandler**: Implements IHandler, manages action structs with button monitoring
- **Registration**: Type-specific methods (RegisterTrigger/RegisterAction)
- **Polymorphic Processing**: InterruptManager::Process() calls both handlers via IHandler*

#### 2.3.7 Registration Pattern

Triggers and actions are registered at startup as struct instances:

```
// Register triggers with TriggerHandler
triggerHandler.RegisterTrigger({
    id: "key_present",
    priority: CRITICAL,
    evaluationFunc: [gpio]() { return gpio->Read(KEY_PIN); },
    executionFunc: [panel]() { panel->LoadPanel("key"); },
    state: INIT
});

// Register actions with ActionHandler
actionHandler.RegisterAction({
    id: "short_press",
    priority: NORMAL,
    evaluationFunc: [actionHandler]() { return actionHandler->HasPendingAction(SHORT_PRESS); },
    executionFunc: [panel]() { panel->ExecuteShortPress(); },
    actionType: SHORT_PRESS
});
```

**Registered Interrupts**:
- **Triggers**: key_present, key_not_present, lock_state, lights_state, error_occurred
- **Actions**: short_press_action, long_press_action

This approach provides:
- Single handler per interrupt type (triggers vs actions)
- Direct struct registration without intermediate handler objects
- Clear separation between trigger and action processing logic

#### 2.3.8 Evaluation Process

**Polymorphic Handler Processing**:
InterruptManager coordinates both handlers through the unified IHandler interface:

```cpp
void InterruptManager::CheckAllInterrupts() {
    triggerHandler_->Process();  // IHandler* - processes trigger structs
    actionHandler_->Process();   // IHandler* - processes action structs  
}
```

**Handler-Specific Processing**:
1. **TriggerHandler::Process()** (evaluated first):
   - Iterates trigger structs by priority (highest first)
   - Calls hasChangedFunc() on each trigger to detect state changes (once per cycle)
   - If changed, calls getCurrentStateFunc() to determine activation/deactivation
   - Executes trigger action for activation, or processes deactivation logic
   - First trigger with changes gets processed, others ignored
   - On deactivation: checks other triggers' current state for fallback
   - NORMAL priority triggers never block restoration
   
2. **ActionHandler::Process()** (evaluated second):
   - Only processes if no triggers are currently active
   - Checks action struct conditions via evaluationFunc()
   - Executes first available action via executionFunc()
   - Clears action state after execution

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
- **Consistent Implementation**: All triggers follow identical change-detection pattern
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
// Check if any blocking triggers are active
bool HasBlockingTriggerActive() const {
    for (const auto& trigger : triggers_) {
        if (trigger.state == ACTIVE &&
            (trigger.priority == CRITICAL || trigger.priority == IMPORTANT)) {
            return true;
        }
    }
    return false;
}
```

**Restoration Decision Logic**:
When a panel-loading trigger deactivates:
1. Check if any blocking triggers (CRITICAL/IMPORTANT) are active
2. If yes: Execute the highest priority blocking trigger's action
3. If no: Restore the user-driven panel
4. NORMAL priority triggers (lights) never affect this decision

**Example Flow**:
- Key trigger deactivates while lock trigger is active
- System detects lock (IMPORTANT) blocks restoration
- Lock trigger's execution function is called
- Lock panel is loaded instead of restoring user panel

### 2.4 Sensor System

#### 2.4.1 Sensor Architecture

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

#### 2.4.2 Sensor Types and Implementations

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
  - Owned by ActionHandler

**Analog Sensors**: ADC-based continuous measurements
- **OilPressureSensor**: Pressure monitoring with unit conversion
- **OilTemperatureSensor**: Temperature monitoring

#### 2.4.2 Unit Management
- Units of measure injected via SetTargetUnit() method
- Sensors implement GetSupportedUnits() for validation
- Sensors report readings in requested units via ConvertReading()
- Panels control visual scales and ranges independently
- OilPressureSensor supports Bar, PSI, kPa units

### 2.5 Theme System

#### 2.5.1 Available Themes
- **Day Theme**: White/bright colors for daylight visibility
- **Night Theme**: Red/dark colors for night vision

#### 2.5.2 Theme Switching
- Automatic based on light sensor
- Instant switching without panel reload
- Theme persists across panel switches

## 3. Non-Functional Requirements

### 3.1 Memory Management Requirements

#### 3.1.1 ESP32 Memory Constraints
**Critical Limitation**: ESP32-WROOM-32 has only ~300KB available RAM
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
  - **TriggerHandler**: Change-based trigger evaluation with phase processing
  - **ActionHandler**: Button input management with panel action routing
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
- **Version 1.1**: Updated after codebase review - corrected input timing, added ActionManager, updated ErrorManager implementation status, corrected panel names and configurations
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
- **Version 1.4**: TriggerHandler architecture redesign - separation of concerns
  - Separated change detection (`hasChangedFunc`) from state retrieval (`getCurrentStateFunc`)
  - Implemented phase-based processing to prevent multiple evaluation corruption
  - Added proper deactivation handling with priority-based restoration logic
  - Enforced single evaluation rule: each trigger checked exactly once per cycle
  - Updated trigger struct to support independent change and state functions
  - Added comprehensive restoration logic for multi-trigger scenarios
- **Version 1.5**: Sensor ownership and resource management clarification
  - Added critical architecture constraint for single sensor ownership
  - Specified TriggerHandler as sole owner of trigger sensors
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