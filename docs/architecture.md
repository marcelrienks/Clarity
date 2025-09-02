# Architecture

## Overview

MVP (Model-View-Presenter) pattern for ESP32 automotive gauges with layered architecture, interface-based design, and v4.0 Trigger/Action interrupt architecture.

## Visual Diagrams

For detailed architectural diagrams, see:
- **[Architecture Overview](diagrams/architecture-overview.md)** - Complete component relationships and dependencies
- **[Application Flow](diagrams/application-flow.md)** - Startup sequence, runtime processing, and interrupt handling flows

## Pattern Structure

```
InterruptManager → TriggerHandler → GPIO Sensors → GPIO
                ↘ ActionHandler → Action Sensor ↗          
                                    
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
- Interrupt-driven panels (Key, Lock) are display-only
- Data panels (Oil) create their own data sensors
- Implement IActionService for action handling

### Components (Views)  
- Render UI elements (gauges, indicators)
- No business logic
- LVGL object management

### Sensors (Models)
- Abstract GPIO/ADC inputs with change detection
- Single ownership model - each GPIO has exactly one sensor
- Split architecture for independent concerns (e.g., KeyPresentSensor, KeyNotPresentSensor)
- Implement proper destructors with GPIO cleanup

## v4.0 Trigger/Action Interrupt Architecture

### Specialized Handler Design
The interrupt system uses InterruptManager to coordinate two specialized handlers based on interrupt behavior:

```
InterruptManager: Central coordination of interrupt processing
├── TriggerHandler: Manages state-based Triggers (GPIO state monitoring)
└── ActionHandler: Manages event-based Actions (action event processing)
```

### Interrupt Evaluation vs Execution Model
The system separates interrupt evaluation (checking for events/state changes) from interrupt execution:

- **Action Evaluation**: Happens on EVERY main loop iteration to detect action events
- **Trigger Evaluation**: Only happens during UI IDLE state  
- **All Execution**: Only happens during UI IDLE state, with Triggers processed before Actions
- **Priority System**: Triggers use CRITICAL > IMPORTANT > NORMAL priorities with override logic

### v4.0 Interrupt Data Structures

#### Trigger Structure (State-Based)
Triggers handle GPIO state changes with dual activation/deactivation functions:

```cpp
// Located in include/types.h
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

struct Trigger {
    const char* id;                          // Unique identifier
    Priority priority;                       // Execution priority (Critical > Important > Normal)
    TriggerType type;                        // Type classification (Panel, Style, Function)
    
    // Dual-state functions - no context needed (singleton calls)
    void (*activateFunc)();                  // Execute when trigger should activate
    void (*deactivateFunc)();                // Execute when trigger should deactivate
    
    // State association
    BaseSensor* sensor;                      // Associated sensor (1:1)
    
    // Override behavior
    bool canBeOverriddenOnActivate;          // Can other triggers override?
    bool isActive;                           // Currently active (true after activate, false after deactivate)
    
    // Execution methods
    void ExecuteActivate() {
        if (activateFunc) {
            activateFunc();
            isActive = true;  // Set active AFTER executing activate function
        }
    }
    
    void ExecuteDeactivate() {
        if (deactivateFunc) {
            deactivateFunc();
            isActive = false;  // Set inactive AFTER executing deactivate function
        }
    }
};
```

#### Action Structure (Event-Based)
Actions handle action events with single execution:

```cpp
// Located in include/types.h
enum class ActionPress : uint8_t {
    SHORT = 0,
    LONG = 1
};

struct Action {
    const char* id;                          // Unique identifier
    
    // Event function - no context needed (singleton calls)
    void (*executeFunc)();                   // Execute when action triggered
    
    // Event state
    bool hasTriggered;                       // Action pending execution
    ActionPress pressType;                   // SHORT or LONG press
    
    // No priority - Actions process in order
    // No sensor - managed by ActionHandler
    // No override logic - Actions cannot block
    
    // Execution method
    void Execute() {
        if (executeFunc && hasTriggered) {
            executeFunc();
            hasTriggered = false;
        }
    }
};

