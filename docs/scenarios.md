# Trigger System Test Scenarios

## Overview

This document defines comprehensive test scenarios for the Clarity trigger system, covering all possible trigger states and combinations. These scenarios validate the state-aware trigger architecture's ability to handle complex real-world situations.

## Scenario Categories

### 1. System Startup Scenarios
### 2. Single Trigger Scenarios  
### 3. Multiple Trigger Scenarios
### 4. Edge Case Scenarios
### 5. Performance Test Scenarios

---

## 1. System Startup Scenarios

### S1.1: Clean System Startup
**Initial State:** Fresh boot, no triggers active
**Expected Flow:**
```
Boot → Splash Panel (3s) → Oil Panel (default)
```
**GPIO States:** All pins LOW
**Expected Result:** Oil panel displayed with Day theme
**Validation:** No trigger events during splash, clean transition to oil panel

### S1.2: Startup with Active Triggers
**Initial State:** Boot with key_present pin HIGH
**Expected Flow:**
```
Boot → Splash Panel (3s) → Key Panel (green, present=true)
```
**GPIO States:** KEY_PRESENT=HIGH, others=LOW
**Expected Result:** Green key panel displayed after splash
**Validation:** InitializeTriggersFromGpio() sets correct initial states

### S1.3: Startup with Active Triggers
**Initial State:** Boot with key_not_present pin HIGH
**Expected Flow:**
```
Boot → Splash Panel (3s) → Key Panel (red, present=false)
```
**GPIO States:** KEY_NOT_PRESENT=HIGH, others=LOW
**Expected Result:** Red key panel displayed after splash
**Validation:** InitializeTriggersFromGpio() sets correct initial states

### S1.4: Startup with Active Triggers
**Initial State:** Boot with lock_present pin HIGH
**Expected Flow:**
```
Boot → Splash Panel (3s) → Lock Panel
```
**GPIO States:** KEY_PRESENT=HIGH, others=LOW
**Expected Result:** Lock panel displayed after splash
**Validation:** InitializeTriggersFromGpio() sets correct initial states

### S1.5: Startup with Theme Trigger Active
**Initial State:** Boot with lights pin HIGH  
**Expected Flow:**
```
Boot → Splash Panel (3s) → Oil Panel with Night Theme
```
**GPIO States:** LIGHTS=HIGH, others=LOW
**Expected Result:** Oil panel with night theme applied
**Validation:** Theme applied during startup, no panel change

---

## 2. Single Trigger Scenarios

### S2.1: Lights Trigger (Theme Only)
**Initial State:** Oil panel, Day theme
**Trigger Sequence:**
```
LIGHTS: LOW → HIGH → Night theme applied to oil panel
LIGHTS: HIGH → LOW → Day theme applied to oil panel
```
**Expected Results:**
- No panel changes, only theme changes
- Oil panel remains throughout
- Theme transitions are immediate

### S2.2: Lock Trigger  
**Initial State:** Oil panel
**Trigger Sequence:**
```
LOCK: LOW → HIGH → Load Lock Panel
LOCK: HIGH → LOW → Restore Oil Panel
```
**Expected Results:**
- Lock panel displays when engaged
- Oil panel restored when disengaged
- Clean panel transitions

### S2.3: Key Present Trigger
**Initial State:** Oil panel
**Trigger Sequence:**
```
KEY_PRESENT: LOW → HIGH → Load Key Panel (green)
KEY_PRESENT: HIGH → LOW → Restore Oil Panel
```
**Expected Results:**
- Green key panel (present=true) when key detected
- Oil panel restored when key removed
- Correct key styling applied

### S2.4: Key Not Present Trigger
**Initial State:** Oil panel
**Trigger Sequence:**
```
KEY_NOT_PRESENT: LOW → HIGH → Load Key Panel (red)
KEY_NOT_PRESENT: HIGH → LOW → Restore Oil Panel
```
**Expected Results:**
- Red key panel (present=false) when key explicitly absent
- Oil panel restored when trigger clears
- Correct key styling applied

---

## 3. Multiple Trigger Scenarios

### S3.1: Priority Override - Key Present Over Lock
**Initial State:** Oil panel
**Trigger Sequence:**
```
1. LOCK: LOW → HIGH              → Lock Panel
2. KEY_PRESENT: LOW → HIGH       → Key Panel (green) [overrides lock]
3. KEY_PRESENT: HIGH → LOW       → Lock Panel [restored, lock still active]
4. LOCK: HIGH → LOW              → Oil Panel [restored]
```
**Expected Results:**
- Critical priority (key) overrides Important priority (lock)
- Lock panel restored when key deactivates (lock still active)
- Oil panel only restored when all triggers inactive

