# Heap Corruption Debugging Investigation

## Problem Overview
Heap corruption crash occurring when loading OemOilPanel after SplashPanel on the "input" branch. Main branch works correctly.

**Error**: `CORRUPT HEAP: Bad head at 0x3ffb3d68. Expected 0xabba1234 got 0xbaad5678`
**Branch**: input (broken) vs main (working)
**Trigger Commit**: ph2.1 (5b3974ab50f742d7ba482f181887e7ff209b2327) introduced issue

## Architecture Context
- ESP32-based system using LVGL for UI
- OemOilPanel uses multiple inheritance: `IPanel, IInputService`
- Constructor dependency injection pattern
- StyleManager handles themes and LVGL styling
- Components create complex LVGL objects (scales, gauges, etc.)

## Systematic Investigation Results

### 1. Initial Theories (REJECTED)
❌ **Multiple inheritance issue**: Splash panel also uses multiple inheritance and works
❌ **IPanel() constructor**: Removing explicit base constructor call didn't fix issue
❌ **Screen member duplication**: Not the root cause

### 2. Timing & Initialization Issues (PARTIALLY CORRECT)
✅ **StyleManager initialization order**: Fixed application hanging by moving LVGL calls from constructor to InitializeStyles()
- Issue: Constructor called `lv_style_init()` before LVGL ready
- Fix: Deferred LVGL initialization until after device preparation

### 3. StyleManager Access Issues (INVESTIGATED)
❌ **Component StyleManager access timing**: Added safety checks, but corruption persists
- Added `styleService_->IsInitialized()` checks before component creation
- Added validation before all StyleManager method calls
- Components still create successfully, corruption happens after

### 4. Theme Application Conflicts (INVESTIGATED)
❌ **Automatic theme application**: Disabled automatic `ApplyThemeToScreen()` in SetTheme()
- Found second ApplyThemeToScreen() call from StyleManager::SetTheme()
- Disabled it, but corruption still occurs
- Manual theme application in Init() works fine

### 5. Component Style Application (INVESTIGATED)
❌ **Component LVGL style application**: Disabled all `lv_obj_add_style()` calls in components
- Disabled gauge styles, danger zone styles, text styles
- Corruption still occurs without any style application
- Issue is NOT in StyleManager style objects or their application

### 6. LVGL Object Creation (CURRENT FOCUS)
❌ **Complex LVGL objects**: Systematically disabled component objects
- Disabled icons, labels, needles - corruption persists
- Disabled scale sections (`lv_scale_add_section`) - corruption persists
- Disabled ALL scale configuration - **corruption still occurs**

## Current State: Ultra-Minimal Test
Currently testing with absolutely minimal component:
```cpp
// Only these operations remain:
scale_ = display->CreateScale(screen);        // Basic LVGL scale creation
lv_obj_set_size(scale_, 240, 240);          // Set size
lv_obj_align(scale_, location.align, ...);   // Set position
// NO styling, NO configuration, NO additional objects
```

**Status**: If this still crashes, the issue is in fundamental LVGL object creation or unrelated to components entirely.

## Key Findings

### 1. Timing Pattern
```
[9799] ApplyThemeToScreen: SUCCESS (Init)
[9899] Component rendering starts
[9982] Component rendered successfully  
[9990] loading...
[9996] TESTING: Skipping second ApplyThemeToScreen
CORRUPT HEAP: Bad head at 0x3ffb3d68 ← Crashes HERE
```

### 2. Memory Addresses
- Screen: `0x3ffb3a9c`
- BackgroundStyle: `0x3ffb841c`
- Corruption: `0x3ffb3d68` (between screen and style)

### 3. Corruption Values
- Original: `0x3f45933c` (random corruption)
- Current: `0xbaad5678` (debug pattern - "bad pointer")

### 4. Architecture Compatibility
✅ **Constructor dependency injection**: Works correctly
✅ **Multiple inheritance**: Not the issue (SplashPanel works)
✅ **StyleManager initialization**: Fixed and working
✅ **Basic panel lifecycle**: Init/Load/Update pattern works

## Options Exhausted

### Memory Management
- ✅ Constructor initialization order
- ✅ Base class initialization
- ✅ Member initialization lists
- ✅ LVGL animation initialization

### StyleManager Issues  
- ✅ Initialization timing
- ✅ Theme application order
- ✅ Style object corruption
- ✅ Double application conflicts
- ✅ Automatic vs manual theme application

### Component Issues
- ✅ Component creation timing
- ✅ StyleManager safety checks
- ✅ LVGL style application
- ✅ Complex object creation (sections, labels, etc.)
- 🔄 **CURRENTLY TESTING**: Basic LVGL object creation

### Architecture Issues
- ✅ Interface changes (IPanel modifications)
- ✅ Multiple inheritance conflicts
- ✅ Method parameter injection vs constructor injection

## ROOT CAUSE IDENTIFIED

### Final Debugging Path to Root Cause

1. **Started with heap corruption after SplashPanel → OemOilPanel transition**
   - Error: `CORRUPT HEAP: Bad head at 0x3ffb3d68`
   - Initially suspected OemOilPanel implementation