// Memory Usage: Triggers ~16 bytes, Actions ~8 bytes
// Total system: ~80 bytes for 5 triggers + ~16 bytes for 2 actions = ~96 bytes total
```

### Triggers vs Actions: Fundamental Differences

While both Triggers and Actions are interrupt mechanisms with function pointers set during registration, they operate with fundamentally different behaviors and processing models:

#### **Triggers (State-Based Interrupts)**

**Core Behavior**:
- **State Change Detection**: Triggers monitor GPIO sensor states and fire only when state transitions occur (HIGH ↔ LOW)
- **Previous State Tracking**: Sensors must implement `HasStateChanged()` using BaseSensor's `DetectChange()` template to track previous state
- **Dual Functions**: Each Trigger has both `activateFunc()` (executed on HIGH transition) and `deactivateFunc()` (executed on LOW transition)
- **Polled During Idle**: Trigger evaluation only happens during UI idle time to prevent LVGL conflicts

**Processing Flow**:
1. **Idle Check**: TriggerHandler::Process() only runs when UI state == IDLE
2. **State Polling**: For each Trigger (sorted by priority), check `sensor->HasStateChanged()`
3. **Change Detection**: If state changed, determine if transition is HIGH (activate) or LOW (deactivate)
4. **Activation Logic**: 
   - If no higher-priority active triggers exist: Execute `activateFunc()`, set `isActive = true`
   - If higher-priority active trigger exists: Set `isActive = true` but do NOT execute function
5. **Deactivation Logic**:
   - Execute `deactivateFunc()`, set `isActive = false`
   - Check for other active triggers of same type
   - If found, execute highest priority same-type trigger's `activateFunc()`

**State Management Requirements**:
```cpp
// Sensors must implement change detection via BaseSensor inheritance
class KeyPresentSensor : public BaseSensor {
private:
    bool previousState_ = false;  // Track previous state for change detection
    
public:
    bool HasStateChanged() {
        bool currentState = std::get<bool>(GetReading());
        return DetectChange(currentState, previousState_);  // BaseSensor template
    }
};
```

**Override Behavior (canBeOverriddenOnActivate Flag)**:

The `canBeOverriddenOnActivate` flag determines whether a Trigger's activation can be blocked by higher-priority active Triggers:

**Flag Values**:
- **`canBeOverriddenOnActivate = false`**: This Trigger cannot be blocked during activation attempts
- **`canBeOverriddenOnActivate = true`**: This Trigger can be blocked by active higher-priority Triggers

**Override Logic Flow**:
1. **Activation Request**: When a Trigger attempts activation (sensor goes HIGH)
2. **Priority Check**: System searches for active Triggers with higher priority than the activating Trigger
3. **Override Decision**: 
   - If **blocking Trigger found**: The higher-priority active non-overridable Trigger's `activateFunc()` re-executes instead
   - If **no blocking Trigger**: The requested Trigger's `activateFunc()` executes normally

**Override Resolution Examples**:
```cpp
// CRITICAL priority Key Trigger (canBeOverriddenOnActivate = false)
{
    .id = "key_present",
    .priority = Priority::CRITICAL,
    .canBeOverriddenOnActivate = false,  // Cannot be overridden
    .activateFunc = []() { PanelManager::Instance().LoadPanel(PanelType::KEY); }
}

// NORMAL priority Lights Trigger (canBeOverriddenOnActivate = true)  
{
    .id = "lights_changed",
    .priority = Priority::NORMAL,
    .canBeOverriddenOnActivate = true,   // Can be overridden
    .activateFunc = []() { StyleManager::Instance().SetTheme(Theme::NIGHT); }
}

// Scenario: Key Trigger active, Lights Trigger attempts activation
// Result: Key Trigger (CRITICAL, non-overridable) blocks Lights activation
// Action: Key Trigger's activateFunc() re-executes, maintaining Key panel
```

**Registration Pattern**:
Critical system triggers (Key, Error) typically set `canBeOverriddenOnActivate = false` to ensure they cannot be blocked, while theme/preference triggers set `canBeOverriddenOnActivate = true` to allow higher-priority interruptions.

#### **Actions (Event-Based Interrupts)**

**Core Behavior**:
- **Event Detection**: Actions monitor for discrete events (short/long press durations) from action input
- **Queue-Based Processing**: Events are detected and queued immediately, but execution deferred to idle time
- **Single Function**: Each Action has only `executeFunc()` - no dual state handling
- **Continuous Evaluation**: Action evaluation happens every main loop iteration for responsiveness

**Processing Flow**:
1. **Continuous Evaluation**: ActionHandler evaluates action input state every main loop iteration
2. **Event Detection**: Detect press duration (50ms-2000ms = SHORT, 2000ms-5000ms = LONG)
3. **Queue Action**: If valid press detected, set `hasTriggered = true` and `pressType`
4. **Idle Execution**: During UI idle, execute all queued Actions in registration order
5. **Queue Clearing**: After execution, set `hasTriggered = false`

**Timing Detection Requirements**:
```cpp
// ActionSensor implements timing logic for press duration
class ActionSensor : public BaseSensor {
private:
    uint32_t pressStartTime_ = 0;
    bool buttonPressed_ = false;
    
public:
    uint32_t GetPressDuration() {
        // Returns duration of completed press event
        return pressEndTime_ - pressStartTime_;
    }
    
