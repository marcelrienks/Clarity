# Input System Refactoring - Interrupt-Driven Architecture

## Overview

This document outlines the ongoing refactoring to implement an interrupt-driven input system that abstracts triggers and inputs under a unified interrupt interface. The goal is to create a more responsive system that can handle inputs during idle time, including animation phases.

## Current Status: **IN PROGRESS**

### Completed Work ✅

#### Phase 1: Foundation (Completed)
- **IInterrupt Interface** (`include/interfaces/i_interrupt.h`)
  - Unified interface for trigger and input interrupt checking
  - Priority-based system (Triggers=100, Input=50, Background=10)
  - Optimized checking with `HasPendingInterrupts()` method

- **IInputAction Interface** (`include/interfaces/i_input_action.h`)
  - Command pattern for panel-specific actions
  - `CanExecute()` method for deferred execution
  - Concrete implementations in `include/actions/input_actions.h`:
    - `PanelSwitchAction`: Switch to different panel
    - `SkipAnimationAction`: Skip current animation
    - `MenuNavigationAction`: Navigate menu options
    - `NoAction`: Null object pattern for no-op

- **InterruptManager** (`include/managers/interrupt_manager.h`)
  - Centralized coordinator for all interrupt sources
  - Priority-ordered interrupt checking
  - Performance optimized with quick pending check
  - Designed for idle time integration

- **Updated IInputService** (`include/interfaces/i_input_service.h`)
  - Changed from direct processing to action-based workflow
  - Methods now return `std::unique_ptr<IInputAction>` instead of void
  - Maintains `CanProcessInput()` for queuing logic

### Work In Progress 🚧

#### Phase 2: Core Manager Updates (Completed ✅)
- **InputManager IInterrupt Implementation** ✅
  - Added `IInterrupt` inheritance 
  - Replaced `PendingInput` struct with `PendingAction` using action objects
  - Implemented priority 50 interrupt checking via `CheckInterrupts()`
  - Action-based workflow with backward compatibility for legacy panels
  - Queues actions when panels return `CanProcessInput() = false`

- **TriggerManager IInterrupt Implementation** ✅
  - Added `IInterrupt` inheritance
  - Maintains existing trigger logic via `ProcessTriggerEvents()`
  - Implemented priority 100 interrupt checking (higher than input)
  - Quick pending check via sensor state comparison

- **Legacy Compatibility Adapter** ✅
  - Created `LegacyInputAdapter` to bridge old panels with new action system
  - Allows gradual panel migration without breaking builds
  - Wraps old `OnShortPress/OnLongPress` methods in action objects

### Pending Work 📋

#### Phase 3: Panel Updates
Update all panels to return actions instead of processing directly:

- **SplashPanel**
  - Short press: `SkipAnimationAction` (if animation can be skipped)
  - Long press: `PanelSwitchAction` to config panel
  - `CanProcessInput()`: Return false during animation

- **OilPanel** 
  - Short press: `NoAction` (no action defined)
  - Long press: `PanelSwitchAction` to config panel
  - `CanProcessInput()`: Always true

- **ErrorPanel**
  - Short press: `MenuNavigationAction(NEXT)` to cycle errors
  - Long press: Custom clear all errors action
  - `CanProcessInput()`: Always true

- **ConfigPanel**
  - Short press: `MenuNavigationAction(NEXT)` for options
  - Long press: `MenuNavigationAction(SELECT)` for selection
  - `CanProcessInput()`: Always true

#### Phase 4: Integration
1. **Service Initialization Updates**
   - Add `InterruptManager` to global services
   - Register `InputManager` and `TriggerManager` as interrupt sources
   - Initialize interrupt system in `main.cpp`

2. **Main Loop Updates** 
   - Replace `triggerManager->ProcessTriggerEvents()`
   - Replace `inputManager->ProcessInputEvents()` 
   - Add single `interruptManager->CheckAllInterrupts()` call

3. **LVGL Integration**
   - Add interrupt checking to LVGL idle callbacks
   - Enable responsive input during animations
   - Check interrupts between animation phases

## Architecture Benefits

### Panel Autonomy
- Panels define their own actions without InputManager needing implementation details
- Easy to modify panel behavior without touching core managers
- Clear separation of concerns

### Unified Interrupt System  
- Triggers and inputs handled through same interrupt interface
- Priority-based processing ensures critical events handled first
- Consistent idle time utilization across all interrupt sources

### Responsive Input
- Interrupts checked during LVGL idle time
- Input processed during animation gaps (fade in/out transitions)
- No more lost button presses during splash animation

### Flexible Actions
- New action types can be added without changing core managers
- Actions can be queued and executed when appropriate
- Complex multi-step actions possible through action composition

## Breaking Changes

⚠️ **This refactoring introduces breaking changes to existing panel implementations:**

1. **IInputService Interface Change**
   - `OnShortPress()` → `GetShortPressAction()`
   - `OnLongPress()` → `GetLongPressAction()`
   - Return types changed from `void` to `std::unique_ptr<IInputAction>`

2. **Manager Dependencies**
   - New dependency on `InterruptManager` in main initialization
   - `InputManager` and `TriggerManager` must implement `IInterrupt`

3. **Build Dependencies**
   - New files require inclusion in build system
   - Additional header dependencies for action classes

## Testing Strategy

### Unit Testing
- Test individual action execution
- Test interrupt priority ordering
- Test action queuing and execution timing

### Integration Testing  
- Test interrupt checking during animations
- Verify responsive input during splash screen
- Test priority handling between triggers and inputs

### Regression Testing
- Ensure existing panel behaviors unchanged
- Verify all button press combinations work
- Test panel switching functionality

## Implementation Notes

### Memory Management
- Actions use `std::unique_ptr` for automatic cleanup
- InterruptManager holds raw pointers (sources manage lifetime)
- No memory leaks from action objects

### Performance Considerations
- Quick `HasPendingInterrupts()` check prevents unnecessary processing
- Priority ordering minimizes time to handle critical events
- Lightweight interrupt checking suitable for idle time calls

### Error Handling
- Null checks for interrupt sources and actions
- Graceful handling of invalid panel states
- Logging for debugging interrupt processing

## Current State

**Phase 1 and Phase 2 are complete!** The interrupt-driven architecture is fully implemented with:
- ✅ All core interfaces and managers
- ✅ Action-based input workflow 
- ✅ Interrupt abstraction for triggers and inputs
- ✅ Legacy adapter for backward compatibility

**The system can now be integrated and tested** while panels are gradually migrated to the new action interface.

Next steps:
1. **Integration testing** - Add InterruptManager to main loop and test core functionality
2. **Panel migration** - Update panels one at a time to use action objects  
3. **LVGL integration** - Add interrupt checking to animation idle callbacks
4. **Remove legacy adapter** - Once all panels are migrated

---

*Last Updated: 2025-01-09*  
*Status: Core Implementation Complete, Ready for Integration*