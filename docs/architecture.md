# Architecture

## Overview

Clarity follows an MVP (Model-View-Presenter) pattern with a layered architecture designed for ESP32-based automotive gauge systems. The architecture promotes separation of concerns and testability through well-defined interfaces.

## Patterns

### MVP Relationships

- **Models**: Sensors (`sensors/`) handle data acquisition from hardware inputs
- **Views**: Components (`components/`) render UI elements and handle visual presentation  
- **Presenters**: Panels (`panels/`) coordinate between sensors and components, managing screen layouts and business logic

### Layer Structure

```
Device → Display → Panels → Components
                     ↓ ↑
                  Sensors
```

**Device Layer** (`device.h/cpp`): Hardware abstraction layer managing the display and LVGL integration

**Panel Layer** (`panels/`): Presenters that orchestrate UI state and coordinate data flow

**Component Layer** (`components/`): Views responsible for rendering specific UI elements

**Sensor Layer** (`sensors/`): Models that abstract hardware input sources

## Roles and Responsibilities

### Device
- Initialize and manage display hardware (GC9A01 via SPI)
- LVGL integration and buffer management
- Hardware abstraction for display operations

### Panels (Presenters)
- Coordinate between sensors and components
- Manage panel lifecycle (init → load → update)
- Handle panel-specific business logic
- Trigger panel switching based on sensor states

### Components (Views)
- Render UI elements (gauges, indicators, icons)
- Handle visual updates without business logic
- Implement render_load() and render_update() methods
- Manage LVGL objects and styling

### Sensors (Models)
- Abstract hardware input sources (GPIO, ADC)
- Provide clean data interfaces to panels
- Handle hardware-specific reading logic

### Managers
- **PanelManager**: Handles panel switching, lifecycle management
- **PreferenceManager**: Manages persistent settings storage
- **StyleManager**: Manages LVGL styling and theme switching
- **TriggerManager**: Coordinates sensor-based panel switching logic

## Logic Flow

### Initialization
1. Device initializes display hardware and LVGL
2. Service container registers all managers and services
3. UI factory creates panels and components
4. Panel manager loads initial panel (splash)

### Runtime Loop
1. Sensor readings are acquired
2. Trigger manager evaluates sensor states
3. Panel manager switches panels based on triggers
4. Active panel updates components with new data
5. Components render visual updates
6. Process repeats

## Dynamic Panel Switching

Panel switching is driven by sensor inputs through the trigger system:

### Trigger Priorities
1. **Key Present/Not Present**: Highest priority
2. **Lock State**: Medium priority  
3. **Oil Monitoring**: Default/fallback panel

### Switch Logic
```
Key Present OR Key Not Present → Key Panel
Lock Active → Lock Panel  
Default → Oil Panel
```

### Based on Sensor Input
- **Light Sensor**: Triggers day/night theme switching (no panel reload)
- **Key Sensor**: Triggers key panel when key state changes
- **Lock Sensor**: Triggers lock panel when lock is active
- **Oil Sensors**: Provide data to oil panel (pressure, temperature)

## Trigger System Interrupts

The trigger system uses interrupt-driven sensor monitoring:

### Interrupt Sources
- GPIO state changes (key, lock sensors)
- Analog threshold crossings (oil pressure, temperature)
- Timer-based polling for non-interrupt sensors

### Trigger Processing
1. Sensor interrupt occurs
2. TriggerManager evaluates new state
3. Panel switching decision made based on priority
4. PanelManager executes panel transition
5. New panel loads and begins updating

## Panels

### Splash Panel
- Initial startup screen
- Animated logo/branding
- Transitions to primary panel after delay

### Oil Panel  
- Primary monitoring interface
- Dual gauge display (pressure + temperature)
- Animated needle movements
- Theme-aware styling

### Key Panel
- Key presence indicator
- Visual feedback (green=present, red=absent)
- Auto-timeout to return to oil panel

### Lock Panel
- Lock state visualization
- Security status indication
- Context-sensitive display

## Components

### Clarity Component
- Base component with common functionality
- Theme management integration
- Standard lifecycle methods

### Oil Components
- **OEM Oil Component**: Base oil monitoring component
- **Oil Pressure Component**: Pressure gauge rendering
- **Oil Temperature Component**: Temperature gauge rendering

### Key Component
- Key presence/absence visualization
- Icon-based status display
- Color-coded state indication

### Lock Component  
- Lock state visualization
- Security icon management
- Status indication

## Hardware Configuration

### GPIO Pins
Defined in `hardware/gpio_pins.h`:
- Digital inputs for key/lock sensors
- Analog inputs for oil pressure/temperature
- SPI pins for display communication

### Display/LCD Interface (SPI)
- **Controller**: GC9A01 (240x240 round display)
- **Interface**: SPI2_HOST
- **Pins**: Configurable CS, DC, RST, SCK, MOSI
- **Buffer**: Dual 60-line buffers (120KB total)

### Sensor Inputs

#### Digital Inputs
- Key present sensor (GPIO)
- Key not present sensor (GPIO)  
- Lock state sensor (GPIO)
- Light sensor (GPIO/interrupt)

#### Analog Inputs
- Oil pressure sensor (ADC)
- Oil temperature sensor (ADC)
- Configurable voltage dividers and scaling

## Key Interfaces

### IDevice
Hardware abstraction for display devices
- `init()`: Initialize display hardware
- `update()`: Process display updates
- `get_display()`: Access LVGL display object

### IPanel
Interface for screen/panel implementations
- `init()`: One-time panel setup
- `load()`: Panel activation and initial render
- `update()`: Periodic updates with sensor data

### IComponent  
Interface for UI components
- `render_load()`: Initial component rendering
- `render_update()`: Update component with new data

### ISensor
Interface for sensor data acquisition
- `read()`: Get current sensor value
- `is_valid()`: Check sensor health/validity

## Service Architecture

The system uses dependency injection through a service container:

### Service Container
- Central registry for all services and managers
- Enables loose coupling between components
- Facilitates testing through interface mocking

### Factory Pattern
- **UIFactory**: Creates panels and components
- **ManagerFactory**: Creates and configures managers
- Encapsulates complex object creation logic

This architecture enables maintainable, testable code with clear separation of concerns and flexible hardware abstraction.