    bool HasStateChanged() {
        // Detects press start/end events for duration calculation
        bool currentState = digitalRead(gpio_);
        return DetectChange(currentState, previousState_);
    }
};
```

**Queue Management**:
- **No Priority System**: Actions execute in registration order, not priority-based
- **No Override Logic**: Actions cannot block each other - all queued Actions execute
- **Event Flags**: Each Action maintains `hasTriggered` flag for pending execution

#### **Key Behavioral Comparisons**

| Aspect | Triggers (State-Based) | Actions (Event-Based) |
|--------|------------------------|----------------------|
| **Evaluation Timing** | Only during UI IDLE | Every main loop iteration |
| **Execution Timing** | Only during UI IDLE | Only during UI IDLE |
| **State Tracking** | Requires previous state tracking | No state persistence needed |
| **Function Count** | Two (activate/deactivate) | One (execute) |
| **Priority System** | Yes (CRITICAL > IMPORTANT > NORMAL) | No (registration order) |
| **Override Logic** | Yes (canBeOverriddenOnActivate) | No (all queued Actions execute) |
| **Sensor Association** | 1:1 with GPIO sensor | Managed by ActionHandler |
| **Processing Model** | Immediate state-based execution | Queue-then-execute model |

#### **Shared Characteristics**

Both interrupt types share these common elements:
- **Function Pointers**: Set during registration with static function addresses
- **No Context Parameters**: Direct singleton manager calls eliminate context overhead
- **Memory Efficient**: Static structures with minimal heap usage
- **IDLE Execution**: Both execute only during UI idle state for LVGL compatibility
- **Change Detection**: Both rely on change detection to trigger processing

#### Critical Memory Constraint
**ESP32 Memory Limitation**: The ESP32-WROOM-32 has 320KB total RAM, with ~250KB available after system overhead and OTA partitioning. Using `std::function` with lambda captures causes:
- Heap fragmentation leading to crashes
- Memory overhead of ~3KB per std::function object
- Unstable system behavior during LVGL operations

**v4.0 Solution**: Direct singleton calls without context parameters eliminate heap allocations and reduce memory footprint to ~96 bytes total.

### v4.0 Interrupt Processing System
The system processes interrupts through specialized handlers based on behavior:
- **TriggerHandler**: Manages GPIO state changes with priority-based override logic
- **ActionHandler**: Processes action events with timing detection
- **InterruptManager**: Coordinates both handlers with smart restoration logic

#### Priority and Type-Based Trigger System
**Trigger Activation Logic**: Triggers are managed by priority and type:

1. **Trigger State Change**: GPIO sensor detects state transition to HIGH (activation)
2. **Priority Check**: Find any active triggers with higher priority
3. **Activation Decision**: 
   - If no higher-priority active triggers: Execute `activateFunc()`, set `isActive = true`
   - If higher-priority trigger active: Set `isActive = true` only (no function execution)

**Trigger Deactivation Logic**: Intelligent handling based on trigger type:

1. **Trigger State Change**: GPIO sensor detects state transition to LOW (deactivation)
2. **Execute Deactivation**: Run `deactivateFunc()`, set `isActive = false`
3. **Same-Type Check**: Find other active triggers with same `TriggerType`
4. **Type-Based Restoration**: Execute highest-priority same-type trigger's `activateFunc()`

```cpp
void TriggerHandler::HandleActivation(Trigger& trigger) {
    // Check for higher priority active triggers
    Trigger* higherPriorityTrigger = FindHigherPriorityActiveTrigger(trigger);
    
    if (!higherPriorityTrigger) {
        trigger.ExecuteActivate();  // Execute and set active
    } else {
        trigger.isActive = true;     // Only mark active, don't execute
    }
}

void TriggerHandler::HandleDeactivation(Trigger& trigger) {
    trigger.ExecuteDeactivate();  // Execute and set inactive
    
    // Find highest priority active trigger of same type
    Trigger* sameTypeTrigger = FindHighestPrioritySameTypeTrigger(trigger.type);
    if (sameTypeTrigger) {
        sameTypeTrigger->ExecuteActivate();  // Re-activate same-type trigger
    }
}
```

**Example Scenario**: Lock and Key triggers (both Panel type):
- Both triggers active, Key panel showing (higher priority)
- Key trigger deactivates
- System finds Lock trigger (same Panel type, still active)
- Executes Lock trigger's `activateFunc()` to show Lock panel
- Ensures correct panel is displayed based on active triggers

#### Smart Panel Restoration
**PanelManager Restoration**: Tracks last user-driven panel for intelligent restoration:

```cpp
class PanelManager {
    PanelType lastUserPanel_;  // Last panel before trigger activation
    
