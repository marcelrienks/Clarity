# Development Patterns

## Overview
This document describes the key design patterns and coding practices used throughout the Clarity codebase. Each pattern is illustrated with actual examples from the implementation.

## 1. MVP (Model-View-Presenter) Architecture

The system follows a clear separation of concerns using the MVP pattern:

### Models (Sensors)
Responsible for data acquisition and hardware abstraction.

**Example: OilPressureSensor**
```cpp
// include/sensors/oil_pressure_sensor.h
class OilPressureSensor : public BaseSensor {
public:
    // Pure data acquisition
    Reading GetReading() override {
        int32_t adc_value = gpio_provider_->AnalogRead(GPIO_PIN);
        return ConvertToUnits(adc_value, target_unit_);
    }
    
    // Change detection for interrupt system
    bool HasStateChanged() override {
        int32_t current = GetCurrentReading();
        return DetectChange(current, previous_reading_);
    }
    
private:
    // Hardware abstraction via provider
    IGpioProvider* gpio_provider_;
    std::string target_unit_;
    int32_t previous_reading_;
};
```

### Views (Components)
Pure UI rendering without business logic.

**Example: GaugeComponent**
```cpp
// src/components/gauge_component.cpp
void GaugeComponent::CreateGaugeDisplay(lv_obj_t* parent) {
    // Pure UI creation - no business logic
    gauge_ = lv_meter_create(parent);
    lv_obj_set_size(gauge_, 95, 95);
    
    // Visual configuration only
    scale_ = lv_meter_add_scale(gauge_);
    lv_meter_set_scale_ticks(gauge_, scale_, 41, 2, 10, 
                              lv_palette_main(LV_PALETTE_GREY));
    
    // No data fetching, no sensor interaction
}

void GaugeComponent::UpdateValue(int32_t value) {
    // Simple display update - presenter provides the value
    lv_meter_set_indicator_value(gauge_, indicator_, value);
}
```

### Presenters (Panels)
Orchestration between models and views.

**Example: OemOilPanel**
```cpp
// src/panels/oem_oil_panel.cpp
class OemOilPanel : public IPanel {
private:
    // Owns sensors (models)
    std::unique_ptr<OilPressureSensor> pressure_sensor_;
    std::unique_ptr<OilTemperatureSensor> temperature_sensor_;
    
    // Owns components (views)
    std::unique_ptr<GaugeComponent> pressure_gauge_;
    std::unique_ptr<GaugeComponent> temperature_gauge_;
    
public:
    void Update(IGpioProvider* gpio, IDisplayProvider* display) override {
        // Presenter orchestrates: get data from model
        auto pressure_reading = pressure_sensor_->GetReading();
        auto temp_reading = temperature_sensor_->GetReading();
        
        // Update views with processed data
        if (auto* pressure_value = std::get_if<int32_t>(&pressure_reading)) {
            pressure_gauge_->UpdateValue(*pressure_value);
        }
        
        if (auto* temp_value = std::get_if<int32_t>(&temp_reading)) {
            temperature_gauge_->UpdateValue(*temp_value);
        }
    }
};
```

## 2. Singleton Pattern with Global Access

Used for system-wide services that need global access.

**Example: ErrorManager**
```cpp
// include/managers/error_manager.h
class ErrorManager {
public:
    // Scott Meyers singleton implementation
    static ErrorManager& Instance() {
        static ErrorManager instance;
        return instance;
    }
    
    // Delete copy/move constructors
    ErrorManager(const ErrorManager&) = delete;
    ErrorManager& operator=(const ErrorManager&) = delete;
    
    // Public interface
    void ReportError(ErrorLevel level, const char* source, const std::string& message);
    bool HasPendingErrors() const;
    
private:
    ErrorManager() = default;  // Private constructor
    
    std::vector<ErrorInfo> error_queue_;
    static constexpr size_t MAX_ERROR_QUEUE_SIZE = 10;
};

// Usage anywhere in the codebase
ErrorManager::Instance().ReportError(ErrorLevel::WARNING, "Sensor", "Reading out of range");
```

## 3. Factory Pattern with Dependency Injection

Centralized object creation with clear dependency management.

**Example: ManagerFactory**
```cpp
// include/factories/manager_factory.h
class ManagerFactory : public IManagerFactory {
private:
    std::unique_ptr<IProviderFactory> provider_factory_;
    
public:
    // Dependency injection via constructor
    explicit ManagerFactory(std::unique_ptr<IProviderFactory> factory) 
        : provider_factory_(std::move(factory)) {}
    
    // Factory methods with dependency resolution
    std::unique_ptr<InterruptManager> CreateInterruptManager() override {
        // Get dependencies from provider factory
        auto gpio_provider = provider_factory_->CreateGpioProvider();
        
        // Create manager with injected dependencies
        return std::make_unique<InterruptManager>(std::move(gpio_provider));
    }
    
    std::unique_ptr<PanelManager> CreatePanelManager(
        IDisplayProvider* display,
        IStyleManager* style_manager) override {
        
        // Resolve complex dependencies
        return std::make_unique<PanelManager>(display, style_manager);
    }
};
```

