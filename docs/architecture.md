# Architecture

## Overview

MVP (Model-View-Presenter) pattern for ESP32 automotive gauges with layered architecture, interface-based design, and handler-based interrupt system using function pointers.

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
- Interrupt-driven panels (Key, Lock) are display-only
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
    void (*executionFunc)(void* context);            // Single execution function - simplified design
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

// Memory Usage: ~29 bytes per interrupt (optimized from previous 33-byte design)
// Total system overhead: ~203 bytes for 7 interrupts (vs 231 bytes with activate/deactivate functions)
```

#### Critical Memory Constraint
**ESP32 Memory Limitation**: The ESP32-WROOM-32 has 320KB total RAM, with ~250KB available after system overhead and OTA partitioning. Using `std::function` with lambda captures causes:
- Heap fragmentation leading to crashes
- Memory overhead of ~3KB per std::function object
- Unstable system behavior during LVGL operations

**Solution**: Static callback pattern eliminates heap allocations and provides predictable memory usage.

### Coordinated Interrupt Processing System
The coordinated system processes interrupts through specialized handlers with centralized restoration logic:
- **PolledHandler**: Manages GPIO state changes with sensors tracking previous state internally
- **QueuedHandler**: Processes button events from message queue
- **InterruptManager**: Coordinates both handlers, handles effect-based execution and **centralized restoration logic**
- **Priority-based coordination**: Highest priority interrupt across both handlers gets processed

#### Centralized Restoration Architecture
**Hybrid Design**: InterruptManager coordinates PolledHandler and QueuedHandler with centralized restoration:

1. **Handler Processing**: Each handler evaluates its interrupts and marks active ones
2. **Priority Coordination**: InterruptManager compares highest priority interrupt from each handler
3. **Effect-Based Execution**: InterruptManager executes based on interrupt effect (LOAD_PANEL, SET_THEME, etc.)
4. **Centralized Restoration**: InterruptManager handles all restoration logic when panel-loading interrupts deactivate

**Key Architectural Decision**: Restoration logic centralized in InterruptManager rather than distributed across interrupt callbacks, reducing complexity and memory overhead.

#### Change Detection for POLLED Interrupts
**PolledHandler Processing**: GPIO state monitoring maintains change detection patterns:
- Sensors still track previous state internally for change detection
- `evaluationFunc` calls sensor's `HasStateChanged()` method exactly once per cycle
- Context parameter provides sensor access without heap allocation
- Same change detection corruption prevention as before

#### Centralized Restoration Logic
**InterruptManager Restoration**: When panel-loading interrupts deactivate, restoration is handled centrally:

```cpp
void InterruptManager::HandleRestoration() {
    // Check for any active panel-loading interrupts
    auto* highestActive = GetHighestPriorityActivePanelInterrupt();
    
    if (highestActive) {
        // Execute highest priority active panel interrupt
        highestActive->executionFunc(highestActive->context);
    } else {
        // No blocking interrupts - restore user panel
        RestoreUserPanel();
    }
}
```

**Benefits of Centralized Restoration**:
- **Single Source of Truth**: All restoration decisions made in one location
- **Memory Efficient**: Eliminates need for deactivate function pointers (saves 28 bytes total)
- **Maintainable**: Clear separation between interrupt execution and system coordination
- **Testable**: Restoration logic can be unit tested independently

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
- PolledHandler owns all GPIO sensors, QueuedHandler owns button sensor
- Panels are display-only and must not create sensors
- No sensor duplication across components

## Managers

- **InterruptManager**: Creates and coordinates PolledHandler and QueuedHandler during LVGL idle time with centralized restoration logic
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
   - Process centralized restoration logic when panel-loading interrupts deactivate

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
- **NORMAL (2)**: Theme changes, button interrupts, preferences

### Centralized Panel Restoration
- **Effect-Based Logic**: Only LOAD_PANEL effects participate in restoration
- **Centralized Processing**: InterruptManager::HandleRestoration() manages all restoration decisions
- **Priority Query**: System queries for highest priority active panel-loading interrupt
- **Theme Independence**: SET_THEME effects never affect panel restoration
- **Simplified Architecture**: Single restoration function eliminates distributed restoration logic

## Available Panels

### Panel Types and Button Behaviors

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
  - No sensor creation - receives state from PolledHandler
  - Short Press: No action (status display only)
  - Long Press: Load config panel
  
- **Lock Panel**: Vehicle security status (Display-only)
  - Visual indication of lock engaged/disengaged state
  - No sensor creation - receives state from PolledHandler
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
    
    // Static callback functions for universal button system
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
- **Universal Button System**: Short/long press functions injected when panel loads

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
    
    // Static callback functions for universal button system
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
    .activateFunc = GenerateDebugError,
    .deactivateFunc = nullptr,
    .sensorContext = debugErrorSensor,
    .serviceContext = errorManager
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

## Universal Button System Architecture

### Button Function Injection Pattern
The universal button system enables all panels to respond to button input through a coordinated injection mechanism:

```cpp
// IActionService interface - all panels must implement
class IActionService {
public:
    virtual ~IActionService() = default;
    virtual void (*GetShortPressFunction())(void* panelContext) = 0;
    virtual void (*GetLongPressFunction())(void* panelContext) = 0;
    virtual void* GetPanelContext() = 0;
};

