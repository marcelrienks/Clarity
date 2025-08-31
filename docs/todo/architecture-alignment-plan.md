# Architecture Alignment Plan

## Executive Summary

This document outlines the comprehensive plan to align the Clarity codebase with its documented architecture. The analysis revealed that while core architectural concepts are implemented (~70% alignment), there are significant deviations in critical areas including interrupt handling, button timing logic, and restoration mechanisms.

## Current State Assessment

### Overall Alignment: ~70%

**Correctly Implemented (✅)**
- Dual factory pattern with dependency injection
- Handler-based sensor ownership model
- Split sensor design (KeyPresent/KeyNotPresent)
- Display-only panels (Key/Lock)
- Effect-based interrupt categorization

**Major Inconsistencies (❌)**
- Evaluation/execution separation not implemented
- Button timing logic in wrong location
- Interrupt structure mismatch
- Complex restoration logic vs. simple design

**Partial Implementation (⚠️)**
- Interrupt processing flow differs internally
- Error panel integration incomplete
- Legacy code remnants present

## Critical Inconsistencies Requiring Action

### 1. Interrupt Evaluation/Execution Separation

#### Current Implementation
```cpp
// Single combined function that signals both evaluation and execution
InterruptResult (*processFunc)(void* context);

// Returns enum indicating what to do
enum InterruptResult {
    NO_ACTION,
    NEEDS_EXECUTION,
    HANDLED
};
```

#### Target Architecture
```cpp
// Separate evaluation from execution
struct Interrupt {
    bool (*evaluationFunc)(void* context);  // Check if state changed
    void (*executionFunc)(void* context);   // Execute the action
    InterruptEffect effect;                 // Effect type
    union {
        PanelType panel;
        Theme theme;
        ButtonAction action;
    } data;
};
```

#### Migration Tasks
- [ ] Define new Interrupt structure with separate function pointers
- [ ] Refactor all interrupt registrations to use new structure
- [ ] Update InterruptManager to handle separate evaluation/execution phases
- [ ] Migrate existing processFunc logic to new model
- [ ] Update handlers to implement new interface

### 2. Button Timing Logic Location

#### Current Implementation
```cpp
// InterruptManager::EvaluateQueuedInterrupts()
const uint32_t SHORT_PRESS_THRESHOLD_MS = 50;
const uint32_t LONG_PRESS_THRESHOLD_MS = 2000;
const uint32_t MAX_PRESS_THRESHOLD_MS = 5000;

// Manual timing calculation in manager
if (elapsed >= SHORT_PRESS_THRESHOLD_MS && elapsed < LONG_PRESS_THRESHOLD_MS) {
    // Short press logic
}
```

#### Target Architecture
```cpp
// ActionButtonSensor should handle its own timing
class ActionButtonSensor : public BaseSensor<ButtonState> {
private:
    static constexpr uint32_t SHORT_PRESS_MIN_MS = 50;
    static constexpr uint32_t SHORT_PRESS_MAX_MS = 2000;
    static constexpr uint32_t LONG_PRESS_MIN_MS = 2000;
    static constexpr uint32_t LONG_PRESS_MAX_MS = 5000;
    
    ButtonState DetermineButtonState();  // Internal timing logic
public:
    bool HasChanged() override;
    ButtonState GetState();
};
```

#### Migration Tasks
- [ ] Move timing constants to ActionButtonSensor
- [ ] Implement DetermineButtonState() with proper debouncing
- [ ] Add button state enum (IDLE, PRESSING, SHORT_PRESS, LONG_PRESS)
- [ ] Remove timing logic from InterruptManager
- [ ] Update QueuedHandler to use new sensor interface
- [ ] Add unit tests for button timing scenarios

### 3. Centralized Restoration Simplification

#### Current Implementation
```cpp
// Complex hybrid logic checking multiple conditions
void InterruptManager::HandleRestoration() {
    // Check key sensor states
    if (keyPresentSensor_->GetState() && keyNotPresentSensor_->GetState()) {
        // Complex state checking
    }
    // Multiple conditional branches
}
```

#### Target Architecture
```cpp
// Simple centralized restoration
void InterruptManager::HandleRestoration() {
    // Check if any restore-triggering interrupt is inactive
    for (const auto& interrupt : restorationInterrupts_) {
        if (!interrupt.isActive) {
            panelManager_->RestorePreviousPanel();
            return;
        }
    }
}
```

#### Migration Tasks
- [ ] Identify all restoration-triggering interrupts
- [ ] Add isActive flag to interrupt structure
- [ ] Simplify HandleRestoration() to single responsibility
- [ ] Remove complex state checking logic
- [ ] Update panel manager restoration interface
- [ ] Test all restoration scenarios

### 4. Interrupt Structure Optimization