## 4. Template Method Pattern (BaseSensor)

Provides common functionality while allowing specialization.

**Example: BaseSensor Change Detection**
```cpp
// include/sensors/base_sensor.h
class BaseSensor : public ISensor {
protected:
    bool initialized_ = false;
    
    // Template method for change detection
    template<typename T>
    bool DetectChange(T currentValue, T& previousValue) {
        if (!initialized_) {
            previousValue = currentValue;
            initialized_ = true;
            return false;  // No change on first read
        }
        
        bool changed = (currentValue != previousValue);
        previousValue = currentValue;
        return changed;
    }
};

// Concrete sensor usage
class LightsSensor : public BaseSensor {
private:
    bool previous_state_ = false;
    
public:
    bool HasStateChanged() override {
        bool current = gpio_provider_->DigitalRead(gpio_pins::LIGHTS);
        return DetectChange(current, previous_state_);  // Uses template method
    }
};
```

## 5. State Management Pattern

Each panel manages its own state with clear lifecycle.

**Example: Panel State Management**
```cpp
// src/panels/splash_panel.cpp
class SplashPanel : public IPanel {
private:
    enum class AnimationState {
        NOT_STARTED,
        IN_PROGRESS,
        COMPLETED
    };
    
    AnimationState state_ = AnimationState::NOT_STARTED;
    unsigned long animation_start_time_ = 0;
    
public:
    void Load(IGpioProvider* gpio, IDisplayProvider* display) override {
        state_ = AnimationState::IN_PROGRESS;
        animation_start_time_ = millis();
        CreateAnimation();
    }
    
    void Update(IGpioProvider* gpio, IDisplayProvider* display) override {
        switch(state_) {
            case AnimationState::IN_PROGRESS:
                if (IsAnimationComplete()) {
                    state_ = AnimationState::COMPLETED;
                    PanelManager::NotificationService().OnPanelLoadComplete(this);
                } else {
                    UpdateAnimation();
                }
                break;
                
            case AnimationState::COMPLETED:
                // No further updates needed
                break;
        }
    }
};
```

## 6. Interface-Based Design

All major components implement interfaces for testability and loose coupling.

**Example: Multiple Interface Implementation**
```cpp
// include/managers/panel_manager.h
class PanelManager : public IPanelNotificationService,
                     public IActionExecutionService,
                     public ITriggerExecutionService {
public:
    // IPanelNotificationService implementation
    void OnPanelLoadComplete(IPanel* panel) override {
        HandlePanelLoadComplete(panel);
    }
    
    // IActionExecutionService implementation  
    void HandleShortPress() override {
        if (current_panel_) {
            current_panel_->ExecuteShortPress();
        }
    }
    
    // ITriggerExecutionService implementation
    void LoadPanel(const char* panel_name) override {
        CreateAndLoadPanel(panel_name);
    }
    
    // Static accessors for each interface
    static IPanelNotificationService& NotificationService() { return Instance(); }
    static IActionExecutionService& ActionService() { return Instance(); }
    static ITriggerExecutionService& TriggerService() { return Instance(); }
};
```

## 7. Early Return Pattern

Validate preconditions and handle edge cases early.

**Example: Defensive Programming**
```cpp
// src/managers/interrupt_manager.cpp
void InterruptManager::Process() {
    // Early validation
    if (!trigger_handler_ || !action_handler_) {
        log_e("Handlers not initialized");
        return;
    }
    
    // Check UI state early
    if (!IsUIIdle()) {
        log_v("UI busy, skipping interrupt processing");
        return;
    }
    
    // Proceed with main logic only after validations
    trigger_handler_->Process();
    action_handler_->Process();
}
```

## 8. RAII (Resource Acquisition Is Initialization)

Automatic resource management through constructors/destructors.

**Example: GPIO Interrupt Management**
```cpp
// src/sensors/key_present_sensor.cpp
class KeyPresentSensor : public BaseSensor {
private:
    IGpioProvider* gpio_provider_;
    static constexpr int GPIO_PIN = gpio_pins::KEY_PRESENT;
    
public:
    KeyPresentSensor(IGpioProvider* provider) : gpio_provider_(provider) {
        // Resource acquisition in constructor
        gpio_provider_->SetPinMode(GPIO_PIN, INPUT_PULLDOWN);
        gpio_provider_->AttachInterrupt(GPIO_PIN, HandleInterrupt, CHANGE);
    }
    
    ~KeyPresentSensor() {
        // Automatic cleanup in destructor
        if (gpio_provider_) {
            gpio_provider_->DetachInterrupt(GPIO_PIN);
        }
    }
};
```

