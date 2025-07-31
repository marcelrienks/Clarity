# Clarity Gauge System Requirements

## Project Overview

Clarity is an ESP32-based digital gauge system for automotive engine monitoring, built with PlatformIO and using LVGL for UI and LovyanGFX for display drivers. It's designed for a 1.28" round display (GC9A01 driver) on a NodeMCU-32S development board.

## Hardware Requirements

### Core Hardware
- **Microcontroller**: ESP32-WROOM-32 (NodeMCU-32S development board)
- **Display**: 1.28" round LCD with GC9A01 driver (240x240 resolution)
- **Memory**: 4MB Flash, 320KB RAM
- **Interface**: SPI2_HOST for display communication

### GPIO Pin Assignments
- **Oil Pressure Sensor**: Analog input for 0-10 Bar range
- **Oil Temperature Sensor**: Analog input for 0-120°C range
- **Key Present**: GPIO 25 (digital input)
- **Key Not Present**: GPIO 26 (digital input)
- **Lock State**: GPIO 27 (digital input)
- **Lights State**: GPIO for theme switching (Day/Night)

### ADC Configuration
- **Resolution**: 12-bit (0-4095)
- **Attenuation**: ADC_11db (0-3.3V range)
- **Sampling**: Time-based with configurable intervals

## Software Architecture Requirements

### Core Design Principles

#### 1. MVP (Model-View-Presenter) Pattern
```
Device → Display → Panels → Components
```
- **Device**: Hardware abstraction layer managing display and LVGL integration
- **Panels**: Presenters that coordinate between sensors and components
- **Components**: Views that render UI elements (gauges, indicators, etc.)
- **Sensors**: Models that handle data acquisition from hardware inputs

#### 2. Direct GPIO Polling Architecture
- **Core 0 Only**: All application logic runs on single core
- **No Interrupts**: Direct GPIO reading with polling-based change detection
- **No Cross-Core Sync**: Eliminates need for mutexes, queues, or synchronization
- **Immediate Response**: Actions triggered directly on pin state changes

#### 3. Manager-Based System
- **PanelManager**: Handles panel switching and lifecycle management
- **TriggerManager**: Processes GPIO changes and determines actions
- **StyleManager**: Manages LVGL styling and themes (Day/Night)
- **PreferenceManager**: Handles persistent configuration storage

### Required Interfaces

#### Core Interfaces
```cpp
class IDevice {
    virtual void prepare() = 0;
    // Hardware abstraction for display devices
};

class IPanel {
    virtual void init() = 0;
    virtual void load(std::function<void()> callback) = 0;
    virtual void update(std::function<void()> callback) = 0;
    // Screen/panel implementations with init/load/update lifecycle
};

class IComponent {
    virtual void render(lv_obj_t* screen, const ComponentLocation& location) = 0;
    virtual void refresh(const Reading& reading) = 0;
    // UI components with render_load/render_update methods
};

class ISensor {
    virtual void init() = 0;
    virtual Reading getReading() = 0;
    // Sensor data acquisition interface
};
```

#### Trigger System Requirements
- **Pin-Change Driven Only**: Actions triggered by GPIO state changes, not states
- **No State Tracking**: No artificial complexity or state history
- **Priority-Based**: Higher priority triggers (key) override lower priority (lock)
- **Theme vs Panel Actions**: Separate handling for UI themes and panel switching

### Memory and Performance Requirements

#### Memory Constraints
- **Flash Usage**: Must fit within 2MB partition (50% of 4MB total)
- **RAM Usage**: Stay under 110KB (33% of 320KB available)
- **LVGL Buffer**: 120KB optimized for 60-line dual buffering

#### Performance Requirements
- **Main Loop Frequency**: 25-30ms cycle time
- **Sensor Sampling**: Configurable intervals (default 100ms)
- **Animation Smoothness**: 60 FPS for gauge animations
- **Boot Time**: Under 3 seconds to operational state

### Display and UI Requirements

#### Display Configuration
- **Resolution**: 240x240 pixels (round display)
- **Color Depth**: 16-bit RGB565
- **Orientation**: Optimized for round display presentation
- **Buffering**: Dual buffer for smooth animations

