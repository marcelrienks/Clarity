# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Clarity is an ESP32-based digital gauge system for automotive engine monitoring, built with PlatformIO and using LVGL for UI and LovyanGFX for display drivers. It's designed for a 1.28" round display (GC9A01 driver) on a NodeMCU-32S development board.

## Development Commands

Build the project:
```bash
pio.exe run
```

Quick compilation test (faster for testing changes):
```bash
pio.exe run -e debug-local    # Fastest build environment for testing
```

Check program size (quick verification):
```bash
pio.exe run --target size
```

Build and upload to device:
```bash
pio.exe run --target upload
```

Build specific environments:
```bash
pio.exe run -e debug-local    # Debug build for local testing (fastest)
pio.exe run -e debug-upload   # Debug build with inverted colors for waveshare display
pio.exe run -e release        # Optimized release build with inverted colors
```

## Architecture

The codebase follows an MVP (Model-View-Presenter) pattern with a layered architecture:

**Device → Display → Panels → Components**

- **Device** (`device.h/cpp`): Hardware abstraction layer managing the display and LVGL integration
- **Panels** (`panels/`): Presenters that coordinate between sensors and components, managing screen layouts
- **Components** (`components/`): Views that render UI elements (gauges, indicators, etc.)
- **Sensors** (`sensors/`): Models that handle data acquisition from hardware inputs
- **Managers**: 
  - `PanelManager`: Handles panel switching and lifecycle
  - `PreferenceManager`: Manages persistent settings
  - `StyleManager`: Manages LVGL styling and themes

## Key Interfaces

- `IDevice`: Hardware abstraction for display devices
- `IPanel`: Interface for screen/panel implementations with init/load/update lifecycle
- `IComponent`: Interface for UI components with render_load/render_update methods
- `ISensor`: Interface for sensor data acquisition

## Build Configuration

The project uses custom partitioning (`partitions.csv`) optimized for the ESP32-WROOM-32 with 4MB flash:
- OTA support with dual app partitions
- FAT filesystem partition for assets
- Core dump partition for debugging

Key build flags:
- `CLARITY_DEBUG`: Enables debug logging
- `INVERT`: Inverts display colors for waveshare displays
- Custom LVGL memory configuration (120KB buffer)

## Display Configuration

Target hardware: 240x240 round GC9A01 display
- SPI interface on SPI2_HOST
- Hardware pins defined in `device.h`
- LVGL buffer sized for 60-line dual buffering

## Development Notes

- Local testing is done via a tool called wokwi to emulate a display, but it can only offer a square display rather than the round display that this application is intended for. And a limitation of that display is it renders the image inverted horizontally

## Testing Limitations

- There seems to be some type of limitation with PlatformIO where build filters are not taken into account with unity tests and they will always try to link multiple test files together.
- There is an issue with PlatformIO and Unity where test files within nested directories are not found and only tests in the root test directory are run