#### Current Implementation
```cpp
struct Interrupt {
    InterruptType type;
    InterruptResult (*processFunc)(void* context);
    ExecutionMode executionMode;  // Legacy field
    ExclusionGroup exclusionGroup;  // Legacy field
    void* context;
};
```

#### Target Architecture (29-byte optimized)
```cpp
struct Interrupt {
    uint8_t id;                           // 1 byte
    InterruptType type;                   // 1 byte
    InterruptEffect effect;                // 1 byte
    bool (*evaluationFunc)(void*);        // 8 bytes
    void (*executionFunc)(void*);         // 8 bytes
    union {                               // 8 bytes
        PanelType panel;
        Theme theme;
        ButtonAction action;
    } data;
    uint8_t flags;                        // 1 byte (active, etc.)
    uint8_t padding;                      // 1 byte
};  // Total: 29 bytes aligned to 32
```

#### Migration Tasks
- [ ] Define new optimized interrupt structure
- [ ] Remove legacy fields (executionMode, exclusionGroup)
- [ ] Implement union-based data storage
- [ ] Add interrupt ID for tracking
- [ ] Update all interrupt registrations
- [ ] Verify memory footprint reduction

## Partial Implementation Fixes

### 5. Interrupt Processing Flow Alignment

#### Issues
- Queued handler uses timeout-based queue instead of direct execution
- Method signatures don't match documented interface

#### Tasks
- [ ] Align QueuedHandler::Process() with documented flow
- [ ] Remove timeout-based queue management
- [ ] Implement direct queued execution during idle
- [ ] Update method signatures to match IHandler interface

### 6. Error Panel Integration

#### Issues
- Error sensor is debug-only
- Error panel not fully integrated into main flow

#### Tasks
- [ ] Implement production error sensor
- [ ] Integrate error evaluation into main loop
- [ ] Add error panel to standard panel rotation
- [ ] Define error conditions and thresholds

## Implementation Priority Matrix

### Phase 1: Critical Foundation (Weeks 1-2)
**Priority: HIGH | Risk: HIGH | Impact: HIGH**

1. **Interrupt Structure Refactoring**
   - Define new structure
   - Create migration shims for compatibility
   - Update InterruptManager core logic

2. **Evaluation/Execution Separation**
   - Implement dual-function approach
   - Migrate existing interrupts incrementally
   - Maintain backward compatibility during transition

### Phase 2: Sensor Logic Migration (Weeks 3-4)
**Priority: HIGH | Risk: MEDIUM | Impact: HIGH**

1. **Button Timing to Sensor**
   - Refactor ActionButtonSensor
   - Move timing logic from manager
   - Comprehensive testing

2. **Sensor State Management**
   - Standardize sensor interfaces
   - Implement proper change detection
   - Add sensor unit tests

### Phase 3: Simplification (Weeks 5-6)
**Priority: MEDIUM | Risk: LOW | Impact: MEDIUM**

1. **Restoration Logic Simplification**
   - Identify restoration triggers
   - Simplify HandleRestoration()
   - Test edge cases

2. **Legacy Code Removal**
   - Remove deprecated fields
   - Clean up old comments
   - Update documentation

### Phase 4: Optimization (Week 7)
**Priority: LOW | Risk: LOW | Impact: LOW**

1. **Memory Optimization**
   - Verify 29-byte structure alignment
   - Profile memory usage
   - Optimize data structures

2. **Performance Tuning**
   - Profile interrupt latency
   - Optimize hot paths
   - Validate timing accuracy

## Testing Strategy

### Unit Tests Required
- [ ] ActionButtonSensor timing scenarios (short/long/timeout)
- [ ] Interrupt evaluation/execution separation
- [ ] Restoration logic with multiple triggers
- [ ] Handler sensor ownership lifecycle
- [ ] Factory dependency injection

### Integration Tests Required
- [ ] Full interrupt flow from GPIO to panel change
- [ ] Button press sequences with timing validation
- [ ] Panel restoration scenarios
- [ ] Theme switching during various states
- [ ] Error panel activation/deactivation

### Performance Tests Required
- [ ] Interrupt latency measurement
- [ ] Memory footprint validation (< 320KB)
- [ ] Button response time (< 50ms)
- [ ] Panel switching performance

## Migration Guidelines

### For Interrupt Refactoring
```cpp
// Step 1: Add compatibility layer
class InterruptAdapter {
    static bool EvaluateFromProcess(void* context) {
        auto result = legacyProcessFunc(context);
        return result == NEEDS_EXECUTION;
    }
    
    static void ExecuteFromProcess(void* context) {
        legacyProcessFunc(context);
    }
};

// Step 2: Gradually migrate each interrupt
// Step 3: Remove adapter once complete
```

### For Button Sensor Migration
```cpp
// Step 1: Extend ActionButtonSensor with timing
// Step 2: Add parallel implementation
// Step 3: A/B test both implementations
// Step 4: Switch over when validated
// Step 5: Remove old implementation
```

