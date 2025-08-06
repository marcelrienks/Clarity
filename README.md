[![Unity Tests](https://github.com/marcelrienks/Clarity/actions/workflows/test.yml/badge.svg)](https://github.com/marcelrienks/Clarity/actions/workflows/test.yml)
[![Wokwi Integration Tests](https://github.com/marcelrienks/Clarity/actions/workflows/wokwi-tests.yml/badge.svg)](https://github.com/marcelrienks/Clarity/actions/workflows/wokwi-tests.yml)

# Clarity
ESP32 automotive gauge system for engine monitoring with LVGL UI on round displays.

_**Note:** If all you want is an ESP32 project for displaying one or two screens that do one job, this project is sincerely over complicated for your purposes. This was a test bed for implementing the usual design patterns of OOP, and trying to see if I could make an MVP pattern work in an embedded project. This now allows for multiple screens, with multiple (reusable) components, and warning triggers.  
After I had built this architecture, the project then became a test bed for using AI agents, meaning it's far more featured and "properly" architected with extensive unit and integration testing than anyone would actually ever need. But hey, it does work._

## Documentation

- [Architecture](docs/architecture.md) - MVP pattern and system design
- [Testing](docs/test.md) - Unit and integration test guide  
- [Scenarios](docs/scenario.md) - Usage workflows
- [Todo](docs/todo.md) - Development tasks

## Quick Start

```bash
# Build
pio.exe run -e debug-local

# Test
pio.exe test -e test-all          # All 101 tests
pio.exe test -e test-sensors      # Sensor tests
pio.exe test -e test-components   # Component tests

# Wokwi Integration Tests
./run_wokwi_tests.sh              # Linux/Mac
run_wokwi_tests.bat               # Windows
```

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