#### Theme Support
- **Day Theme**: Light background, dark text/indicators
- **Night Theme**: Dark background, light text/indicators
- **Theme Switching**: Triggered by lights_state GPIO
- **Color Consistency**: All components must support both themes

### Panel and Component Requirements

#### Required Panels
1. **SplashPanel**: Startup animation with "Clarity" branding
2. **OemOilPanel**: Primary oil pressure and temperature gauges
3. **KeyPanel**: Key presence indicator
4. **LockPanel**: Lock state indicator

#### Component Specifications
- **OemOilPressureComponent**: 0-10 Bar range with visual gauge
- **OemOilTemperatureComponent**: 0-120°C range with visual gauge
- **KeyComponent**: Present/Not Present/Inactive states with color coding
- **LockComponent**: Engaged/Disengaged states with visual indicator

### Build and Development Requirements

#### Build Environments
- **debug-local**: Fastest build for development testing
- **debug-upload**: Debug build with inverted colors for waveshare displays
- **release**: Optimized production build with inverted colors

#### Development Tools
- **PlatformIO**: Primary build system
- **Wokwi**: Emulation for testing (limitations: square display, inverted rendering)
- **Native Testing**: Unit tests for core logic
- **Integration Testing**: Full system tests with scenarios

#### Code Quality Standards
- **Logging**: Structured logging with appropriate levels (DEBUG/INFO/VERBOSE)
- **Error Handling**: Graceful handling of sensor failures and edge cases
- **Resource Management**: Proper cleanup of LVGL objects and animations
- **Memory Safety**: No memory leaks or buffer overflows

### Configuration and Persistence

#### Required Configuration
```cpp
struct Config {
    std::string panelName;      // Default panel on startup
    Theme defaultTheme;         // Day/Night theme preference
    // Additional settings as needed
};
```

#### Persistence Requirements
- **NVS (Non-Volatile Storage)**: Configuration persistence across reboots
- **Default Fallback**: Graceful handling when no config exists
- **Configuration Validation**: Ensure loaded config values are valid

### Testing and Quality Assurance

#### Required Test Coverage
- **Unit Tests**: Core logic and utility functions
- **Integration Tests**: End-to-end scenarios with Wokwi
- **Build Verification**: All environments must compile successfully
- **Memory Testing**: Verify memory usage stays within constraints

#### Performance Benchmarks
- **Boot Time**: Measure and verify startup performance
- **Memory Usage**: Track Flash/RAM usage across builds
- **Response Time**: Ensure trigger response under 50ms

### Deployment and Distribution

#### Firmware Packaging
- **Binary Output**: `.bin` files for OTA updates
- **Partition Layout**: Custom partitioning with OTA support
- **Version Management**: Clear versioning and changelog

#### Production Requirements
- **OTA Support**: Over-the-air firmware updates
- **Failsafe**: Robust error handling and recovery
- **Monitoring**: Performance and error logging capability

## Anti-Patterns and Constraints

### Forbidden Patterns
❌ **State Tracking Complexity**: No artificial state history or tracking
❌ **Cross-Core Communication**: All logic must remain on single core
❌ **Artificial Delays**: No delays to "fix" timing issues
❌ **Continuous Processing**: Events only on actual hardware changes
❌ **Resource Leaks**: All LVGL objects must be properly cleaned up

### Required Patterns
✅ **Direct GPIO Reading**: Polling-based change detection
✅ **Immediate Response**: Actions triggered directly on changes
✅ **Clean Interfaces**: Proper separation of concerns between layers
✅ **Resource Management**: Proper initialization and cleanup
✅ **Graceful Degradation**: System continues operation despite sensor failures

## Future Extension Points

### Planned Extensibility
- **Additional Sensors**: Framework supports new sensor types
- **New Panels**: Panel factory pattern enables easy additions
- **Additional Themes**: Theme system supports expansion
- **Configuration Options**: Settings framework ready for new options

### API Stability
- Core interfaces (IPanel, IComponent, ISensor) must remain stable
- Manager APIs should be backward compatible
- Configuration format should support versioning

This requirements document serves as the authoritative guide for maintaining system architecture integrity during future development and ensures all changes meet the established design principles and constraints.