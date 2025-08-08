# Project Context

## Current Status
- **Branch**: input
- **Last Commit**: 7117e46 ph1
- **Modified Files**: 21 files changed
- **Build Status**: ❌ Experiencing heap corruption crashes during runtime

## Project Overview
Clarity is an ESP32-based digital gauge system for automotive engine monitoring, built with PlatformIO using LVGL for UI and LovyanGFX for display drivers. Designed for a 1.28" round display (GC9A01 driver) on a NodeMCU-32S development board.

## Recent Activity
```
7117e46 ph1
1bcf01d planned
f05807f testing
c18b883 Merge pull request #40 from marcelrienks/error
03995e8 update
```

## Current Implementation Status

### Phase 1: ✅ Complete
- **InputManager**: GPIO 34 button with debouncing and timing logic (50ms-500ms short, >500ms long)
- **IInputService Interface**: Contract for panel input handling
- **Hardware Setup**: Button sensor integration with existing GPIO provider system
- **Architecture**: Clean separation with dependency injection

### Phase 2: ❌ Blocked by Heap Corruption
- **Multiple Inheritance Issue**: `class Panel : public IPanel, public IInputService` causing memory corruption
- **Symptom**: "CORRUPT HEAP: Bad head at 0x3ffb3ae8. Expected 0xabba1234 got 0x00000000"
- **Impact**: Application crashes during panel loading, corrupted panel names in logs
- **Root Cause**: Vtable/memory layout conflicts with multiple inheritance on ESP32

### Current Modified Files
```
 M include/interfaces/i_panel.h          # Added GetInputService() method
 M include/panels/*.h                    # Removed duplicate screen_ members
 M src/panels/*.cpp                      # Fixed constructor issues  
 M include/managers/panel_manager.h      # Added InputManager integration
 M src/managers/panel_manager.cpp        # Input service registration logic
 M src/main.cpp                          # InputManager initialization
?? include/handlers/                     # Composition-based input handlers (new approach)
?? include/panels/config_panel.h         # New placeholder config panel
```

## Architecture Summary
- **Device Layer**: Hardware abstraction (`device.h/cpp`)
- **Display Layer**: LVGL integration with LovyanGFX drivers  
- **Panel Layer**: Screen management with MVP pattern (`panels/`)
- **Component Layer**: UI elements and gauges (`components/`)
- **Sensor Layer**: ADC data acquisition (`sensors/`)
- **Manager Layer**: Coordination (Panel, Style, Trigger, Input)

## Critical Issues

### 1. Heap Corruption (Priority: Critical)
- **Problem**: Multiple inheritance causing memory corruption
- **Attempted Fixes**: 
  - Removed duplicate screen_ member shadowing
  - Tried composition approach with input handlers
  - Disabled input service registration temporarily
- **Status**: Still crashing, needs architectural redesign

### 2. Input System Integration
- **Goal**: Single button navigation (short/long press) for each panel
- **Planned Behaviors**:
  - OilPanel: Short=none, Long=config
  - SplashPanel: Short=skip animation, Long=config  
  - ErrorPanel: Short=cycle errors, Long=clear all
  - ConfigPanel: Short=next option, Long=select
- **Current Status**: Architecture complete but integration blocked

## Next Steps Available

### Immediate (Fix Critical Issues)
1. **Simplify Input Integration**: Remove multiple inheritance, use delegation pattern
2. **Stabilize Base Application**: Get panels working without input first
3. **Alternative Architecture**: Event-based input handling instead of interface inheritance

### Short Term  
1. **Complete Phase 2**: Implement panel input behaviors with stable approach
2. **Add Config Panel**: Phase 3 basic menu with grey styling
3. **Test Hardware**: Verify GPIO 34 button works with physical device

### Medium Term
1. **Full Config Implementation**: Phase 4 with persistent settings
2. **Panel Switching**: Complete TriggerManager integration for config panel
3. **Error Recovery**: Graceful handling of input system failures

## Development Commands
- **Build**: `pio.exe run -e debug-local` (fastest for testing)
- **Size Check**: `pio.exe run --target size`  
- **Upload**: `pio.exe run --target upload`
- **Clean Build**: `pio.exe run -e release`

## Recommended Immediate Action
1. **Revert to stable state**: Remove multiple inheritance from all panels
2. **Implement event delegation**: InputManager posts events, panels subscribe
3. **Test basic functionality**: Ensure splash → oil panel works without crashes
4. **Iterative approach**: Add input handling panel by panel once base is stable

## Technical Debt
- Multiple inheritance approach causing instability
- Duplicate screen_ member removal may have missed dependencies
- Need comprehensive testing of panel lifecycle without input system
- Input system architecture needs redesign for ESP32 constraints

*Generated: $(date)*