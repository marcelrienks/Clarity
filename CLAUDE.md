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

For architecture details, see: `docs/architecture.md`

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
- Hardware pins defined in `device_provider.h`
- LVGL buffer sized for 60-line dual buffering

## Development Notes

- Local testing is done via a tool called wokwi to emulate a display, but it can only offer a square display rather than the round display that this application is intended for. And a limitation of that display is it renders the image inverted horizontally

## Testing Limitations

- There seems to be some type of limitation with PlatformIO where build filters are not taken into account with unity tests and they will always try to link multiple test files together.
- There is an issue with PlatformIO and Unity where test files within nested directories are not found and only tests in the root test directory are run