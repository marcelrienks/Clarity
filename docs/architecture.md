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

## Service Architecture

Dependency injection via service container enables loose coupling and testability.