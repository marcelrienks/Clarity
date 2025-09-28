# Error Handling System Design

**Related Documentation:**
- **[Architecture](architecture.md)** - MVP pattern and system architecture
- **[Requirements](requirements.md)** - Error handling requirements and integration
- **[Interrupt Architecture](interrupts.md)** - Error trigger implementation

## Overview

This document outlines the comprehensive error handling system design for the Clarity ESP32-based digital gauge system. The design integrates seamlessly with the existing MVP architecture while providing robust error collection, display, and recovery mechanisms.

## System Architecture

### System Pattern
```
DeviceProvider → PanelManager → Panels → Components
```

### Trigger System
- Static trigger mapping with priority-based execution via TriggerHandler
- Priority levels: CRITICAL (2) > IMPORTANT (1) > NORMAL (0)
- Automatic panel switching based on GPIO state changes

### Main Loop Structure
- `setup()`: Initializes all services and dependencies
- `loop()`: Processes trigger events and updates panels continuously

## Error Handling Constraints

### Arduino/ESP32 Limitations
- **Critical Constraint**: Adding try-catch blocks to `setup()` or `loop()` functions prevents Arduino's built-in crash reporting over serial
- ESP32 provides hardware watchdog and exception handlers for crash dumps
- Must preserve native error reporting while adding application-level error handling

### Design Requirements
1. Non-intrusive error collection that doesn't interfere with Arduino crash reporting
2. Integration with existing trigger and panel systems
3. Memory-efficient implementation suitable for ESP32 constraints
4. Priority-aware error handling that can override current operations
5. Automatic recovery and restoration mechanisms

## Current Error Handling Implementation

### 1. Error Data Structures

Defined in `include/definitions/state_types.h`:

```cpp
/// @enum ErrorLevel (in enums.h)
/// @brief Severity levels for application errors with auto-dismiss support
enum class ErrorLevel : uint8_t {
    WARNING = 0,   ///< Non-critical issues (auto-dismissible after timeout)
    ERROR = 1,     ///< Significant issues (removable on acknowledgment)
    CRITICAL = 2   ///< Critical issues (highest priority, manual dismiss only)
};

/// @struct ErrorInfo
/// @brief Complete error information structure optimized for embedded systems
struct ErrorInfo {
    ErrorLevel level;        ///< Severity level of the error
    const char *source;      ///< Component/manager that reported the error (static string)
    char message[128];       ///< Fixed-size message buffer for embedded optimization
    unsigned long timestamp; ///< millis() timestamp when error occurred
    bool acknowledged;       ///< Whether user has acknowledged the error

    // Helper method to set message safely
    void SetMessage(const std::string& msg) {
        strncpy(message, msg.c_str(), 127);
        message[127] = '\0'; // Ensure null termination
    }
};

/// @struct PanelNames (in constants.h)
struct PanelNames {
    // ... existing panel constants ...
    static constexpr const char* ERROR = "ErrorPanel";  ///< Error display panel
};
```

### 2. Global Error Manager Service

Implemented in `include/managers/error_manager.h`:

```cpp
class ErrorManager {
public:
    static ErrorManager& Instance();
    
    // Error reporting interface
    void ReportError(ErrorLevel level, const char* source, const std::string& message);
    void ReportWarning(const char* source, const std::string& message);
    void ReportCriticalError(const char* source, const std::string& message);
    
    // Error queue management
    bool HasPendingErrors() const;
    bool HasCriticalErrors() const;
    std::vector<ErrorInfo> GetErrorQueue() const;
    void AcknowledgeError(size_t errorIndex);  // Auto-removes non-critical errors
    void ClearAllErrors();
    
    // Integration with trigger system
    bool ShouldTriggerErrorPanel() const;
    void SetErrorPanelActive(bool active);

private:
    static constexpr size_t MAX_ERROR_QUEUE_SIZE = 10;  // Memory-constrained device
    
    std::vector<ErrorInfo> errorQueue_;
    bool errorPanelActive_ = false;
    unsigned long lastWarningDismissalTime_ = 0;
    
    ErrorManager() = default;
    void TrimErrorQueue();  // Remove old errors when queue is full
};
```

### 3. Integration with Trigger System

#### Error Trigger Registration

Implemented in TriggerHandler initialization:

```cpp
// Error trigger registered with TriggerHandler
Trigger errorTrigger = {
    .id = "error_occurred",
    .priority = Priority::CRITICAL,
    .type = TriggerType::PANEL,
    .activateFunc = []() { PanelManager::Instance().LoadPanel(PanelType::ERROR); },
    .deactivateFunc = []() { PanelManager::Instance().CheckRestoration(); },
    .sensor = &errorSensor,
    .isActive = false
};
```

#### Error Trigger Processing

Implemented in `TriggerHandler::Process()`:

```cpp
void TriggerHandler::Process() {
    // Process all registered triggers including error trigger
    for (auto& trigger : triggers_) {
        if (trigger.sensor && trigger.sensor->HasStateChanged()) {
            ProcessTriggerStateChange(trigger);
        }
    }
}
```

#### Error Trigger Constants

Defined in `include/utilities/types.h`:

