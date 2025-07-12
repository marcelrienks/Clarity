[![Build ESP32 Project with PlatformIO](https://github.com/marcelrienks/Clarity/actions/workflows/c-cpp.yml/badge.svg)](https://github.com/marcelrienks/Clarity/actions/workflows/c-cpp.yml)

# Clarity
An ESP32 project, using platformio, which builds a custom digital gauge for monitoring and displaying your engines key parameters, on various screen configurations and combinations of components and sensors.

_**Note:** this project is over complicated for general use cases of just creating a screen to do one task. This is intended to follow proper patterns and allow for easy implementation of multiple screens, and extension of more screens, which can be configured to show in a loop._

## Main Libraries:
* Arduino
* [LVGL](https://docs.lvgl.io/master/)
* [LovyanGFX](https://docs.arduino.cc/libraries/lovyangfx/)

## Architecture:
Conceptually there is one `device`, with one `display` (this is based on lvgl structure), this can show one of many `panel` configurations, each of which having one or many `component(s)` shown together. _Note: components can be part of many panel configurations (many to many relationship)_
> Device > Display > Panels > Components

This is designed around an MVP pattern.  
A sensor will represent the model, and be responsible for handling data reading. A component will represent the view, and be responsible for building the view for the sensor data. And the panel will represent the presenter, and will be responsible for building and displaying one or more components at once.
> Sensor(model) <> Panel(presenter) <> Component(view)  

Based on an input (either from sensor signal or from a button press) the device/display will load different screens. For example an emergency panel can be shown when a sensor signal moves beyond a threshold, replacing the panel that was configured to display using a button input which normally cycles through multiple screen configurations.

_This would also allow for future extension with more panels configurations and sensors._

## Panels (Screens)

### **OemOilPanel** 
**Primary dashboard displaying engine oil monitoring**
- **Components**: OemOilComponent (pressure gauge), OemOilTemperatureComponent (temperature gauge)
- **Sensors**: OilPressureSensor, OilTemperatureSensor  
- **Display**: Dual circular gauges with animated value updates
- **Role**: Default panel, restoration target for trigger system
- **Features**: Real-time sensor data, smooth animations, threshold monitoring

### **KeyPanel**
**Key/ignition status indicator**
- **Component**: KeyComponent (key status icon)
- **Sensor**: KeySensor (via KeyTrigger)
- **Display**: Key icon with state-based coloring
- **Trigger**: KeyTrigger (GPIO pins 25 & 26)
- **States**: 
  - Present: Green key icon
  - Not Present: Red key icon  
  - Invalid (both pins HIGH): No panel load
- **Features**: Automatic panel switching, restoration on clear

### **LockPanel**
**Vehicle lock/immobilizer status**
- **Component**: LockComponent (lock status display)
- **Sensor**: LockSensor (via LockTrigger)  
- **Trigger**: LockTrigger (GPIO pin 27)
- **Display**: Lock status indication
- **Features**: Trigger-driven activation, priority-based switching

### **SplashPanel**
**Startup/transition screen**
- **Purpose**: Loading screen during panel transitions
- **Usage**: Smooth visual transitions between panels
- **Features**: Branding display, initialization feedback

## Components

### **OemOilComponent**
**Primary oil pressure gauge display**
- **Type**: Circular gauge with needle indicator
- **Range**: Configurable pressure range with color zones
- **Features**: Smooth animations, threshold-based styling
- **Integration**: Direct sensor binding for real-time updates

### **OemOilTemperatureComponent** 
**Oil temperature gauge display**
- **Type**: Circular gauge with temperature mapping
- **Features**: Value mapping, color-coded zones, animated updates
- **Integration**: Temperature sensor data processing

### **KeyComponent**
**Key status icon display**
- **Type**: SVG-based key icon with state colors
- **States**: Present (green), Not Present (red), Inactive (hidden)
- **Features**: State-based color changes, clean icon rendering
- **Integration**: KeySensor data via panel coordination

### **LockComponent**
**Lock/immobilizer status display**  
- **Type**: Lock status indicator
- **Features**: Binary state display, clear visual feedback
- **Integration**: LockSensor data via trigger system

### **ClarityComponent**
**Base component providing common functionality**
- **Type**: Abstract base class for component inheritance
- **Features**: Common rendering pipeline, shared utilities
- **Usage**: Foundation for all display components

## Component Parts:
These are the components used for this specific project, but the code can be adjusted to suit any combination of microcontroller and lcd display.
* NodeMCU-32
* 1.28" round display (GC9A01 driver)

## Partitions:
This is specific to an NodeMCU-32S dev board, using a ESP32-WROOM-32 (ESP32-D0WD-V3 chip) with 32Mb(4MB) flash
> - **nvs:** Minimal non-volatile storage for key-value pairs (12 KB).
> - **otadata:** OTA data partition (8 KB).
> - **ota_0:** Application partition for OTA updates (2 MB).
> - **coredump:** Minimal core dump partition (64 KB).
> - **fat:** FAT filesystem partition using the remaining space (1.875 MB).

## LVGL guidelines
### Screens & Widgets:
Screen Loading is for:
- Panel switching (Oil Panel → Key Panel)
- Major layout changes

Widget Updates are for:
- Data changes within the same panel
- Status updates (key present/absent)

### LVGL Screen Memory Model:
1. Multiple screens can exist in memory simultaneously - Creating screen B doesn't delete screen A
2. Only one screen is active/displayed at a time - When you call lv_screen_load(screen_B), screen A is no longer visible
3. Inactive screens are NOT rendered - LVGL only renders the currently active screen, so screen A stops being drawn/updated
4. Memory remains allocated - Screen A and all its widgets stay in memory until explicitly deleted with lv_obj_del(screen_A)

## Credits:
This project took inspiration from a product that [Rotarytronics](https://www.rotarytronics.com/) has already built and can be purchased online.

The code is also based on other projects from:
* [Garage Tinkering](https://github.com/valentineautos)
* [fbiego](https://github.com/fbiego)

As well as contributions from [Eugene Petersen](https://github.com/gino247)

## Panel-Trigger Architectural Patterns

The system implements several well-established design patterns to coordinate between panels and triggers:

### 🏗️ Core Architectural Patterns

#### **MVP (Model-View-Presenter) Pattern**
- **Model**: Sensors (KeySensor, LockSensor, OilPressureSensor, etc.)
- **View**: Components (KeyComponent, LockComponent, OemOilComponent, etc.) 
- **Presenter**: Panels (KeyPanel, LockPanel, OemOilPanel, etc.)

#### **Observer Pattern (Trigger System)**
- **Subject**: InterruptManager continuously evaluates triggers
- **Observers**: Triggers (KeyTrigger, LockTrigger) monitor sensor states
- **Notification**: Panel switches when trigger conditions are met

### 🔄 Panel-Trigger Interaction Patterns

#### **Pattern 1: Direct Panel-to-Sensor Binding**
**Example**: OemOilPanel ↔ Oil Sensors
```
OemOilPanel → directly reads from → OilPressureSensor, OilTemperatureSensor
```
- Panel directly owns and updates from sensors
- No trigger involvement for normal operation
- Used for continuous data display

#### **Pattern 2: Trigger-Driven Panel Switching**
**Example**: KeyTrigger → KeyPanel, LockTrigger → LockPanel
```
Sensor → Trigger → InterruptManager → PanelManager → Panel
```
- Sensors provide data
- Triggers evaluate conditions and decide when to switch
- Triggers specify target panels
- Global interrupt system manages the switching

#### **Pattern 3: Sensor Abstraction Layer**
**Consistent Pattern**:
```
Trigger → Sensor (abstraction) → Hardware (GPIO pins)
```
- **KeyTrigger** uses **KeySensor** 
- **LockTrigger** uses **LockSensor**
- Sensors encapsulate hardware details (GPIO pins, debouncing, etc.)
- Triggers focus on business logic (when to switch panels)

### 🎯 Trigger Management Patterns

#### **Priority-Based Evaluation**
```
InterruptManager evaluates triggers by priority:
0 = KeyTrigger (highest priority)
1 = LockTrigger (lower priority)
```
- Higher priority triggers can override lower priority ones
- Active trigger prevents lower priority triggers from activating

#### **State Machine Pattern**
```
Inactive → Active → Restoration
```
- **Inactive**: No trigger conditions met
- **Active**: Trigger condition met, panel switched
- **Restoration**: Trigger cleared, restore previous panel

#### **Factory Pattern**
```cpp
PanelManager.register_panel<KeyPanel>("KeyPanel");
InterruptManager.register_trigger<KeyTrigger>("key_trigger");
```
- Dynamic registration of panels and triggers
- Template-based type-safe creation

### 📋 Data Flow Patterns

#### **Continuous Monitoring Flow (Oil Panel)**
```
Sensors → continuous readings → Panel → Components → UI Update
```

#### **Event-Driven Flow (Key/Lock)**
```
Hardware Event → Sensor → Trigger → InterruptManager → PanelManager → Panel Switch
```

#### **Restoration Flow**
```
Trigger Clears → InterruptManager → PanelManager.get_restoration_panel() → Restore Previous Panel
```

### 🔧 Design Patterns Used

#### **Singleton Pattern**
- `PanelManager::get_instance()`
- `InterruptManager::get_instance()`
- Central coordination points

#### **Strategy Pattern**
- Different triggers implement `ITrigger` interface
- Different panels implement `IPanel` interface
- Pluggable behavior for different hardware states

#### **Template Method Pattern**
- Panel lifecycle: `init() → load() → update() → callbacks`
- Trigger lifecycle: `init() → evaluate() → get_target_panel()`

#### **Dependency Injection**
- Triggers receive sensors in constructor/as members
- Panels receive sensors for data updates
- Loose coupling between components

### 🎭 Specific Panel-Trigger Relationships

#### **KeyPanel ↔ KeyTrigger**
- **Trigger owns**: KeySensor instance
- **Panel owns**: KeyComponent (for display)
- **Data flow**: KeySensor → KeyTrigger (logic) + KeyPanel → KeyComponent (display)
- **Triggering**: Any key state change (Present/NotPresent) → load KeyPanel

#### **LockPanel ↔ LockTrigger**
- **Trigger owns**: LockSensor instance  
- **Panel owns**: LockComponent (for display)
- **Triggering**: Lock engaged (HIGH) → load LockPanel

#### **OemOilPanel (No Direct Trigger)**
- **Panel owns**: Oil sensors directly
- **Role**: Default/restoration panel
- **Triggering**: Restoration target when other triggers clear

### 🔄 Lifecycle Coordination

#### **Startup Flow**
```
1. PanelManager.init()
2. Register all panels with factory
3. Register all triggers with InterruptManager  
4. Load default panel (OemOilPanel)
5. Start continuous trigger evaluation
```

#### **Runtime Flow**
```
1. Continuous sensor monitoring
2. Trigger evaluation every loop
3. Panel switching on trigger activation
4. Panel restoration when triggers clear
5. Continuous UI updates within active panel
```

This architecture provides clean separation of concerns, with sensors handling hardware, triggers handling business logic for panel switching, and panels handling presentation logic.