    void CheckRestoration() {
        // Find highest priority active trigger
        for (const auto& trigger : GetSystemTriggers()) {
            if (trigger.isActive && !trigger.canBeOverriddenOnActivate) {
                trigger.ExecuteActivate();  // Re-load trigger's panel
                return;
            }
        }
        
        // No blocking triggers - restore user panel
        LoadPanel(lastUserPanel_);
    }
};
```

#### Action Event Processing
**ActionHandler Timing**: Detects press duration for appropriate action:
- **Short Press**: 50ms - 2000ms duration
- **Long Press**: 2000ms - 5000ms duration
- **Continuous Evaluation**: Always checks action state for responsiveness
- **Idle Execution**: Actions execute only during UI idle

**v4.0 Benefits**:
- **Clear Separation**: Triggers (state-based) vs Actions (event-based)
- **Smart Override Logic**: Non-overridable triggers can block others
- **Memory Efficient**: ~96 bytes total vs previous 203+ bytes
- **Maintainable**: Dual functions group related logic together
- **Testable**: Clear interfaces for unit testing

### Sensor Architecture

#### Split Sensor Design (Critical Requirement)
Each GPIO pin must have exactly one dedicated sensor class:
- **KeyPresentSensor** (GPIO 25): Independent class for key present detection
- **KeyNotPresentSensor** (GPIO 26): Independent class for key not present detection
- **LockSensor** (GPIO 27): Lock status monitoring
- **LightsSensor** (GPIO 33): Day/night detection (theme only)
- **ActionSensor** (GPIO 32): Action input with debouncing

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

**v4.0 Ownership Model**: Each GPIO has exactly one sensor instance to prevent resource conflicts:
- TriggerHandler owns all GPIO sensors (Key, Lock, Lights)
- ActionHandler owns ActionSensor
- Panels are display-only and must not create sensors
- No sensor duplication across components

## Managers

- **InterruptManager**: Creates and coordinates TriggerHandler and ActionHandler with v4.0 architecture
- **PanelManager**: Creates panels on demand, manages switching, lifecycle, and smart restoration to last user panel
- **PreferenceManager**: Persistent settings
- **StyleManager**: LVGL themes (Day/Night) controlled by LightsSensor trigger
- **ErrorManager**: Error collection with error trigger integration (CRITICAL priority)

## v4.0 Interrupt Processing Flow

### Main Loop Processing Model
The v4.0 architecture follows a precise sequence with Trigger/Action separation:

**Exact Main Loop Flow**:
1. **Main Loop Start**: Begin new iteration
2. **Main: LVGL Tasks**: Process UI updates and rendering  
3. **InterruptManager: Evaluate Actions**: Always check action events
4. **InterruptManager: Queue Actions**: Queue action events if detected
5. **InterruptManager: If Idle**: Check if UI is in idle state
   - **If NOT Idle**: Skip to step 8 (ensures action detection continues)
   - **If Idle**: Continue to step 6
6. **InterruptManager: Process Triggers**: Evaluate and execute state-based triggers with priority override
7. **InterruptManager: Execute Actions**: Process queued action events
8. **Main: Loop End**: Complete iteration, return to step 1

**v4.0 Benefits**:
- **Action responsiveness**: Actions always evaluated regardless of UI state
- **Smart priorities**: Triggers use CRITICAL > IMPORTANT > NORMAL with override logic
- **Clean separation**: State-based vs event-based interrupts handled appropriately

### Handler Processing Model
1. **InterruptManager::Process()** (v4.0 coordination)
   - Always evaluate Actions for action events
   - If UI is IDLE:
     - Call TriggerHandler::Process() for GPIO state evaluation
     - Execute Triggers with priority-based override logic
     - Execute pending Actions (action events)
   - Handle smart restoration when triggers deactivate

2. **TriggerHandler::Process()** (GPIO states - IDLE only)
   - Evaluate GPIO sensors for state changes
   - Execute activate/deactivate functions based on state
   - Apply priority-based blocking logic

3. **ActionHandler::Process()** (action events)
   - **Evaluation Phase** (every loop): Detect action press durations
   - **Execution Phase** (IDLE only): Execute pending actions
   - **Timing Detection**: 50ms-2000ms short, 2000ms-5000ms long

### v4.0 Priority System
- **CRITICAL (2)**: Errors, key security - cannot be overridden
- **IMPORTANT (1)**: Lock status - cannot be overridden
- **NORMAL (0)**: Lights/theme changes - can be overridden

### v4.0 Smart Restoration
- **Trigger Deactivation**: Each trigger's deactivateFunc can call CheckRestoration()
- **Last User Panel**: PanelManager tracks last user-driven panel
- **Priority Resolution**: Active non-overridable triggers re-execute on restoration
- **Clean Fallback**: Returns to last user panel when no blocking triggers active
- **No Context Needed**: Direct singleton calls eliminate context parameters

## Available Panels

### Panel Types and Action Behaviors

- **Splash Panel**: Startup animation with user control
  - Short Press: Skip animation, load default panel immediately
  - Long Press: Load config panel
  - Auto-transition: After animation completion
  
- **Oil Panel** (OemOilPanel): Primary monitoring display
  - Dual gauge monitoring (pressure/temperature) - creates own data sensors
  - Short Press: No action (reserved for future functionality)
  - Long Press: Load config panel
  - Continuous: Animate pressure & temperature gauges
  
- **Key Panel**: Security key status indicator (Display-only)
  - Visual indication: Green icon (key present) / Red icon (key not present)
  - No sensor creation - receives state from TriggerHandler
  - Short Press: No action (status display only)
  - Long Press: Load config panel
  
- **Lock Panel**: Vehicle security status (Display-only)
  - Visual indication of lock engaged/disengaged state
  - No sensor creation - receives state from TriggerHandler
  - Short Press: No action (status display only)
  - Long Press: Load config panel
  
- **Error Panel**: System error management with navigation
  - Scrollable error list with severity color-coding
  - Error cycling and acknowledgment functionality
  - Short Press: Cycle through errors (navigate to next error)
  - Long Press: Clear all acknowledged errors
  - Auto-restoration: When all errors viewed/acknowledged
  
- **Config Panel**: System configuration with hierarchical navigation
  - Multi-level menu system with state persistence
  - Theme settings, default panel selection, preferences
  - Short Press: Navigate through current menu level options
  - Long Press: Select highlighted option / Enter submenu / Apply setting
  
### Config Panel State Machine Architecture

The Config Panel implements a hierarchical state machine for menu navigation:

```cpp
enum class ConfigState {
    MAIN_MENU,        // Top-level menu: Theme, Default Panel, Exit
    THEME_SUBMENU,    // Theme selection: Day, Night, Auto
    PANEL_SUBMENU,    // Default panel selection: Oil, Splash, etc.
    APPLYING_SETTING  // Transient state while applying changes
};

