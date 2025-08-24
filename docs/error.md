# Error Handling System Design

## Overview

This document outlines the comprehensive error handling system design for the Clarity ESP32-based digital gauge system. The design integrates seamlessly with the existing MVP architecture while providing robust error collection, display, and recovery mechanisms.

## Architecture Analysis

### Current System Pattern
```
DeviceProvider → PanelManager → Panels → Components
```

### Trigger System
- Static trigger mapping with priority-based execution via `TriggerManager`
- Priority levels: CRITICAL (0) > IMPORTANT (1) > NORMAL (2)
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

## Recommended Error Handling Design

### 1. Error Data Structures

Add to `include/utilities/types.h`:

```cpp
/// @enum ErrorLevel
/// @brief Severity levels for application errors
enum class ErrorLevel { 
    WARNING,   ///< Non-critical issues that don't affect core functionality
    ERROR,     ///< Significant issues that may impact features
    CRITICAL   ///< Critical issues requiring immediate attention
};

/// @struct ErrorInfo
/// @brief Complete error information structure
struct ErrorInfo {
    ErrorLevel level;           ///< Severity level of the error
    const char* source;         ///< Component/manager that reported the error
    std::string message;        ///< Human-readable error description
    unsigned long timestamp;    ///< millis() timestamp when error occurred
    bool acknowledged;          ///< Whether user has acknowledged the error
};

/// @struct PanelNames - Add error panel constant
struct PanelNames {
    // ... existing panel constants ...
    static constexpr const char* ERROR = "ErrorPanel";  ///< Error display panel
};
```

### 2. Global Error Manager Service

Create `include/managers/error_manager.h`:

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
    void AcknowledgeError(size_t errorIndex);
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

#### Extend Trigger Mappings

Modify `src/managers/trigger_manager.cpp`:

```cpp
// Add error trigger to existing triggers array
Trigger TriggerManager::triggers_[] = {
    // ... existing triggers ...
    {TRIGGER_ERROR_OCCURRED, -1, TriggerActionType::LoadPanel, 
     PanelNames::ERROR, PanelNames::OIL, TriggerPriority::CRITICAL}
};
```

#### Add Error Trigger Processing

Extend `TriggerManager::ProcessTriggerEvents()`:

```cpp
void TriggerManager::ProcessTriggerEvents() {
    // Existing sensor-based polling
    CheckSensorChanges();
    
    // Check for error conditions
    CheckErrorTrigger();
}

void TriggerManager::CheckErrorTrigger() {
    bool shouldShowErrorPanel = ErrorManager::Instance().ShouldTriggerErrorPanel();
    CheckTriggerChange(TRIGGER_ERROR_OCCURRED, shouldShowErrorPanel);
}
```

#### Add Error Trigger Constants

Add to `include/utilities/types.h`:

```cpp
/// @brief Error trigger constant
constexpr const char *TRIGGER_ERROR_OCCURRED = "error_occurred";
```

### 4. Error Panel Implementation

Create `include/panels/error_panel.h`:

```cpp
class ErrorPanel : public IPanel {
public:
    ErrorPanel() = default;
    ~ErrorPanel() = default;
    
    // IPanel interface implementation
    void Init(IGpioProvider *gpio, IDisplayProvider *display) override;
    void Load(std::function<void()> completionCallback, IGpioProvider *gpio, IDisplayProvider *display) override;
    void Update(std::function<void()> completionCallback, IGpioProvider *gpio, IDisplayProvider *display) override;

private:
    void CreateErrorList(lv_obj_t* parent);
    void UpdateErrorDisplay();
    void HandleErrorAcknowledgment(size_t errorIndex);
    
    lv_obj_t* errorContainer_ = nullptr;
    lv_obj_t* errorList_ = nullptr;
    lv_obj_t* errorCountLabel_ = nullptr;
    bool panelLoaded_ = false;
};
```

### 5. Error Reporting Integration Points

#### Component-Level Error Reporting

Example integration in existing components:

```cpp
// In src/sensors/oil_pressure_sensor.cpp
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
    // ... existing panel creation code ...
    } else {
        ErrorManager::Instance().ReportError(ErrorLevel::ERROR, 
            "PanelManager", std::string("Unknown panel type: ") + panelName);
        return nullptr;
    }
}
```

#### Service-Level Integration

Modify `src/main.cpp` to include error manager initialization:

```cpp
// Add to global services
std::unique_ptr<ErrorManager> errorManager;

void initializeServices() {
    // ... existing service initialization ...
    
    log_d("Initializing ErrorManager...");
    errorManager = std::make_unique<ErrorManager>();
    
    // ... rest of initialization ...
}
```

### 6. Error Panel UI Design

#### Error Display Features
- **Error List**: Scrollable list showing all pending errors
- **Error Details**: Level, source, timestamp, and message for each error
- **Visual Indicators**: 
  - Red background for critical errors
  - Orange background for regular errors
  - Yellow background for warnings
- **User Actions**:
  - Touch/button to acknowledge individual errors
  - Auto-dismiss warnings after configurable timeout (e.g., 10 seconds)
  - Manual dismiss for errors and critical errors

#### Error Count Indicator
- Small badge showing total pending error count
- Visible on other panels when errors exist but error panel not active
- Color-coded by highest severity level

### 7. Error Recovery and Restoration

#### Automatic Panel Restoration
- Error panel automatically restores to previous panel when all errors are cleared or acknowledged
- Leverages existing trigger system's `restoreTarget` mechanism
- No manual intervention required for normal operation

#### Error Persistence
- Errors persist until explicitly acknowledged by user
- Critical errors cannot be auto-dismissed
- Warning and error levels can have configurable auto-dismiss timers

### 8. Memory Management

#### Bounded Error Queue
- Maximum 10 errors in queue (configurable via `MAX_ERROR_QUEUE_SIZE`)
- Oldest errors automatically removed when queue is full
- Priority given to higher severity errors

#### Efficient String Handling
- Use string views where possible for component names
- Avoid dynamic allocation in critical error paths
- Pre-allocate error message buffers where feasible

## Implementation Benefits

### 1. **Non-Intrusive Design**
- No try-catch blocks in main loops preserving Arduino's native error handling
- ESP32 crash reporting and watchdog functionality remains intact
- Error collection happens through explicit reporting calls

### 2. **Priority-Aware Error Handling**
- Critical errors immediately override current panels via CRITICAL priority trigger
- Error panel takes precedence over all other panel switching
- Automatic restoration when errors are resolved

### 3. **Seamless Integration**
- Leverages existing trigger system architecture
- Uses established panel loading and lifecycle management
- Follows existing MVP pattern and dependency injection

### 4. **Memory Efficient**
- Bounded error queue suitable for ESP32 memory constraints
- Efficient error structure with minimal overhead
- Automatic cleanup of old errors

### 5. **User Experience**
- Clear visual indication of system issues
- Appropriate error severity handling
- Automatic recovery without user intervention where possible
- Manual acknowledgment for critical issues

### 6. **Maintainable and Extensible**
- Easy to add error reporting to any component
- Consistent error handling API across the application
- Centralized error management with clear interfaces

## Implementation Phases

### Phase 1: Core Infrastructure
1. Add error data structures to `types.h`
2. Implement `ErrorManager` singleton service
3. Integrate error trigger into `TriggerManager`
4. Update service initialization in `main.cpp`

### Phase 2: Error Panel
1. Create `ErrorPanel` class implementing `IPanel`
2. Implement error list UI with LVGL components
3. Add error panel registration to `PanelManager`
4. Test error panel loading and restoration

### Phase 3: Error Integration
1. Add error reporting calls to critical components (sensors, managers)
2. Implement error acknowledgment and auto-dismiss functionality
3. Add error count indicators to other panels
4. Test complete error handling workflow

### Phase 4: Polish and Optimization
1. Optimize memory usage and error queue management
2. Add configuration options for timeouts and queue size
3. Implement comprehensive error logging
4. Performance testing and optimization

This design provides a robust, integrated error handling system that enhances the reliability and maintainability of the Clarity application while respecting the constraints of the Arduino/ESP32 platform.