2. **Systematically eliminated suspects**:
   - ✅ Disabled all LVGL styling → still crashed
   - ✅ Disabled all component objects except basic scale → still crashed
   - ✅ Disabled event callbacks → still crashed
   - ✅ Confirmed multiple inheritance not the issue (SplashPanel works)

3. **Added comprehensive heap monitoring**:
   - All LVGL operations completed successfully
   - `lv_screen_load()` completed without issue
   - Memory usage normal throughout Load() method
   - Load() returned successfully to panel manager

4. **Narrowed crash location through logging**:
   ```
   SplashCompletionCallback:
     panel_.reset() → OK
     Ticker::handleLvTasks() → OK
     CreateAndLoadPanel(OemOilPanel) → OK
       - Init() → OK
       - Load() → OK
       - Returns to SplashCompletionCallback → OK
     After CreateAndLoadPanel returns → OK
     SplashCompletionCallback returns → CRASH!
   ```

5. **Discovered the callback chain**:
   - LVGL timer fires → `animation_complete_timer_callback`
   - Calls `thisInstance->callbackFunction_()`
   - Which calls `SplashCompletionCallback`
   - Which calls `panel_.reset()` destroying SplashPanel
   - But we're still inside SplashPanel's timer callback!

### The Issue
**Callback Stack Corruption**: The heap corruption occurs when `SplashCompletionCallback` returns because:

1. **Callback Chain**: 
   - LVGL timer → `animation_complete_timer_callback` → `callbackFunction_()` → `SplashCompletionCallback`
   - Inside `SplashCompletionCallback`, `panel_.reset()` destroys the SplashPanel
   - But we're still executing inside a callback from that panel's timer!

2. **Memory Timeline**:
   - SplashPanel timer fires and calls callback
   - Callback calls `panel_.reset()` which destroys SplashPanel (and its timer)
   - When callback returns, it's unwinding through destroyed memory
   - Heap corruption at address `0x3ffb3d68`

### Why OemOilPanel but not other panels?
- The corruption happens during the transition FROM SplashPanel TO OemOilPanel
- It's not about OemOilPanel itself, but the callback destruction sequence
- The memory layout with OemOilPanel just happens to expose the corruption

### The Fix Attempts
1. **Removed panel_.reset() from SplashCompletionCallback** - Still crashed because CreateAndLoadPanel also destroys the panel
2. **Removed panel_.reset() from both locations** - Still crashed with different corruption pattern
3. **Added currentTimer_ = nullptr after timer deletion** - Prevents double-delete but doesn't fix core issue

### The Real Issue
The problem is deeper than just the panel destruction. When we're in the LVGL timer callback:
- We're executing code that belongs to the SplashPanel
- Any attempt to destroy or modify the panel's memory while in this callback corrupts the stack
- Even without explicit destruction, overwriting the panel_ pointer causes issues

### Why Main Branch Works
The main branch likely has different callback timing or structure that avoids this issue. Key differences noted:
- No currentTimer_ tracking in main branch
- Different parameter passing to panel methods
- Possibly different LVGL timer handling

## Proposed Solutions

### Option 1: Deferred Destruction Using LVGL Idle Callback
Use LVGL's idle callback mechanism to defer panel destruction until the system is idle:
- Instead of destroying panel immediately, queue it for destruction
- Register an LVGL idle callback that runs when no other callbacks are active
- The idle callback safely destroys the old panel
- Prevents destruction while inside the panel's own callback

### Option 2: Restructure Callback Flow
Modify the callback architecture to avoid panel state changes during callbacks:
- Move panel transition logic outside of timer callbacks
- Use a state machine or event queue for panel transitions
- Timer callbacks only set flags/events, actual transitions happen in main loop
- Ensures no panel is destroyed while its callbacks are executing

### Option 3: Revert to ph1 Approach
Understand why ph1 doesn't exhibit this issue and what ph2.1 changed:
- Compare callback structures between commits
- Analyze what new features were added
- Identify the specific change that introduced the bug
- Selectively revert problematic changes
3. **Comparison testing**: Create identical LVGL objects outside component context
4. **Rollback testing**: Revert to working commit and incrementally apply changes
5. **Hardware testing**: Test on different ESP32 boards/configurations

## Critical Questions Remaining
1. **Why does main branch work?** - What specific change in input branch causes this?
2. **Is LVGL scale creation fundamentally broken?** - Platform/version issue?
3. **Is corruption actually asynchronous?** - Happening in callback/timer during component rendering?
4. **Memory state dependency?** - Does heap state after SplashPanel affect OemOilPanel?

## Conclusion
The heap corruption is **NOT** caused by:
- Architecture changes (multiple inheritance, constructor injection)
- StyleManager initialization or usage
- Component style application
- Complex LVGL object creation

The issue appears to be at the **most fundamental level** of LVGL object creation, which suggests either:
- A platform-specific LVGL bug
- Memory management issue in the ESP32/LVGL integration
- Asynchronous corruption unrelated to the component rendering sequence

The systematic elimination approach has ruled out all higher-level causes, pointing to a low-level issue requiring deep memory analysis or LVGL debugging.