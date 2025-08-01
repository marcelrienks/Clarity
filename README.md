[![Test Clarity Gauge System](https://github.com/marcelrienks/Clarity/actions/workflows/test.yml/badge.svg)](https://github.com/marcelrienks/Clarity/actions/workflows/test.yml)

# Clarity
An ESP32 project, using platformio, which builds a custom digital gauge for monitoring and displaying your engines key parameters, on various screen configurations and combinations of components and sensors.

_**Note:** If all you want is an ESP32 project for displaying one or two screens that do one job, this project is sincerely over complicated for your purposes. This was a test bed for implementing the usual design patterns of OOP, and trying to see if I could make an MVP pattern work in an embedded project. This now allows for multiple screens, with multiple (reusable) components, and warning triggers.  
After I had built this architecture, the project then became a test bed for using AI agents, meaning it's far more featured and "properly" architected with extensive unit and integration testing than anyone would actually ever need. But hey, it does work._

## Todo:
* 1. Create Arch doc (and link it in this readme under the existing architecture section):
   * include patterns:
      * MVP Relationships
      * Layer Structure
   * include roles and responsibilities
   * include logic flow
   * dynamic panel switching based on sensor input
   * trigger system interrupts
   * panels
   * components
   * hardware config
      * GPIO pins
      * Display/LCD Interface (SPI)
      * Sensor Inputs (Digital + Analog)
* 2. Create Scenario doc (and link it in this readme under the existing Scenario section):
   * Major scenario that tests all aspects in one  
   app starts with pressure and temperate values set to half way > splash animates with day theme (white text) > oil panel loads with day theme (white scale ticks and icon) > oil panel needles animates > lights trigger high = oil panel does NOT reload, simply changes theme to night (red scale ticks and icon) > lock trigger high = lock panel > key not present trigger high = key panel (present = false means red icon) > key present trigger high = lock panel (both key present and key not present true = invalid state) > key not present trigger low = key panel (present = true means green icon because key present trigger still high) > key present trigger low = lock panel (because lock panel still high) > lock trigger low = oil panel loads with night theme (red scale ticks and icon because lights trigger still active) > oil panel needles animates > lights trigger low = oil panel does NOT reload, simply theme changes to day (white scale ticks and icon)
   * Individual test scenarios:
      * app starts > splash animates with day theme (white text)
      * app starts with pressure and temperate values set to half way > splash animates with day theme (white text) > oil panel loads with day theme (white scale ticks and icon) > oil panel needles animates
      * app starts > splash animates with day theme (white text) > oil panel loads with day theme (white scale ticks and icon) > key present trigger high = key panel loads (present = true means green icon) > key present trigger low = oil panel loads
      * app starts with key present trigger high > splash animates > oil panel does NOT load, key panel loads (present = true means green icon) > key present trigger low = oil panel loads
      * app starts > splash animates with day theme (white text) > oil panel loads with day theme (white scale ticks and icon) > key not present trigger high = key panel loads (present = false means red icon) > key not present trigger low = oil panel loads
      * app starts with key not present trigger high > splash animates > oil panel does NOT load, key panel loads (present = false means red icon) > key not present trigger low = oil panel loads
      * app starts > splash animates with day theme (white text) > oil panel loads with day theme (white scale ticks and icon) > lock trigger high = lock panel loads > lock trigger low = oil panel loads
      * app starts with lock trigger high > splash animates > oil panel does NOT load, lock panel loads > lock trigger low = oil panel loads
      * app starts > splash animates with day theme (white text) > oil panel loads with day theme (white scale ticks and icon) > lights trigger high = oil panel does NOT reload, simply theme changes to night (red scale ticks and icon) > lights trigger low = oil panel does NOT reload, simply theme changes to day (white scale ticks and icon)
      * app starts with lights trigger high > splash animates with night theme (red text) > oil panel loads with night theme (red scale ticks and icon) > lights trigger low = oil panel does NOT reload, simply theme changes to day (white scale ticks and icon)
* 3. Create unit tests
* 4. Create integration tests (is it possible to do so to prove scenarios?)
* 5. Create wokwi tests based on scenarios

## Main Libraries:
* Arduino
* [LVGL](https://docs.lvgl.io/master/)
* [LovyanGFX](https://docs.arduino.cc/libraries/lovyangfx/)

## Architecture:

<architecture.md>

## Scenarios:

<scenario.md>

## Credits

This project took inspiration from a product that [Rotarytronics](https://www.rotarytronics.com/) has already built and can be purchased online.

The code is also based on other projects from:
* [Garage Tinkering](https://github.com/garagetinkering)
* [fbiego](https://github.com/fbiego)

As well as contributions and criticism from [Eugene Petersen](https://github.com/gino247)
