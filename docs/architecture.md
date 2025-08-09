# Architecture

## Overview

MVP (Model-View-Presenter) pattern for ESP32 automotive gauges with layered architecture and interface-based design.

## Pattern Structure

```
Device → Display → Panels → Components
                                       ↓ ↑
                                    Sensors
```

### MVP Mapping
- **Models**: Sensors - hardware data acquisition
- **Views**: Components - UI rendering
- **Presenters**: Panels - orchestration and business logic

## Core Layers

### Device
Hardware abstraction for display (GC9A01) and LVGL integration.

### Panels (Presenters)
- Coordinate sensors and components
- Handle lifecycle: init → load → update
- Implement panel switching logic

### Components (Views)  
- Render UI elements (gauges, indicators)
- No business logic
- LVGL object management

### Sensors (Models)
- Abstract GPIO/ADC inputs
- Provide clean data interfaces

## Managers

- **PanelManager**: Panel switching and lifecycle
- **PreferenceManager**: Persistent settings
- **StyleManager**: LVGL themes
- **TriggerManager**: Sensor-based panel switching

## Panel Switching

Priority-based triggers:
1. Key state → Key Panel
2. Lock active → Lock Panel  
3. Default → Oil Panel

Light sensor triggers theme changes (no panel switch).

## Available Panels

- **Splash**: Startup animation
- **Oil**: Dual gauge monitoring (pressure/temperature)
- **Key**: Key presence indicator
- **Lock**: Security status

## Hardware Configuration

### Display
- GC9A01 240x240 round
- SPI2_HOST interface
- 120KB dual buffer

### Inputs
- Digital: Key, Lock, Light sensors (GPIO)
- Analog: Oil pressure/temperature (ADC)

## Key Interfaces

- `IDevice`: Display hardware abstraction
- `IPanel`: Screen implementation (init/load/update)
- `IComponent`: UI element rendering
- `ISensor`: Data acquisition

## History:

Issues and previous architectures that have decided the direction of this project.

## Trigger Interrupts and Input

Attempted in the past to use hardware GPIO interrupts to log messages on queues, in separate core threads, but ultimately this caused mass complexity with sharing state between the cores, and processing through all messages to set up a previous state for logic etc. Ultimately decided on simply checking all GPIO state during main core thread and LVGL idle state. This does mean that triggers are not processed immediately, and there is a delay, but this is minimal, and the architecture is much more reliable.

Also interrupting LVGL and animations causes non stop crashing errors and complexity that does not justify the immediate reaction, both error logs, and input can wait to be processed during idle time rather.

<This section needs to be expanded on>

## Service Architecture

Dependency injection via service container enables loose coupling and testability.

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