### S3.2: Key Present vs Key Not Present
**Initial State:** Oil panel
**Trigger Sequence:**
```
1. KEY_PRESENT: LOW → HIGH       → Key Panel (green)
2. KEY_NOT_PRESENT: LOW → HIGH   → Key Panel (red) [overrides, same priority FIFO]
3. KEY_PRESENT: HIGH → LOW       → Key Panel (red) [key_not_present still active]
4. KEY_NOT_PRESENT: HIGH → LOW   → Oil Panel [all keys inactive]
```
**Expected Results:**
- Last key trigger wins when both active (FIFO within same priority)
- Panel reflects currently active key trigger
- Oil panel only when no key triggers active

### S3.3: Key Not Present vs Key Present (Reverse Order)
**Initial State:** Oil panel
**Trigger Sequence:**
```
1. KEY_NOT_PRESENT: LOW → HIGH   → Key Panel (red)
2. KEY_PRESENT: LOW → HIGH       → Key Panel (green) [overrides, same priority FIFO]
3. KEY_NOT_PRESENT: HIGH → LOW   → Key Panel (green) [key_present still active]
4. KEY_PRESENT: HIGH → LOW       → Oil Panel [all keys inactive]
```
**Expected Results:**
- Last key trigger wins when both active
- Panel reflects currently active key trigger
- Oil panel only when no key triggers active

### S3.4: Theme + Panel Triggers
**Initial State:** Oil panel, Day theme
**Trigger Sequence:**
```
1. LIGHTS: LOW → HIGH            → Oil Panel with Night Theme
2. KEY_PRESENT: LOW → HIGH       → Key Panel (green) with Night Theme
3. KEY_PRESENT: HIGH → LOW       → Oil Panel with Night Theme [lights still active]
4. LIGHTS: HIGH → LOW            → Oil Panel with Day Theme
```
**Expected Results:**
- Theme changes apply to current panel
- Panel changes maintain current theme
- Independent theme and panel trigger handling

### S3.5: Triple Trigger Activation
**Initial State:** Oil panel, Day theme
**Trigger Sequence:**
```
1. LIGHTS: LOW → HIGH            → Oil Panel with Night Theme
2. LOCK: LOW → HIGH              → Lock Panel with Night Theme
3. KEY_PRESENT: LOW → HIGH       → Key Panel (green) with Night Theme [highest priority]
4. KEY_PRESENT: HIGH → LOW       → Lock Panel with Night Theme [lock still active]
5. LOCK: HIGH → LOW              → Oil Panel with Night Theme [lights still active]
6. LIGHTS: HIGH → LOW            → Oil Panel with Day Theme
```
**Expected Results:**
- Highest priority active trigger controls panel
- Theme persists across panel changes
- Correct restoration chain based on remaining active triggers

---

## 4. Edge Case Scenarios

### S4.1: Rapid Toggle Single Trigger
**Initial State:** Oil panel
**Trigger Sequence:** (within single loop iteration)
```
Messages: [KEY_PRESENT: HIGH, KEY_PRESENT: LOW, KEY_PRESENT: HIGH]
Consolidated State: {KEY_PRESENT: ACTIVE}
Expected Result: Key Panel (green)
```
**Validation:** Message consolidation works correctly, final state used

### S4.2: Rapid Toggle Multiple Triggers
**Initial State:** Oil panel
**Trigger Sequence:** (within single loop iteration)
```
Messages: [KEY_PRESENT: HIGH, KEY_NOT_PRESENT: HIGH, KEY_PRESENT: LOW]
Consolidated State: {KEY_NOT_PRESENT: ACTIVE}
Expected Result: Key Panel (red)
```
**Validation:** Complex message consolidation, correct final state

### S4.3: All Triggers Rapid Activation
**Initial State:** Oil panel, Day theme
**Trigger Sequence:** (within single loop iteration)
```
Messages: [LIGHTS: HIGH, LOCK: HIGH, KEY_PRESENT: HIGH, KEY_NOT_PRESENT: HIGH]
Consolidated State: {LIGHTS: ACTIVE, LOCK: ACTIVE, KEY_NOT_PRESENT: ACTIVE}
Expected Result: Key Panel (red) with Night Theme [last key trigger + theme]
```
**Validation:** Priority resolution with theme application

