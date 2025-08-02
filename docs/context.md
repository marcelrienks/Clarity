# Project Context

## Current Status
- **Branch**: test
- **Last Commit**: d5fa4dd in12
- **Modified Files**: 120 files changed
- **Working Directory**: `/mnt/c/Users/marcelr/Dev/github/Clarity/.pio/build/test`

## Project Overview
Clarity is an ESP32-based digital gauge system for automotive engine monitoring, built with PlatformIO using LVGL for UI and LovyanGFX for display drivers. It's designed for a 1.28" round display (GC9A01 driver) on a NodeMCU-32S development board.

## Recent Activity
```
d5fa4dd in12
dee67d1 in11
411c987 in10
06fcc47 in9
8829655 t
```

## Current State
The project is in active development with extensive modifications across the entire codebase:
- 120 files currently modified
- All major components have changes: sensors, panels, managers, components
- Test infrastructure has been updated
- GitHub workflows and CI/CD configurations modified
- Documentation updates in progress

## Architecture Summary
- **Device Layer**: Hardware abstraction (`device.h/cpp`) - manages display and LVGL integration
- **Panel Layer**: Screen management (`panels/`) - presenters coordinating sensors and components
- **Component Layer**: UI elements (`components/`) - views rendering gauges and indicators
- **Sensor Layer**: Data acquisition (`sensors/`) - models handling hardware inputs
- **Manager Layer**: System coordination
  - `PanelManager`: Panel switching and lifecycle
  - `PreferenceManager`: Persistent settings
  - `StyleManager`: LVGL styling and themes
  - `TriggerManager`: Event handling

## Key Interfaces
- `IDevice`: Hardware abstraction for display devices
- `IPanel`: Screen/panel implementations with init/load/update lifecycle
- `IComponent`: UI components with render_load/render_update methods
- `ISensor`: Sensor data acquisition interface

## Build Configuration
- Target: ESP32-WROOM-32 with 4MB flash
- Custom partitioning with OTA support
- LVGL with 120KB buffer for 240x240 round GC9A01 display
- Debug flags: `CLARITY_DEBUG`, `INVERT` for display variations

## Next Steps Available
1. **Continue development** on current branch: `test`
2. **Build project**: `pio.exe run -e debug-local` (fastest for testing)
3. **Run tests**: Comprehensive test suite available in `/test` directory
4. **Review and commit**: 120 modified files ready for staging
5. **Architecture refinement**: Core systems appear to be under active development

## Development Commands
```bash
# Quick compilation test (fastest)
pio.exe run -e debug-local

# Build specific environments
pio.exe run -e debug-upload   # Debug with inverted colors for waveshare
pio.exe run -e release        # Optimized release build

# Build and upload
pio.exe run --target upload

# Check program size
pio.exe run --target size
```

## Current Focus
Based on the extensive modifications (120 files), the project appears to be undergoing significant development or refactoring. Key areas of activity include:

1. **Core Components**: All major component types have modifications
2. **Sensor Integration**: Complete sensor layer updates
3. **Test Infrastructure**: Comprehensive test coverage being implemented
4. **Build System**: CI/CD and workflow improvements
5. **Documentation**: Architecture and context documentation updates

The project is in an active development phase with substantial changes across all layers of the architecture. The test branch suggests this is experimental or feature development work.

*Generated: 2025-08-02*