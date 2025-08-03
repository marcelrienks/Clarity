[![Test Clarity Gauge System](https://github.com/marcelrienks/Clarity/actions/workflows/test.yml/badge.svg)](https://github.com/marcelrienks/Clarity/actions/workflows/test.yml)

# Clarity
An ESP32 project, using platformio, which builds a custom digital gauge for monitoring and displaying your engines key parameters, on various screen configurations and combinations of components and sensors.

_**Note:** If all you want is an ESP32 project for displaying one or two screens that do one job, this project is sincerely over complicated for your purposes. This was a test bed for implementing the usual design patterns of OOP, and trying to see if I could make an MVP pattern work in an embedded project. This now allows for multiple screens, with multiple (reusable) components, and warning triggers.  
After I had built this architecture, the project then became a test bed for using AI agents, meaning it's far more featured and "properly" architected with extensive unit and integration testing than anyone would actually ever need. But hey, it does work._

## Todo:
[Items to do](docs/todo.md)

* review and remove historical comments that only state what was done during refactoring or changes

## Architecture:

[Architecture Documentation](docs/architecture.md)

## Scenarios:

[Scenario Documentation](docs/scenario.md)

## Testing

This project includes comprehensive unit tests for core logic and components.

### Running Tests

Execute all unit tests:
```bash
pio.exe test -e test
```

### Test Coverage

The test suite covers:
- **Timing/Ticker Logic** - Frame timing calculations and dynamic delay handling
- **Sensor Logic** - Value change detection, ADC conversions, key state determination  
- **Configuration Management** - Settings persistence, validation, and default creation

### Test Results

Tests run on the native platform (not requiring ESP32 hardware) and provide fast feedback on core functionality:
```
12 test cases: 12 succeeded in 00:00:08.063
```

### Test Implementation

Tests are implemented using the Unity testing framework and focus on business logic without ESP32/LVGL dependencies. See `/test/test_all.cpp` for the complete test implementation.

## Main Libraries:
* Arduino
* [LVGL](https://docs.lvgl.io/master/)
* [LovyanGFX](https://docs.arduino.cc/libraries/lovyangfx/)
## Credits

This project took inspiration from a product that [Rotarytronics](https://www.rotarytronics.com/) has already built and can be purchased online.

The code is also based on other projects from:
* [Garage Tinkering](https://github.com/garagetinkering)
* [fbiego](https://github.com/fbiego)

As well as contributions and criticism from [Eugene Petersen](https://github.com/gino247)