// Universal button interrupt with dynamic function injection
struct UniversalButtonInterrupt {
    const char* id;                                    // "universal_short_press" / "universal_long_press"
    InterruptSource source = InterruptSource::QUEUED;
    InterruptEffect effect = InterruptEffect::BUTTON_ACTION;
    bool (*evaluationFunc)(void* context);           // HasShortPressEvent / HasLongPressEvent
    void (*activateFunc)(void* context);             // ExecutePanelFunction - calls current panel's function
    void* sensorContext;                              // ActionButtonSensor
    void* serviceContext;                             // PanelManager
    
    // Runtime function injection from current panel
    void (*currentPanelFunction)(void* panelContext); // Injected when panels switch
    void* currentPanelContext;                        // Current panel instance
};
```

### Function Injection Process
1. **Panel Switch**: PanelManager loads new panel
2. **Function Extraction**: Get functions from panel via IActionService interface
3. **Interrupt Update**: Inject panel functions into universal button interrupts
4. **Context Update**: Update panel context for function execution
5. **Ready State**: Button interrupts now execute current panel's functions

### Static Callback Implementation
```cpp
class UniversalButtonCallbacks {
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
  - Inherits from `IActionService` for universal button handling
- `IActionService`: Universal button function interface for all panels
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

### QUEUED Universal Panel Button Registration (every panel has these)
```cpp
// Universal short press - routed to QueuedHandler by InterruptManager
interruptManager.RegisterInterrupt({
    .id = "universal_short_press",
    .priority = Priority::NORMAL,
    .source = InterruptSource::QUEUED,
    .effect = InterruptEffect::BUTTON_ACTION,
    .evaluationFunc = HasShortPressEvent,
    .executionFunc = ExecutePanelShortPress,     // Single execution function
    .context = actionButtonSensor,               // Simplified context handling
    .data = { .buttonActions = { nullptr, nullptr, nullptr } }  // Functions injected at runtime
});

// Universal long press - routed to QueuedHandler by InterruptManager
interruptManager.RegisterInterrupt({
    .id = "universal_long_press",
    .priority = Priority::NORMAL,
    .source = InterruptSource::QUEUED,
    .effect = InterruptEffect::BUTTON_ACTION,
    .evaluationFunc = HasLongPressEvent,
    .executionFunc = ExecutePanelLongPress,      // Single execution function
    .context = actionButtonSensor,               // Simplified context handling
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
        panelManager->LoadPanel("KEY");
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