struct ConfigMenuOption {
    const char* displayName;     // Text displayed on screen
    ConfigState targetState;     // State when selected (NONE for actions)
    void (*actionFunc)(void* context); // Function to execute (null for submenus)
    bool requiresConfirmation;   // Whether to show confirmation dialog
};

class ConfigPanel {
private:
    ConfigState currentState_ = ConfigState::MAIN_MENU;
    int selectedOption_ = 0;     // Currently highlighted option
    int maxOptions_ = 0;         // Number of options in current menu
    
    // Main menu options
    std::array<ConfigMenuOption, 3> mainMenuOptions_ = {
        {"Theme Settings", ConfigState::THEME_SUBMENU, nullptr, false},
        {"Default Panel", ConfigState::PANEL_SUBMENU, nullptr, false},
        {"Exit", ConfigState::MAIN_MENU, &ConfigPanel::ExitToOilPanel, false}
    };
    
    // Theme submenu options
    std::array<ConfigMenuOption, 3> themeOptions_ = {
        {"Day Theme", ConfigState::MAIN_MENU, &ConfigPanel::SetDayTheme, true},
        {"Night Theme", ConfigState::MAIN_MENU, &ConfigPanel::SetNightTheme, true},
        {"Back", ConfigState::MAIN_MENU, nullptr, false}
    };
    
public:
    // IActionService implementation
    void (*GetShortPressFunction())(void* panelContext) override {
        return &ConfigPanel::HandleShortPress;
    }
    void (*GetLongPressFunction())(void* panelContext) override {
        return &ConfigPanel::HandleLongPress;
    }
    
    // Static callback functions for universal action system
    static void HandleShortPress(void* panelContext) {
        auto* panel = static_cast<ConfigPanel*>(panelContext);
        panel->NavigateToNextOption();
    }
    
    static void HandleLongPress(void* panelContext) {
        auto* panel = static_cast<ConfigPanel*>(panelContext);
        panel->SelectCurrentOption();
    }
    
    // Navigation and selection logic
    void NavigateToNextOption();     // Cycle through current menu options
    void SelectCurrentOption();      // Execute selected option or enter submenu
    void SetDayTheme(void* context); // Apply day theme via StyleManager
    void SetNightTheme(void* context); // Apply night theme via StyleManager
    void ExitToOilPanel(void* context); // Return to Oil panel
};
```

### Config Panel Integration Points
- **StyleManager**: Theme changes applied immediately and persisted
- **PreferenceManager**: Settings saved for persistence across reboots
- **PanelManager**: Default panel preference used during startup
- **Universal Action System**: Short/long press functions injected when panel loads

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
  - ActionSensor (GPIO 32) - User input sensor
- **Analog Sensors** (ADC):
  - Oil pressure sensor - Continuous pressure monitoring
  - Oil temperature sensor - Continuous temperature monitoring

## Error Panel State Management Architecture

### Error Cycling and Auto-Restoration System

The Error Panel manages error display, navigation, and automatic restoration:

```cpp
struct ErrorState {
    std::vector<ErrorInfo> activeErrors;    // Current error queue from ErrorManager
    int currentErrorIndex = 0;              // Currently displayed error
    std::set<size_t> viewedErrors;          // Errors that have been viewed
    bool allErrorsViewed = false;           // Flag for auto-restoration activation
};

