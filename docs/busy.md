# UIState Management and Interrupt Processing

## Overview

The Clarity application uses a UIState system to coordinate interrupt processing with LVGL UI operations. This document describes the distributed state management architecture that ensures interrupts are only processed when the UI is idle, preventing conflicts with animations and transitions.

## Core Architecture Principles

1. **Single-Point Interrupt Checking**: Interrupts are ONLY checked from the main loop
2. **Distributed State Management**: Components closest to the activity set the state
3. **LVGL Synchronization**: UIState reflects LVGL's activity status
4. **Explicit State Transitions**: All state changes should be logged for debugging

## UIState Enum

```cpp
enum class UIState {
    IDLE,           // No LVGL activity, safe for all operations
    LOADING,        // Panel loading with potential animations
    UPDATING,       // Panel updating (sensor data refresh)
    ANIMATING,      // LVGL animations in progress
    TRANSITIONING,  // Panel transitions in progress
    LVGL_BUSY       // Generic LVGL busy state
};
```

### State Descriptions

- **IDLE**: LVGL is not running any animations or transitions. Safe to:
  - Process all interrupts (triggers and actions)
  - Switch panels
  - Execute user actions
  
- **LOADING**: A panel is being loaded, potentially with entrance animations
  - Interrupts are deferred
  - Critical triggers may still be processed (implementation-specific)
  
- **UPDATING**: Panel is refreshing its data (e.g., sensor readings)
  - Low-priority interrupts deferred
  - High-priority triggers may be processed
  
- **ANIMATING**: LVGL animations are actively running (e.g., gauge needle movement)
  - All interrupts deferred until animation completes
  - Ensures smooth visual transitions
  
- **TRANSITIONING**: Panel-to-panel transitions in progress
  - All interrupts deferred
  - Prevents panel switching during transitions
  
- **LVGL_BUSY**: Generic busy state for other LVGL operations
  - Reserved for future use
  - All interrupts deferred

## State Ownership and Management

### Who Can Set UIState

1. **Panels** (Most Granular)
   - Have direct knowledge of their animations
   - Should set state when starting/completing animations
   - Example: OemOilPanel sets ANIMATING during gauge updates

2. **PanelManager** (High-Level)
   - Sets default states for panel operations
   - Manages state during panel switches
   - Provides fallback state management

3. **Special Components**
   - Components with LVGL knowledge may set states
   - Must coordinate with PanelManager

### State Setting Rules

```cpp
// Panel sets state during animation
void OemOilPanel::UpdateGauge() {
    panelService_->SetUiState(UIState::ANIMATING);
    // Start LVGL animation
    lv_anim_start(&animation_);
}

// Panel clears state when complete
void OemOilPanel::AnimationComplete() {
    panelService_->SetUiState(UIState::IDLE);
}

// PanelManager provides default behavior
void PanelManager::UpdatePanel() {
    SetUiState(UIState::UPDATING);
    panel_->Update(callback);
}
```

## Main Loop Flow

```
main loop() {
    // 1. Check interrupts only when IDLE
    if (uiState == IDLE) {
        interruptManager->CheckAllInterrupts();
        // This processes: triggers → actions
    }
    
    // 2. Update active panel
    panelManager->UpdatePanel();
    // Panel may change UIState during update
    
    // 3. Process LVGL tasks
    Ticker::handleLvTasks();
    
    // 4. Dynamic delay
    Ticker::handleDynamicDelay();
}
```

### Interrupt Processing Order

When UIState is IDLE, interrupts are processed in priority order:
1. **Triggers** (TriggerManager) - Hardware state changes
2. **Actions** (ActionManager) - User input actions

## Implementation Guidelines

### For Panel Developers

1. **Set State Before Animations**
   ```cpp
   void MyPanel::Load() {
       if (hasAnimation) {
           panelService_->SetUiState(UIState::LOADING);
           startLoadAnimation();
       }
   }
   ```

2. **Clear State After Animations**
   ```cpp
   void MyPanel::OnAnimationComplete() {
       panelService_->SetUiState(UIState::IDLE);
       if (completionCallback_) {
           completionCallback_();
       }
   }
   ```

3. **Use Appropriate States**
   - LOADING: For initial panel load
   - ANIMATING: For data update animations
   - TRANSITIONING: For panel-to-panel transitions

### For System Components

1. **Check State Before Operations**
   ```cpp
   bool ActionManager::CanExecuteActions() {
       UIState state = panelService_->GetUiState();
       return state == UIState::IDLE;
   }
   ```

2. **Queue Operations When Busy**
   ```cpp
   if (!CanExecuteActions()) {
       pendingAction_ = action;
       return;
   }
   ```

## Example Scenarios

### Scenario 1: Splash Screen with Pending Action

```
1. Splash panel loads → Sets LOADING
2. User long-presses during splash → Action queued
3. Splash animation completes → Sets IDLE  
4. Main loop detects IDLE → Processes pending action
5. Action loads config panel → Skips default panel
```

### Scenario 2: Gauge Animation

```
1. Oil panel updating → Sets UPDATING
2. New sensor data arrives → Starts gauge animation
3. Panel sets ANIMATING → Interrupts deferred
4. Animation completes → Sets IDLE
5. Main loop resumes interrupt processing
```

### Scenario 3: Panel Transition

```
1. Trigger detected → Request panel switch
2. PanelManager sets TRANSITIONING
3. Old panel unloads, new panel loads
4. Transition complete → Sets IDLE
5. Normal operation resumes
```

## State Transition Diagram

```
         ┌──────┐
    ┌────│ IDLE │────┐
    │    └──┬───┘    │
    │       │        │
    ▼       ▼        ▼
┌────────┐┌─────────┐┌──────────┐
│LOADING ││UPDATING ││ANIMATING │
└────┬───┘└────┬────┘└─────┬────┘
     │         │            │
     └─────────┴────────────┘
               │
               ▼
           ┌──────┐
           │ IDLE │
           └──────┘
```

## Debugging State Issues

### Enable State Logging

```cpp
void PanelManager::SetUiState(UIState state) {
    log_d("UIState transition: %s → %s", 
          StateToString(uiState_), 
          StateToString(state));
    uiState_ = state;
}
```

### Common Issues

1. **Stuck in Busy State**
   - Panel forgot to set IDLE after animation
   - Check animation completion callbacks

2. **Interrupts Not Processing**
   - Verify main loop is checking state
   - Ensure panels set IDLE when appropriate

3. **Jerky Animations**
   - Interrupts processing during animation
   - Verify ANIMATING state is set

## Future Enhancements

1. **State Priorities**: Allow certain high-priority interrupts during specific states
2. **State Timeout**: Automatic recovery if stuck in busy state
3. **Performance Metrics**: Track time spent in each state
4. **State Stack**: Support nested state management for complex UIs

## Best Practices

1. **Always Set State**: Don't assume default behavior
2. **Log Transitions**: Help future debugging
3. **Complete Callbacks**: Always transition to IDLE in completion callbacks
4. **Test Interrupts**: Verify interrupts work correctly with your panel
5. **Document States**: Comment why specific states are used

## Summary

The distributed UIState management system ensures smooth UI operation by:
- Preventing interrupt processing during LVGL activity
- Allowing fine-grained control by panels
- Maintaining single-point interrupt checking in main loop
- Providing clear state ownership rules

This architecture scales with application complexity while maintaining predictable behavior.