### S4.4: Simultaneous Deactivation
**Initial State:** Multiple triggers active
**Trigger Sequence:** (within single loop iteration)
```
Initial: KEY_PRESENT=HIGH, LOCK=HIGH, LIGHTS=HIGH
Messages: [KEY_PRESENT: LOW, LOCK: LOW, LIGHTS: LOW]
Consolidated State: {} (all inactive)
Expected Result: Oil Panel with Day Theme
```
**Validation:** Clean restoration to default state

### S4.5: Invalid Trigger Combinations
**Initial State:** Oil panel
**Trigger Sequence:**
```
Both KEY_PRESENT and KEY_NOT_PRESENT simultaneously HIGH
Expected Behavior: Last received message wins (FIFO)
```
**Validation:** System handles impossible hardware states gracefully

---

## 5. Performance Test Scenarios

### S5.1: High Frequency Trigger Events
**Test:** 100 trigger events per second for 10 seconds
**Validation:** 
- No message queue overflow
- No missed trigger events
- Stable panel rendering
- Memory usage remains stable

### S5.2: Message Queue Stress Test
**Test:** Fill message queue to capacity, then drain
**Validation:**
- Queue handles maximum capacity
- FIFO ordering maintained
- No memory leaks
- Correct final state processing

### S5.3: Panel Load Performance
**Test:** Rapid panel switching between all panel types
**Validation:**
- Single panel load per execution cycle
- No redundant UI object creation
- Smooth LVGL rendering
- Memory efficiency maintained

### S5.4: Long Running Stability
**Test:** 24-hour continuous operation with random trigger patterns
**Validation:**
- No memory leaks
- Consistent trigger response times
- Stable panel rendering
- No trigger state corruption

---

## 6. Test Execution Matrix

### Priority Level Testing
| Priority | Trigger | Expected Panel | Theme Effect |
|----------|---------|----------------|--------------|
| CRITICAL (0) | key_present | Key (green) | Inherits current |
| CRITICAL (0) | key_not_present | Key (red) | Inherits current |
| IMPORTANT (1) | lock_state | Lock | Inherits current |
| NORMAL (2) | lights_state | No change | Day/Night toggle |

### Multi-Trigger Combinations
| Active Triggers | Expected Panel | Expected Theme |
|-----------------|----------------|----------------|
| lights | Oil | Night |
| lock | Lock | Current |
| key_present | Key (green) | Current |
| lights + lock | Lock | Night |
| lights + key_present | Key (green) | Night |
| lock + key_present | Key (green) | Current |
| All active | Key (last activated) | Night |

### Restoration Scenarios
| Deactivated Trigger | Remaining Active | Expected Result |
|---------------------|------------------|-----------------|
| key_present | key_not_present | Key (red) |
| key_not_present | key_present | Key (green) |
| key_present | lock | Lock panel |
| lock | key_present | Key (green) |
| Any | lights only | Oil with Night theme |
| All | None | Oil with Day theme |

---

## 7. Automated Test Implementation

### Test Framework Structure
```cpp
class TriggerScenarioTest {
    void SetupScenario(const char* name, InitialState initial);
    void ApplyTriggerSequence(std::vector<TriggerEvent> events);
    void ValidateExpectedState(ExpectedState expected);
    void LogScenarioResult(bool passed, const char* details);
};
```

### Test Data Format
```cpp
struct TriggerEvent {
    const char* triggerId;
    bool pinState;
    uint32_t timestamp;
};

struct ExpectedState {
    const char* expectedPanel;
    const char* expectedTheme;
    std::vector<const char*> activeTriggers;
};
```

### Validation Criteria
1. **Panel State**: Correct panel loaded and displayed
2. **Theme State**: Correct theme applied and visible
3. **Trigger State**: All trigger objects in expected execution state
4. **Memory State**: No memory leaks or corruption
5. **Performance**: Response time within acceptable limits
6. **Logging**: Clear audit trail of trigger decisions

### Continuous Integration
- All scenarios run on every code change
- Performance regression detection
- Memory leak detection
- Hardware-in-loop testing with GPIO simulation
- Automated report generation with pass/fail status

This comprehensive scenario matrix ensures the state-aware trigger architecture handles all real-world combinations correctly while maintaining optimal performance and system reliability.