class ErrorPanel {
private:
    ErrorState errorState_;
    
public:
    // IActionService implementation
    void (*GetShortPressFunction())(void* panelContext) override {
        return &ErrorPanel::HandleShortPress;
    }
    void (*GetLongPressFunction())(void* panelContext) override {
        return &ErrorPanel::HandleLongPress;
    }
    
    // Static callback functions for universal action system
    static void HandleShortPress(void* panelContext) {
        auto* panel = static_cast<ErrorPanel*>(panelContext);
        panel->CycleToNextError();
    }
    
    static void HandleLongPress(void* panelContext) {
        auto* panel = static_cast<ErrorPanel*>(panelContext);
        panel->ClearAllAcknowledgedErrors();
    }
    
    // Error navigation and management
    void CycleToNextError() {
        if (!errorState_.activeErrors.empty()) {
            // Mark current error as viewed
            errorState_.viewedErrors.insert(errorState_.currentErrorIndex);
            
            // Move to next error
            errorState_.currentErrorIndex = (errorState_.currentErrorIndex + 1) % errorState_.activeErrors.size();
            
            // Check if all errors have been viewed
            CheckAutoRestorationCondition();
            
            UpdateDisplay();
        }
    }
    
    void ClearAllAcknowledgedErrors() {
        // Clear all viewed errors from ErrorManager
        ErrorManager::Instance().ClearViewedErrors(errorState_.viewedErrors);
        
        // Update local state
        RefreshErrorState();
        
        // Initiate restoration if no errors remain
        if (errorState_.activeErrors.empty()) {
            InitiateAutoRestoration();
        }
    }
    
    void CheckAutoRestorationCondition() {
        // Auto-restore when all errors have been viewed
        if (errorState_.viewedErrors.size() >= errorState_.activeErrors.size()) {
            errorState_.allErrorsViewed = true;
            InitiateAutoRestoration();
        }
    }
    
    void InitiateAutoRestoration() {
        // Notify ErrorManager that errors have been processed
        ErrorManager::Instance().SetErrorPanelActive(false);
        // This will cause error_occurred interrupt to deactivate
        // Leading to automatic panel restoration via interrupt system
    }
};
```

### Error Panel Integration
- **ErrorManager**: Provides error queue and acknowledgment tracking
- **Auto-Restoration**: Initiated when all errors viewed or acknowledged
- **Interrupt Integration**: error_occurred interrupt deactivation enables restoration
- **Visual Feedback**: Color-coded errors by severity (Critical/Error/Warning)

## Debug Error System Architecture

### Development and Testing Support

The debug error system enables testing and development of the error handling system:

```cpp
// Debug error sensor for development (GPIO 34)
class DebugErrorSensor : public ISensor, public BaseSensor {
private:
    IGpioProvider* gpioProvider_;
    bool previousState_ = false;
    
public:
    DebugErrorSensor(IGpioProvider* gpio) : gpioProvider_(gpio) {}
    
    void Init() override {
        if (gpioProvider_) {
            gpioProvider_->SetPinMode(gpio_pins::DEBUG_ERROR, INPUT);
            // Note: GPIO 34 is input-only, requires external pull-down
        }
    }
    
    Reading GetReading() override {
        if (gpioProvider_) {
            return gpioProvider_->DigitalRead(gpio_pins::DEBUG_ERROR);
        }
        return false;
    }
    
    bool HasStateChanged() {
        bool currentState = std::get<bool>(GetReading());
        return DetectChange(currentState, previousState_);
    }
    
    bool GetDebugInterruptState() {
        return std::get<bool>(GetReading());
    }
};

// Debug error interrupt registration (development only)
#ifdef CLARITY_DEBUG
interruptManager.RegisterInterrupt({
    .id = "debug_error_trigger",
    .priority = Priority::CRITICAL,
    .source = InterruptSource::POLLED,
    .effect = InterruptEffect::CUSTOM_FUNCTION,
    .evaluationFunc = DebugErrorInterruptActive,
    .executionFunc = GenerateDebugError,
    .context = debugErrorSensor
});
#endif

// Static callback for debug error generation
static void GenerateDebugError(void* context) {
    auto* errorManager = static_cast<ErrorManager*>(context);
    if (errorManager) {
        errorManager->ReportError(ErrorLevel::ERROR, "DebugSystem", "Debug error activated via GPIO 34");
    }
}
```

### Debug System Integration
- **GPIO 34**: Input-only pin for debug error interrupt (development only)
- **Conditional Compilation**: Only active in CLARITY_DEBUG builds
- **Error Generation**: Creates test errors for system validation
- **Hardware Integration**: Requires external pull-down resistor

## Universal Action System Architecture

### Action Function Injection Pattern
The universal action system enables all panels to respond to action input through a coordinated injection mechanism with precise timing control:

**Action Hardware Configuration**:
- **GPIO**: Single action input on GPIO 32 with INPUT_PULLDOWN configuration
- **Timing**: Short press (50ms-2000ms), Long press (2000ms-5000ms)
- **Debouncing**: 50ms debounce time to prevent false triggers

```cpp
// IActionService interface - all panels must implement
class IActionService {
public:
    virtual ~IActionService() = default;
    virtual void (*GetShortPressFunction())(void* panelContext) = 0;
    virtual void (*GetLongPressFunction())(void* panelContext) = 0;
    virtual void* GetPanelContext() = 0;
};

