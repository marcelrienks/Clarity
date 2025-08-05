[![Test Clarity Gauge System](https://github.com/marcelrienks/Clarity/actions/workflows/test.yml/badge.svg)](https://github.com/marcelrienks/Clarity/actions/workflows/test.yml)

# Clarity
An ESP32 project, using platformio, which builds a custom digital gauge for monitoring and displaying your engines key parameters, on various screen configurations and combinations of components and sensors.

_**Note:** If all you want is an ESP32 project for displaying one or two screens that do one job, this project is sincerely over complicated for your purposes. This was a test bed for implementing the usual design patterns of OOP, and trying to see if I could make an MVP pattern work in an embedded project. This now allows for multiple screens, with multiple (reusable) components, and warning triggers.  
After I had built this architecture, the project then became a test bed for using AI agents, meaning it's far more featured and "properly" architected with extensive unit and integration testing than anyone would actually ever need. But hey, it does work._

## Documentation

### Architecture
[Architecture Documentation](docs/architecture.md)

Comprehensive overview of the MVP (Model-View-Presenter) pattern implementation, system layers, component relationships, and hardware abstraction. Covers the Device → Display → Panels → Components architecture with detailed explanations of roles, responsibilities, and service patterns.

### Testing  
[Testing Documentation](docs/test.md)

Complete testing guide covering Unity framework integration, PlatformIO limitations, test environment configuration, and detailed commands for running different test suites. Includes workarounds for single-file testing constraints and mock system architecture.

This project includes comprehensive unit tests for core logic and components. See [Testing Documentation](docs/test.md) for complete details on PlatformIO/Unity limitations and advanced testing features.

#### Quick Test Commands

Execute all unit tests:
```bash
pio.exe test -e test-all
```

Execute specific test suites:
```bash
pio.exe test -e test-sensors       # Sensor tests only
pio.exe test -e test-managers-core # Manager tests only  
pio.exe test -e test-components    # Component tests only
```

#### Test Coverage

The test suite covers:
- **Timing/Ticker Logic** - Frame timing calculations and dynamic delay handling
- **Sensor Logic** - Value change detection, ADC conversions, key state determination  
- **Configuration Management** - Settings persistence, validation, and default creation

#### Scenarios
[Scenario Documentation](docs/scenario.md)

Usage scenarios and operational workflows for the automotive gauge system.

### Development Tasks
[Todo Items](docs/todo.md)

Current development tasks and improvement backlog.

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
