# Implementation Files Overview

This document provides a comprehensive overview of each implementation file in the Clarity ESP32 automotive gauge project, documenting their purpose and role within the MVP (Model-View-Presenter) architecture.

## Architecture Overview

The system follows a layered architecture: **Device → Display → Panels → Components**

- **Device Layer**: Hardware abstraction (device.cpp, providers)
- **Display Layer**: Display abstraction and LVGL integration (display_provider)
- **Panels Layer**: Presenters that coordinate between data and display (panels/)
- **Components Layer**: Views that render specific UI elements (components/)

The MVP pattern implementation:
- **Models**: Sensors and preference management
- **Views**: Components that render UI elements
- **Presenters**: Panels that coordinate between models and views

## Application Entry Point

### `src/main.cpp`
- **Purpose**: Arduino-style application entry point with integrated dependency injection setup and application lifecycle management
- **Role**: Complete application orchestration - hardware initialization, service registration, and main loop coordination
- **Dependencies**: All service interfaces, factories, managers, and hardware providers

## Hardware Abstraction Layer

### `src/device.cpp`
- **Purpose**: Manages ESP32 display hardware (GC9A01), SPI configuration, and LVGL initialization
- **Role**: Hardware abstraction layer (foundation layer)
- **Dependencies**: LovyanGFX, LVGL
- **Implements**: `IDevice`

### Providers (Hardware Abstraction)

#### `src/providers/esp32_gpio_provider.cpp`
- **Purpose**: Provides actual ESP32 GPIO operations (digitalRead, digitalWrite, etc.)
- **Role**: Hardware abstraction for GPIO operations
- **Implements**: `IGpioProvider`

#### `src/providers/lvgl_display_provider.cpp`
- **Purpose**: Provides LVGL display operations and object creation
- **Role**: Display abstraction layer
- **Dependencies**: LVGL
- **Implements**: `IDisplayProvider`

## Sensors (Model Layer)

### `src/sensors/key_sensor.cpp`
- **Purpose**: Reads two GPIO pins to determine if ignition key is present/absent
- **Role**: Model - provides key state data
- **Dependencies**: GPIO pins, hardware pin definitions
- **Implements**: `ISensor`

### `src/sensors/lock_sensor.cpp`
- **Purpose**: Reads GPIO pin to determine lock engagement status
- **Role**: Model - provides lock state data
- **Dependencies**: GPIO pins
- **Implements**: `ISensor`

### `src/sensors/oil_pressure_sensor.cpp`
- **Purpose**: Reads analog GPIO pin (ADC) to measure oil pressure (0-10 Bar)
- **Role**: Model - provides oil pressure data
- **Dependencies**: ADC, analog GPIO pins
- **Implements**: `ISensor`

### `src/sensors/oil_temperature_sensor.cpp`
- **Purpose**: Reads analog GPIO pin (ADC) to measure oil temperature (0-120°C)
- **Role**: Model - provides oil temperature data
- **Dependencies**: ADC, analog GPIO pins
- **Implements**: `ISensor`

## Components (View Layer)

### `src/components/clarity_component.cpp`
- **Purpose**: Renders "Clarity" text label for splash screen
- **Role**: View - renders splash screen text element
- **Dependencies**: `IStyleService`, LVGL label
- **Implements**: `IComponent`

### `src/components/key_component.cpp`
- **Purpose**: Renders key icon with color-coded status (present/absent)
- **Role**: View - renders key status visual element
- **Dependencies**: `IStyleService`, key icon asset
- **Implements**: `IComponent`

### `src/components/lock_component.cpp`
- **Purpose**: Renders lock icon with color-coded status (engaged/disengaged)
- **Role**: View - renders lock status visual element
- **Dependencies**: `IStyleService`, lock icon asset
- **Implements**: `IComponent`

### OEM Oil Components

#### `src/components/oem/oem_oil_component.cpp`
- **Purpose**: Complex gauge component with scale, needle, labels, and danger zones
- **Role**: View - renders automotive-style gauge with full 3D needle effects
- **Dependencies**: `IStyleService`, LVGL scale/line objects
- **Implements**: `IComponent`

#### `src/components/oem/oem_oil_pressure_component.cpp`
- **Purpose**: Specialized oil pressure gauge (0-60 display range, danger below 5)
- **Role**: View - renders pressure-specific gauge configuration
- **Dependencies**: Inherits from `OemOilComponent`
- **Implements**: `IComponent` via inheritance

#### `src/components/oem/oem_oil_temperature_component.cpp`
- **Purpose**: Specialized oil temperature gauge (0-120°C range, danger above 100°C, reversed scale)
- **Role**: View - renders temperature-specific gauge with reversed mapping
- **Dependencies**: Inherits from `OemOilComponent`
- **Implements**: `IComponent` via inheritance