// Universal action interrupt with dynamic function injection
struct UniversalActionInterrupt {
    const char* id;                                    // "universal_short_press" / "universal_long_press"
    InterruptSource source = InterruptSource::QUEUED;
    InterruptEffect effect = InterruptEffect::ACTION_EVENT;
    bool (*evaluationFunc)(void* context);           // HasShortPressEvent / HasLongPressEvent
    void (*activateFunc)(void* context);             // ExecutePanelFunction - calls current panel's function
    void* sensorContext;                              // ActionSensor
    void* serviceContext;                             // PanelManager
    
    // Runtime function injection from current panel
    void (*currentPanelFunction)(void* panelContext); // Injected when panels switch
    void* currentPanelContext;                        // Current panel instance
};
```

### Function Injection Process
1. **Panel Switch**: PanelManager loads new panel
2. **Function Extraction**: Get functions from panel via IActionService interface
3. **Interrupt Update**: Inject panel functions into universal action interrupts
4. **Context Update**: Update panel context for function execution
5. **Ready State**: Action interrupts now execute current panel's functions

### Static Callback Implementation
```cpp
class UniversalActionCallbacks {
public:
    // Universal execution functions that call injected panel functions
    static void ExecutePanelShortPress(void* context) {
        auto* panelManager = static_cast<PanelManager*>(context);
        if (panelManager) {
            auto* interrupt = panelManager->GetShortPressInterrupt();
            if (interrupt && interrupt->currentPanelFunction) {
                interrupt->currentPanelFunction(interrupt->currentPanelContext);
            }
        }
    }
    
    static void ExecutePanelLongPress(void* context) {
        auto* panelManager = static_cast<PanelManager*>(context);
        if (panelManager) {
            auto* interrupt = panelManager->GetLongPressInterrupt();
            if (interrupt && interrupt->currentPanelFunction) {
                interrupt->currentPanelFunction(interrupt->currentPanelContext);
            }
        }
    }
};
```

## Key Interfaces

### Factory Interfaces
- `IProviderFactory`: Factory interface for creating hardware providers with dependency injection support
- `IManagerFactory`: Factory interface for creating managers (future extension point)

### Provider Interfaces (Hardware Abstraction)
- `IGpioProvider`: Hardware GPIO abstraction for digital/analog I/O
- `IDisplayProvider`: LVGL display abstraction for screen operations
- `IDeviceProvider`: Hardware device driver abstraction for low-level device communication

### Component Interfaces (MVP Pattern)
- `IPanel`: Screen implementation interface with lifecycle management
  - `Init/Load/Update`: Standard panel lifecycle methods
  - Inherits from `IActionService` for universal action handling
- `IActionService`: Universal action function interface for all panels
  - `GetShortPressFunction()`: Returns panel's short press static function pointer
  - `GetLongPressFunction()`: Returns panel's long press static function pointer  
  - `GetPanelContext()`: Returns panel instance for function context
- `IComponent`: UI element rendering interface for LVGL components
- `ISensor`: Data acquisition interface with change detection support

### Service Interfaces
- `IHandler`: Unified interface for interrupt processing (POLLED and QUEUED interrupts)
- `IManager`: Base manager interface for service lifecycle management

### Utility Interfaces
- `ILogger`: Logging abstraction interface (future)
- `IPreferenceService`: Configuration persistence interface (future)

## Coordinated Interrupt Registration

### POLLED Panel Loading Registration (GPIO state monitoring)
```cpp
// Routed to PolledHandler by InterruptManager - Centralized restoration design
interruptManager.RegisterInterrupt({
    .id = "key_present",
    .priority = Priority::CRITICAL,
    .source = InterruptSource::POLLED,
    .effect = InterruptEffect::LOAD_PANEL,
    .evaluationFunc = KeyPresentChanged,
    .executionFunc = LoadKeyPanel,         // Single execution function
    .context = keyPresentSensor,
    .data = { .panel = { "KEY", true } }   // trackForRestore = true
});
// Note: Restoration handled centrally by InterruptManager when interrupt deactivates
```

### POLLED Theme Setting Registration
```cpp
// Routed to PolledHandler by InterruptManager - No restoration impact
interruptManager.RegisterInterrupt({
    .id = "lights_changed", 
    .priority = Priority::NORMAL,
    .source = InterruptSource::POLLED,
    .effect = InterruptEffect::SET_THEME,
    .evaluationFunc = LightsChanged,
    .executionFunc = SetThemeBasedOnLights,    // Single execution function
    .context = lightsSensor,
    .data = { .theme = Theme::NIGHT }          // Will be determined dynamically
});
```

### QUEUED Universal Panel Action Registration (every panel has these)
```cpp
// Universal short press - routed to QueuedHandler by InterruptManager
interruptManager.RegisterInterrupt({
    .id = "universal_short_press",
    .priority = Priority::NORMAL,
    .source = InterruptSource::QUEUED,
    .effect = InterruptEffect::ACTION_EVENT,
    .evaluationFunc = HasShortPressEvent,
    .executionFunc = ExecutePanelShortPress,     // Single execution function
    .context = actionButtonSensor,               // Simplified context handling
    .data = { .actionEvents = { nullptr, nullptr, nullptr } }  // Functions injected at runtime
});