```cpp
/// @brief Error trigger constant
constexpr const char *TRIGGER_ERROR_OCCURRED = "error_occurred";
```

### 4. Error Panel Implementation

Implemented in `include/panels/error_panel.h`:

```cpp
class ErrorPanel : public IPanel {
public:
    // Dependency injection constructor
    ErrorPanel(IGpioProvider *gpio, IDisplayProvider *display, IStyleManager *styleManager,
               IPanelManager *panelManager);
    ~ErrorPanel();

    // IPanel interface implementation
    void Init() override;
    void Load() override;
    void Update() override;

    // Action handling for button events
    void HandleShortPress();  // Cycle to next error (view-once system)
    void HandleLongPress();   // Clear all errors and exit panel

private:
    void SortErrorsBySeverity();
    void AdvanceToNextError();
    static void ShowPanelCompletionCallback(lv_event_t *event);

    // Dependencies
    IGpioProvider *gpioProvider_;
    IDisplayProvider *displayProvider_;
    IStyleManager *styleManager_;
    IPanelManager *panelManager_;

    // UI components
    lv_obj_t* screen_ = nullptr;
    ErrorComponent errorComponent_;  // Stack-allocated component
    ComponentLocation centerLocation_;

    // Error management state
    std::vector<ErrorInfo> currentErrors_;       // Current unviewed errors
    std::vector<ErrorInfo> viewedErrors_;        // Errors that have been viewed
    size_t currentErrorIndex_;                   // Index in current error list
    std::string previousTheme_;                  // Theme to restore on exit
    bool componentInitialized_ = false;
    bool panelLoaded_ = false;
};
```

### 5. Error Reporting Integration Points

#### Component-Level Error Reporting

Current integration in components:

```cpp
// Implemented in src/sensors/oil_pressure_sensor.cpp
Reading OilPressureSensor::GetReading() {
    auto reading = analogRead(pin_);
    if (reading == 0 || reading > 4095) {
        ErrorManager::Instance().ReportError(ErrorLevel::ERROR, 
            "OilPressureSensor", "Sensor reading out of range");
        return std::monostate{};
    }
    return static_cast<int32_t>(reading);
}

// In src/managers/panel_manager.cpp
std::shared_ptr<IPanel> PanelManager::CreatePanel(const char *panelName) {
    if (strcmp(panelName, PanelNames::SPLASH) == 0) {
        return std::make_shared<SplashPanel>();
    } else if (strcmp(panelName, PanelNames::OIL) == 0) {
        return std::make_shared<OemOilPanel>();
    } else if (strcmp(panelName, PanelNames::KEY) == 0) {
        return std::make_shared<KeyPanel>();
    } else if (strcmp(panelName, PanelNames::LOCK) == 0) {
        return std::make_shared<LockPanel>();
    } else if (strcmp(panelName, PanelNames::ERROR) == 0) {
        return std::make_shared<ErrorPanel>();
    } else if (strcmp(panelName, PanelNames::CONFIG) == 0) {
        return std::make_shared<ConfigPanel>();
    } else {
        ErrorManager::Instance().ReportError(ErrorLevel::ERROR, 
            "PanelManager", std::string("Unknown panel type: ") + panelName);
        return nullptr;
    }
}
```

#### Service-Level Integration

Implemented in `src/main.cpp` using singleton pattern:

```cpp
// ErrorManager uses singleton pattern and is created via ManagerFactory
ErrorManager *errorManager;

bool initializeServices() {
    // ... other service initialization ...
    
    // Create ErrorManager via ManagerFactory (singleton pattern)
    errorManager = managerFactory->CreateErrorManager();
    if (!errorManager) {
        log_e("Failed to create ErrorManager via factory");
        return false;
    }
    
    // ErrorManager::Instance() is available globally throughout the system
    log_d("ErrorManager singleton initialized and available globally");
    
    return true;
}
```

### 6. Error Panel UI Design

#### Error Display Features
- **Single Error Display**: Shows one error at a time with full message visibility
- **Error Details**: Large, readable display of level, source, and complete message
- **Visual Indicators**:
  - Circular colored border matching error severity (red/orange/yellow)
  - Large error level text (CRIT/ERR/WARN) with severity color
  - Responsive layout optimized for 240x240 round display
- **User Actions**:
  - **Short Press**: Advance to next error (view-once system - viewed errors are removed)
  - **Long Press**: Clear all remaining errors and exit panel

#### Error Count Display
- Shows remaining error count as "errors: N" (decreases as errors are viewed)
- Navigation instructions displayed at bottom of screen
- Real-time count updates as new errors are added during active session

#### View-Once System Design
- **Dynamic Error Addition**: New errors can be added during an active error session
- **View-Once Removal**: Each error is removed from display when navigated to
- **Severity Sorting**: Errors automatically sorted by severity (CRITICAL → ERROR → WARNING)
- **Session Independence**: ErrorPanel maintains its own error list separate from ErrorManager
- **Auto-restoration**: Panel automatically exits when no errors remain

#### Display Constraints and Message Length Limits

The Clarity system uses a **240x240 pixel round display** with specific constraints for error message display:

**Error Message Display Area:**
- **Message area**: 180x100 pixels within the circular display
- **Font**: Montserrat 12pt with text wrapping enabled (`LV_LABEL_LONG_WRAP`)
- **Layout**: Center-aligned, multi-line text support
- **Available lines**: ~7-8 lines of text (100px height ÷ 14px line height)
- **Characters per line**: ~25-30 characters (varies by character width)

**Message Length Guidelines:**
- **Optimal length**: 60-80 characters (fits cleanly without excessive wrapping)
- **Maximum reasonable**: ~100 characters (acceptable with wrapping)
- **Hard limit**: 128 characters (enforced by `DataConstants::ErrorInfo::MAX_MESSAGE_LENGTH`)

**Message Optimization Examples:**
```cpp
// TOO LONG (89 chars) - Poor display formatting:
"ConfigurationManager is null - oil temperature sensor configuration cannot be registered"

// OPTIMIZED (46 chars) - Clean display:
"ConfigManager null - temp sensor config failed"

// TOO LONG (75 chars) - Causes excessive wrapping:
"ConfigurationManager is null - system cannot register configuration schema"

// OPTIMIZED (41 chars) - Fits comfortably:
"ConfigManager null - system config failed"
```

**Implementation Guidelines:**
- Use abbreviated component names (e.g., "ConfigManager" vs "ConfigurationManager")
- Focus on essential information: what failed and impact
- Prefer active voice and concise phrasing
- Test message display on actual hardware when possible
- All 108 error reporting locations have been reviewed for optimal display

**Technical Details:**
- Error message buffer: Fixed 128-byte char array for embedded optimization
- Text truncation: Automatic via `strncpy()` with null termination
- Thread safety: Error messages copied to avoid pointer invalidation
- Memory efficiency: No dynamic allocation for error strings

### 7. Error Recovery and Restoration

#### Automatic Panel Restoration
- Error panel automatically restores to previous panel when all errors are cleared or acknowledged
- Leverages existing trigger system's `restoreTarget` mechanism
- No manual intervention required for normal operation

#### Error Persistence and Acknowledgment Policy
- **Warnings and Errors**: Automatically removed when acknowledged via `AcknowledgeError()`
- **Critical Errors**: Remain visible after acknowledgment until explicitly cleared via `ClearAllErrors()`
- **Auto-dismiss**: Warnings auto-dismiss after 10 seconds (configurable)
- **View-Once System**: ErrorPanel implements independent removal on navigation

### 8. Trigger System Integration Improvements

#### Dynamic Error Generation During Error Panel
Recent improvements to the trigger system now support dynamic error addition during active error sessions:

```cpp
// In src/handlers/trigger_handler.cpp - HandleTriggerActivation()
// Early return if error panel is active - suppress trigger execution but keep state
// Exception: Allow error trigger to execute during error panel to support dynamic error addition
if (ErrorManager::Instance().IsErrorPanelActive() && strcmp(trigger.id, "error") != 0) {
    return;
}
```

#### Error Trigger Execution
- **Normal triggers**: Blocked during error panel display to prevent interference
- **Error trigger**: Allowed to execute during error panel to support dynamic error addition
- **Debug error generation**: Generates unique timestamped errors for testing

#### Error Trigger Constants (in definitions/constants.h)
```cpp
namespace TriggerIds {
    static constexpr const char* ERROR = "error";  // Debug error trigger ID
}
```

### 9. Memory Management

#### Bounded Error Queue
- Maximum 10 errors in queue (configurable via `MAX_ERROR_QUEUE_SIZE`)
- Oldest errors automatically removed when queue is full
- Priority given to higher severity errors (CRITICAL → ERROR → WARNING)
- Auto-dismiss warnings after 10 seconds

#### Efficient String Handling
- Fixed 128-byte char arrays instead of std::string for embedded optimization
- Safe string copying with null termination
- No dynamic allocation in error reporting paths
- Thread-safe error message handling

## System Benefits

### **Non-Intrusive Design**
- No try-catch blocks in main loops preserving Arduino's native error handling
- ESP32 crash reporting and watchdog functionality remains intact
- Error collection happens through explicit reporting calls

### **Priority-Aware Error Handling**
- Critical errors immediately override current panels via CRITICAL priority trigger
- Error panel takes precedence over all other panel switching
- Automatic restoration when errors are resolved

### **Seamless Integration**
- Leverages existing trigger system architecture
- Uses established panel loading and lifecycle management
- Follows existing MVP pattern and dependency injection

### **Memory Efficient**
- Bounded error queue suitable for ESP32 memory constraints
- Efficient error structure with minimal overhead
- Automatic cleanup of old errors

### **User Experience**
- Clear visual indication of system issues
- Appropriate error severity handling
- Automatic recovery without user intervention where possible
- Manual acknowledgment for critical issues

### **Maintainable and Extensible**
- Easy to add error reporting to any component
- Consistent error handling API across the application
- Centralized error management with clear interfaces

This design provides a robust, integrated error handling system that enhances the reliability and maintainability of the Clarity application while respecting the constraints of the Arduino/ESP32 platform.