## Panels (Presenter Layer)

### `src/panels/splash_panel.cpp`
- **Purpose**: Displays "Clarity" text with fade-in/fade-out animation sequence
- **Role**: Presenter - coordinates splash screen display and transitions
- **Dependencies**: `IComponentFactory`, animation timers
- **Implements**: `IPanel`

### `src/panels/key_panel.cpp`
- **Purpose**: Shows key presence status, reads GPIO pins directly for real-time updates
- **Role**: Presenter - coordinates between key sensors and key component display
- **Dependencies**: `IComponentFactory`, GPIO provider
- **Implements**: `IPanel`

### `src/panels/lock_panel.cpp`
- **Purpose**: Shows lock engagement status using sensor data
- **Role**: Presenter - coordinates between lock sensor and lock component display
- **Dependencies**: `IComponentFactory`, `LockSensor`
- **Implements**: `IPanel`

### `src/panels/oem_oil_panel.cpp`
- **Purpose**: Complex panel showing both oil pressure and temperature gauges with smooth animations
- **Role**: Presenter - coordinates between oil sensors and gauge components with animation management
- **Dependencies**: `IComponentFactory`, oil sensors, LVGL animations
- **Implements**: `IPanel`

## Managers (Service Layer)

### `src/managers/panel_manager.cpp`
- **Purpose**: Manages panel creation, loading, switching, and cleanup
- **Role**: Service layer - orchestrates panel presenter lifecycle
- **Dependencies**: `IPanelFactory`, display/GPIO providers
- **Implements**: `IPanelService`

### `src/managers/preference_manager.cpp`
- **Purpose**: Manages application configuration persistence using ESP32 NVS (Non-Volatile Storage)
- **Role**: Model service - handles persistent application state
- **Dependencies**: ESP32 Preferences library, ArduinoJson
- **Implements**: `IPreferenceService`

### `src/managers/style_manager.cpp`
- **Purpose**: Manages LVGL styles, themes (day/night), and visual appearance
- **Role**: Service layer - provides styling for view components
- **Dependencies**: LVGL styles and color management
- **Implements**: `IStyleService`

### `src/managers/trigger_manager.cpp`
- **Purpose**: Monitors GPIO pins for state changes, manages panel/theme switching triggers
- **Role**: Controller - handles hardware events and triggers appropriate responses
- **Dependencies**: GPIO provider, panel service, style service
- **Implements**: `ITriggerService`

## Factories (Creation Layer)

### `src/factories/component_factory.cpp`
- **Purpose**: Creates component instances with dependency injection
- **Role**: Factory pattern for view component creation
- **Dependencies**: Component registrations, style/display services
- **Implements**: `IComponentFactory`

### `src/factories/panel_factory.cpp`
- **Purpose**: Creates panel instances based on panel type names
- **Role**: Factory pattern for presenter creation
- **Dependencies**: Component factory, display/GPIO providers
- **Implements**: `IPanelFactory`

### `src/factories/manager_factory.cpp`
- **Purpose**: Creates manager instances with proper dependency injection (legacy - mostly replaced by service container)
- **Role**: Factory pattern for service layer creation
- **Dependencies**: Various service interfaces

## System Infrastructure

### `src/system/service_container.cpp`
- **Purpose**: Manages service registration, resolution, and lifetime (singleton/transient)
- **Role**: Infrastructure - provides dependency injection for entire system
- **Dependencies**: STL containers, type management
- **Implements**: `IServiceContainer`

### `src/system/component_registry.cpp`
- **Purpose**: Registers and creates components, panels, and sensors with service container integration
- **Role**: Infrastructure - component creation with dependency injection
- **Dependencies**: Service container, factory functions

## Utilities

### `src/utilities/ticker.cpp`
- **Purpose**: Provides timing utilities, LVGL task handling, and frame rate management (~60fps)
- **Role**: Infrastructure utility - timing and display refresh coordination
- **Dependencies**: LVGL timer system, Arduino millis()

### `src/utilities/lv_tools.cpp`
- **Purpose**: Helper functions for screen creation and theme application
- **Role**: View utility - LVGL screen management helpers
- **Dependencies**: LVGL, style service

## Summary

This implementation follows clean architecture principles with clear separation of concerns:

1. **Hardware abstraction** through providers and the device layer
2. **Dependency injection** throughout the system via service container
3. **MVP pattern** with clear roles for models (sensors), views (components), and presenters (panels)
4. **Factory patterns** for object creation
5. **Service layer** for cross-cutting concerns like styling and preferences

The architecture enables testability, maintainability, and clean separation between hardware-specific code and business logic.