## Risk Mitigation

### High Risk Areas
1. **Interrupt System Changes**
   - Mitigation: Implement compatibility layer
   - Rollback: Feature flag for old/new system

2. **Button Timing Migration**
   - Mitigation: Parallel implementation
   - Rollback: Keep old logic available

### Medium Risk Areas
1. **Restoration Logic Changes**
   - Mitigation: Comprehensive testing
   - Rollback: Version control checkpoints

2. **Memory Structure Changes**
   - Mitigation: Profile before/after
   - Rollback: Conditional compilation

## Success Metrics

### Functional Metrics
- [ ] All unit tests passing (100%)
- [ ] All integration tests passing (100%)
- [ ] No regression in button response time
- [ ] Panel switching remains smooth

### Performance Metrics
- [ ] Memory usage < 300KB (currently ~320KB)
- [ ] Interrupt latency < 1ms
- [ ] Button detection < 50ms
- [ ] Zero memory leaks

### Code Quality Metrics
- [ ] Architecture alignment > 95%
- [ ] No legacy code remnants
- [ ] Documentation complete
- [ ] Test coverage > 80%

## Documentation Updates Required

### Architecture Documents
- [ ] Update architecture-overview.md with actual implementation
- [ ] Revise application-flow.md with correct flow
- [ ] Add migration guide for developers
- [ ] Update README with new build instructions

### Code Documentation
- [ ] Document new interrupt structure
- [ ] Add sensor timing documentation
- [ ] Update handler interface docs
- [ ] Create restoration flow diagram

## Timeline Summary

| Week | Phase | Key Deliverables |
|------|-------|-----------------|
| 1-2  | Foundation | New interrupt structure, evaluation/execution separation |
| 3-4  | Sensor Logic | Button timing migration, sensor standardization |
| 5-6  | Simplification | Restoration logic, legacy removal |
| 7    | Optimization | Memory/performance tuning |
| 8    | Documentation | Complete documentation updates |

## Next Steps

1. **Immediate Actions**
   - [ ] Review and approve this plan
   - [ ] Set up feature branches for each phase
   - [ ] Create test harnesses for validation
   - [ ] Begin Phase 1 implementation

2. **Communication**
   - [ ] Share plan with team
   - [ ] Set up weekly progress reviews
   - [ ] Document decisions and changes

3. **Preparation**
   - [ ] Set up performance profiling tools
   - [ ] Create rollback procedures
   - [ ] Prepare test environments

## Appendix A: File Impact Analysis

### Files Requiring Major Changes
- `include/managers/interrupt_manager.h/cpp` - Core refactoring
- `include/sensors/action_button_sensor.h/cpp` - Timing logic addition
- `include/interfaces/i_handler.h` - Interface updates
- `include/utilities/types.h` - Structure definitions

### Files Requiring Minor Changes
- `include/handlers/queued_handler.h/cpp` - Use new sensor interface
- `include/handlers/polled_handler.h/cpp` - Update process signature
- `src/main.cpp` - Initialization updates
- All panel files - Update interrupt registration

### Files for Removal
- Legacy interrupt-related code sections
- Deprecated execution mode logic
- Old exclusion group implementations

## Appendix B: Code Examples

### Example: New Interrupt Registration
```cpp
// Before
interruptManager->RegisterInterrupt({
    .type = QUEUED,
    .processFunc = &ProcessButtonPress,
    .executionMode = EXCLUSIVE,
    .exclusionGroup = BUTTON_GROUP,
    .context = this
});

// After
interruptManager->RegisterInterrupt({
    .id = BUTTON_SHORT_PRESS,
    .type = QUEUED,
    .effect = BUTTON_ACTION,
    .evaluationFunc = &EvaluateButtonPress,
    .executionFunc = &ExecuteButtonPress,
    .data = { .action = SHORT_PRESS },
    .flags = 0
});
```

### Example: Simplified Restoration
```cpp
// Before (complex checking)
void HandleRestoration() {
    if (keyPresent && !keyNotPresent && lockActive) {
        // Complex logic
        if (panelManager->GetCurrentPanel() == KEY_PANEL) {
            // More conditions
        }
    }
}

// After (simple and clear)
void HandleRestoration() {
    if (ShouldRestore()) {
        panelManager->RestorePreviousPanel();
    }
}

bool ShouldRestore() {
    return !HasActiveRestorationTrigger();
}
```

## Conclusion

This plan provides a systematic approach to aligning the Clarity codebase with its documented architecture. The phased implementation minimizes risk while ensuring continuous functionality. Success depends on careful execution, comprehensive testing, and clear communication throughout the process.

The estimated 8-week timeline allows for thorough implementation and testing of each phase. The compatibility layers and rollback procedures ensure that the system remains stable during the transition.

Upon completion, the codebase will achieve >95% alignment with the documented architecture, improving maintainability, performance, and developer experience.