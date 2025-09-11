[![Wokwi Basic Hardware Test](https://github.com/marcelrienks/Clarity/actions/workflows/wokwi-basic-test.yml/badge.svg)](https://github.com/marcelrienks/Clarity/actions/workflows/wokwi-basic-test.yml)
[![Wokwi Full System Test](https://github.com/marcelrienks/Clarity/actions/workflows/wokwi-full-test.yml/badge.svg)](https://github.com/marcelrienks/Clarity/actions/workflows/wokwi-full-test.yml)

# Clarity
ESP32 automotive gauge system for engine monitoring with LVGL UI on round displays.

_**Note:** If all you want is an ESP32 project for displaying one or two screens that do one job, this project is sincerely over complicated for your purposes. This was a test bed for implementing the usual design patterns of OOP, and trying to see if I could make an MVP pattern work in an embedded project. This now allows for multiple screens, with multiple (reusable) components, and warning triggers.  
After I had built this architecture, the project then became a test bed for using AI agents, meaning it's far more featured and "properly" architected with extensive unit and integration testing than anyone would actually ever need. But hey, it does work._

## Documentation

### Core Documentation
- **[Testing](docs/test.md)** - Complete testing guide with Wokwi simulation and GitHub Actions
- [Architecture](docs/architecture.md) - MVP pattern and system design
- [Hardware](docs/hardware.md) - Hardware setup and configuration  
- [Requirements](docs/requirements.md) - System requirements and specifications
- [Scenarios](docs/scenarios.md) - Usage workflows and user scenarios
- [Standards](docs/standards.md) - Coding and naming standards
- [Patterns](docs/patterns.md) - Design patterns used in the project

### System Design
- [Interrupt Architecture](docs/interrupt-architecture.md) - Interrupt system design and implementation
- [Error Handling](docs/error.md) - Error management and reporting system
- [Input System](docs/input.md) - User input handling and processing
- [Sensor System](docs/sensor.md) - Sensor data acquisition and processing
- [Implementation Status](docs/implementation-status.md) - Current implementation progress

### Diagrams
- [Architecture Overview](docs/diagrams/architecture-overview.md) - High-level system architecture
- [Application Flow](docs/diagrams/application-flow.md) - Application execution flow
- [Interrupt Handling Flow](docs/diagrams/interrupt-handling-flow.md) - Interrupt processing flow

### CI/CD & Workflows
- [Workflow Setup](.github/workflows/README.md) - Detailed GitHub Actions configuration guide

## Quick Start

```bash
# Build for manual debugging (full verbose logs)
pio run -e debug-local

# Build for integration testing (clean logs)  
pio run -e test-wokwi

# Build for hardware upload
pio run -e debug-upload

# Wokwi Simulation Testing
cd test/manual && ./wokwi_debug.sh      # Interactive manual testing
cd test/wokwi && ./wokwi_run.sh         # Automated integration testing
```

### Test Coverage
- **Basic Test**: GPIO, buttons, sensors, timing validation (5 phases)
- **Full Test**: Complete system integration with all panels, triggers, animations, themes, and configuration (7 phases)
- **Automated CI/CD**: Tests run automatically on commits and pull requests

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
