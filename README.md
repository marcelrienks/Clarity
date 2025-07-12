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

## Screens list:
### Current:
* _WIP_
### TODO:
* **Oil pressure gauge**  
Displaying an analogue oil pressure gauge, key check symbol, and immobiliser symbol. Fully replacing the stock gauge design of the car (specific to Mazda mx5 NC)
* **Oil pressure warning**

## Components list:
### Current:
* _WIP_
### TODO:
* **Oil pressure gauge**
* **Oil pressure warning**
* **Key check**
* **Immobiliser check**

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

## Known Issues

### Architecture: Trigger System Design Conflicts

The current trigger system creates architectural challenges that break the MVC pattern:

**Current problematic flow:**
```
main > panelmanager >> trigger >> panel > component
```

**Issues:**
1. **MVC Pattern Violation**: Triggers bypass the presenter layer (panels) and directly control panel switching
2. **Circular Dependencies**: Triggers that use sensors create circular references when sensors get reinitialized from panels
3. **Instantiation Requirements**: To maintain MVC pattern, all panels would need to be instantiated to gain access to their triggers:
   ```
   main > panelmanager >> panel >> component / sensor
   main > panelmanager >> panel >> component / trigger
   ```

**Potential Solutions:**
- **Option A**: Redesign panels as widgets where oil/clarity become components, and triggers load separate display contexts
- **Option B**: Move trigger logic into a separate service layer that communicates with panels through events
- **Option C**: Implement a mediator pattern where triggers publish events and panels subscribe to relevant state changes

This architectural decision impacts scalability and maintainability as the system grows.