// Universal long press - routed to QueuedHandler by InterruptManager
interruptManager.RegisterInterrupt({
    .id = "universal_long_press",
    .priority = Priority::NORMAL,
    .source = InterruptSource::QUEUED,
    .effect = InterruptEffect::ACTION_EVENT,
    .evaluationFunc = HasLongPressEvent,
    .executionFunc = ExecutePanelLongPress,      // Single execution function
    .context = actionButtonSensor,               // Simplified context handling
    .serviceContext = panelManager,
    .data = { .actionEvents = { nullptr, nullptr, nullptr } }  // Functions injected at runtime
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
    │   └── Creates ActionHandler (owns action sensor)
    │       └── ActionSensor (GPIO 32)
    └── Creates PanelManager
        └── Creates panels on demand (with providers injected)
            ├── KeyPanel (display-only - creates components, no sensors)
            ├── LockPanel (display-only - creates components, no sensors)
            └── OilPanel (creates own data sensors and components)
```

### Resource Management
- Single sensor instance per GPIO pin created by specialized handler (TriggerHandler owns GPIO sensors, ActionHandler owns action sensor)
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

5. **Specialized Ownership**: TriggerHandler owns GPIO sensors, ActionHandler owns action sensor, preventing resource conflicts

6. **Display-Only Panels**: Interrupt-driven panels don't create sensors, eliminating duplication and simplifying architecture

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

#### ManagerFactory (implements IManagerFactory)
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

// Direct manager creation usage
auto panelManager = managerFactory->CreatePanelManager();
auto oilPanel = panelManager->CreatePanel("OemOilPanel");
```

## Memory Management Architecture

**Critical Constraint**: ESP32-WROOM-32 has 320KB total RAM, with ~250KB available after system overhead and OTA partitioning, making memory management a first-class architectural concern.

### Memory Optimization Requirements

#### 1. Static Callback System with Centralized Restoration (Critical)
**Problem**: `std::function` with lambda captures causes heap fragmentation and system crashes.
**Solution**: All interrupt callbacks must use static function pointers with centralized restoration:

```cpp
// Required pattern for all interrupt callbacks - simplified design
struct InterruptCallbacks {
    static bool KeyPresentChanged(void* context) {
        auto* sensor = static_cast<KeyPresentSensor*>(context);
        return sensor && sensor->HasStateChanged();
    }
    
    static void LoadKeyPanel(void* context) {
        auto* panelManager = static_cast<PanelManager*>(context);
        panelManager->LoadPanel(PanelType::KEY);
        // Note: No restoration logic here - handled by InterruptManager
    }
};
```

**Memory Savings**: Centralized restoration eliminates need for deactivate function pointers, saving 4 bytes per interrupt (28 bytes total system savings).
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
1. **Memory Constraints**: All design decisions must account for ~250KB available RAM limit
2. **Single-Core Operation**: LVGL operations must not be interrupted by GPIO processing
3. **Heap Management**: Minimize dynamic allocations to prevent fragmentation
4. **Resource Cleanup**: Proper GPIO interrupt detachment in destructors

### Sensor Duplication Prevention
**Critical Rule**: Each GPIO pin must have exactly one sensor instance
**Enforcement**:
- Handlers create and own their sensors internally
- Data panels create their own data sensors
- Interrupt-driven panels are display-only (no sensor creation)
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

- **Specialized Processing**: TriggerHandler for GPIO monitoring, ActionHandler for action events
- **Central Coordination**: InterruptManager coordinates both handlers and manages execution
- **Effect-Based Execution**: LOAD_PANEL, SET_THEME, SET_PREFERENCE, CUSTOM_FUNCTION effects
- **Simplified Restoration**: Effect-based logic eliminates complex priority blocking
- **Eliminated Redundancy**: Direct coordination removes unnecessary UnifiedInterruptHandler layer

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