## 9. Static Callback Pattern (Memory Safe)

Avoiding std::function to prevent heap fragmentation on ESP32.

**Example: Trigger Functions**
```cpp
// include/utilities/system_definitions.h
inline std::vector<Trigger> CreateSystemTriggers(...) {
    return {
        {
            .id = "key_present",
            .priority = Priority::CRITICAL,
            .type = TriggerType::PANEL,
            // Static function pointers - no heap allocation
            .activateFunc = []() { 
                PanelManager::TriggerService().LoadPanel(PanelNames::KEY); 
            },
            .deactivateFunc = []() { 
                PanelManager::TriggerService().CheckRestoration(); 
            },
            .sensor = keyPresentSensor,
            .isActive = false
        }
        // No std::function, no captures, no heap fragmentation
    };
}
```

## 10. Logging Standards

Structured logging with appropriate levels for different scenarios.

### Log Level Guidelines
- **Error logs (log_e)**: Critical errors that affect system operation
- **Warning logs (log_w)**: Important issues that should be addressed
- **Info logs (log_i)**: Major system actions and state changes
- **Debug logs (log_d)**: Development debugging (should be cleaned up after use)
- **Verbose logs (log_v)**: Method entry indication only

### Implementation Rules

**Example: Proper Logging Usage**
```cpp
// src/managers/panel_manager.cpp
void PanelManager::CreateAndLoadPanel(const char* panel_name) {
    log_v("CreateAndLoadPanel() called");  // Verbose for method entry only
    
    if (!panel_name) {
        log_e("Panel name is null");  // Error for critical issues
        return;
    }
    
    log_i("Panel transition requested: %s", panel_name);  // Info for major actions
    
    auto panel = panel_factory_.CreatePanel(panel_name);
    if (!panel) {
        log_w("Failed to create panel: %s", panel_name);  // Warning for issues
        return;
    }
    
    // log_d would only be used temporarily during debugging
    // and should be removed once the issue is resolved
}

// Avoid logging in frequently called methods
bool GpioProvider::DigitalRead(int pin) {
    return ::digitalRead(pin);  // No logging - called too frequently
}
```

## 11. Dependency Injection Pattern

Components receive dependencies rather than creating them.

**Example: Panel with Injected Service**
```cpp
// include/panels/splash_panel.h
class SplashPanel : public IPanel {
private:
    IPanelNotificationService* notification_service_;
    
public:
    // Constructor injection with default parameter
    explicit SplashPanel(
        IPanelNotificationService* service = &PanelManager::NotificationService())
        : notification_service_(service) {}
    
    void Update(IGpioProvider* gpio, IDisplayProvider* display) override {
        if (IsAnimationComplete()) {
            // Use injected service - enables testing with mocks
            notification_service_->OnPanelLoadComplete(this);
        }
    }
};

// Test usage with mock
TEST(SplashPanelTest, NotifiesOnCompletion) {
    MockNotificationService mock_service;
    SplashPanel panel(&mock_service);  // Inject mock
    
    // Test behavior with mock service
}
```

## 12. Const Correctness

Proper use of const for safety and clarity.

**Example: Const Methods and Parameters**
```cpp
// include/sensors/oil_pressure_sensor.h
class OilPressureSensor : public BaseSensor {
public:
    // Const method - doesn't modify object state
    std::vector<std::string> GetSupportedUnits() const override {
        return {"Bar", "PSI", "kPa"};
    }
    
    // Const reference parameter - avoid copying
    void SetTargetUnit(const std::string& unit) override {
        if (IsValidUnit(unit)) {
            target_unit_ = unit;
        }
    }
    
private:
    // Const parameter and const method
    bool IsValidUnit(const std::string& unit) const {
        auto units = GetSupportedUnits();
        return std::find(units.begin(), units.end(), unit) != units.end();
    }
};
```

## Summary

These patterns work together to create a maintainable, testable, and efficient system:

1. **MVP Architecture** - Clear separation of concerns
2. **Singleton Pattern** - Global service access
3. **Factory Pattern** - Centralized object creation
4. **Template Method** - Reusable algorithms with specialization
5. **State Management** - Predictable component behavior
6. **Interface-Based Design** - Testability and loose coupling
7. **Early Return** - Defensive programming
8. **RAII** - Automatic resource management
9. **Static Callbacks** - Memory-safe function pointers
10. **Logging Standards** - Consistent diagnostic output
11. **Dependency Injection** - Flexible and testable components
12. **Const Correctness** - Type safety and clarity

Each pattern is chosen for specific benefits in the embedded ESP32 environment, particularly focusing on memory efficiency and system reliability.