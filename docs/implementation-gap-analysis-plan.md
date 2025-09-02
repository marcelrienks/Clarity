# Implementation Gap Analysis & Requirements Plan

**Date**: 2025-09-02  
**Current Branch**: int_three  
**Analysis Scope**: Compare documented architecture (docs/) with actual implementation (src/include/)

## Executive Summary

The Clarity project has comprehensive documentation for a sophisticated Trigger/Action interrupt architecture, but the current implementation uses a different, older interrupt system. This document outlines the gaps between the documented requirements and actual code, providing a prioritized plan to achieve the documented architecture.

## Current Implementation Analysis

### What Currently Exists

1. **Interrupt System Architecture**:
   - `InterruptManager` with `PolledHandler` and `QueuedHandler`
   - Single `Interrupt` struct with unified handling
   - Handler-owned sensors (PolledHandler owns GPIO sensors, QueuedHandler owns ButtonSensor)
   - Basic priority system with `Priority` enum (CRITICAL=0, IMPORTANT=1, NORMAL=2)
   - Simple interrupt registration and execution

2. **Main Loop Structure**:
   - LVGL tasks processing
   - Interrupt processing during UI idle
   - Error panel checking
   - Panel updating

3. **Sensor Architecture**:
   - `BaseSensor` with change detection template
   - Split key sensors: `KeyPresentSensor` and `KeyNotPresentSensor`
   - Handler ownership model working correctly

4. **Panel System**:
   - Basic panel management via `PanelManager`
   - Panel creation through factories
   - Button integration exists but basic

## Documented Architecture Requirements (Not Implemented)

### 1. Trigger/Action Separation Architecture

**Documented**: Clear separation between state-based Triggers and event-based Actions
```cpp
// Required Trigger structure
struct Trigger {
    const char* id;
    Priority priority;
    TriggerType type;
    void (*activateFunc)();
    void (*deactivateFunc)();
    BaseSensor* sensor;
    bool isActive;
};

// Required Action structure  
struct Action {
    const char* id;
    void (*executeFunc)();
    bool hasTriggered;
    ActionPress pressType;
};
```

**Current**: Single `Interrupt` struct handles both concepts
```cpp
// Current implementation
struct Interrupt {
    const char* id;
    Priority priority;
    InterruptSource source;  // POLLED or QUEUED
    void (*execute)(void* context);
    void* context;
    // ... additional fields
};
```

**Gap**: Complete architectural redesign needed

### 2. TriggerHandler/ActionHandler Architecture

**Documented**: Separate handlers with distinct responsibilities
- **TriggerHandler**: GPIO state monitoring with dual functions
- **ActionHandler**: Button event processing with timing

**Current**: PolledHandler + QueuedHandler with different responsibilities
- **PolledHandler**: GPIO monitoring but single execute function
- **QueuedHandler**: Button handling but limited timing logic

**Gap**: Handlers exist but don't match documented interface and behavior

### 3. Priority-Based Override Logic

**Documented**: Sophisticated blocking with same-type restoration
- Higher priority triggers block lower priority activation
- Same TriggerType restoration on deactivation
- Smart restoration to last user panel

**Current**: Basic priority ordering, no blocking logic or restoration chains

**Gap**: Complete override logic system missing

### 4. Processing Model

**Documented**: 
- Actions evaluate ALWAYS (every main loop)
- Triggers evaluate/execute only during IDLE
- Processing order: Triggers before Actions

**Current**:
- Both handlers process during idle only
- No continuous action evaluation

**Gap**: Evaluation timing model incorrect

### 5. Button System Integration

**Documented**: Universal button functions via IActionService
- Panel implements IActionService interface
- Action interrupts inject current panel functions
- Short press (50ms-2000ms) and long press (2000ms-5000ms)

**Current**: Basic button integration exists but not following documented pattern

**Gap**: IActionService interface missing, function injection incomplete

## Implementation Plan

### Phase 1: Core Architecture Foundation (Priority 1)
**Estimated Effort**: 2-3 weeks

#### 1.1 Create Trigger/Action Type System
- [ ] Implement `TriggerType` enum (PANEL, STYLE, FUNCTION)
- [ ] Implement `ActionPress` enum (SHORT, LONG)
- [ ] Create separate `Trigger` and `Action` structs per documentation
- [ ] Update `types.h` with new structures

#### 1.2 Create TriggerHandler
- [ ] Implement `TriggerHandler` class inheriting from `IHandler`
- [ ] Move GPIO sensor ownership from PolledHandler
- [ ] Implement dual function system (activateFunc/deactivateFunc)
- [ ] Add priority-based blocking logic
- [ ] Implement same-type restoration logic

#### 1.3 Create ActionHandler  
- [ ] Implement `ActionHandler` class inheriting from `IHandler`
- [ ] Move ButtonSensor ownership from QueuedHandler
- [ ] Implement press duration detection (50ms-2000ms, 2000ms-5000ms)
- [ ] Add event queuing system
- [ ] Implement continuous evaluation model

#### 1.4 Update InterruptManager
- [ ] Coordinate TriggerHandler and ActionHandler
- [ ] Implement evaluation timing model (Actions always, Triggers idle only)
- [ ] Remove old PolledHandler/QueuedHandler system
- [ ] Update interrupt registration to route to correct handler

### Phase 2: Priority and Restoration Logic (Priority 1)  
**Estimated Effort**: 1-2 weeks

#### 2.1 Implement Priority Override Logic
- [ ] Higher priority trigger blocks lower priority activation  
- [ ] Suppress execution while maintaining isActive state
- [ ] Priority comparison logic (CRITICAL > IMPORTANT > NORMAL)

#### 2.2 Implement Type-Based Restoration
- [ ] Track active triggers by TriggerType
- [ ] Find highest priority same-type trigger on deactivation
- [ ] Execute restoration chain properly
- [ ] Implement user panel fallback

#### 2.3 Update PanelManager Integration
- [ ] Track last user-driven panel
- [ ] Implement trigger-driven vs user-driven panel distinction  
- [ ] Add restoration checking mechanism
- [ ] Update panel loading to work with new trigger system

### Phase 3: Button System and Panel Integration (Priority 2)
**Estimated Effort**: 1-2 weeks

#### 3.1 Create IActionService Interface
- [ ] Define universal button function interface
- [ ] Add short press and long press function signatures
- [ ] Update all panels to implement IActionService

#### 3.2 Implement Function Injection System
- [ ] Action interrupts receive current panel's functions
- [ ] Dynamic function updates when panels switch
- [ ] Context management for panel execution

#### 3.3 Update Panel Requirements
- [ ] All panels implement documented button behaviors:
  - Oil: No short press, long press → config
  - Splash: Short press → skip, long press → config  
  - Key: Display only (no functions)
  - Lock: Display only (no functions)
  - Error: Short → cycle errors, long → clear
  - Config: Short → navigate, long → select

### Phase 4: Memory Optimization and Safety (Priority 2)
**Estimated Effort**: 1 week

#### 4.1 Static Function Pointers
- [ ] Ensure all interrupt callbacks use static function pointers
- [ ] Remove any std::function usage that could cause heap fragmentation
- [ ] Implement direct singleton calls to eliminate context parameters

#### 4.2 Memory-Efficient Design
- [ ] Optimize interrupt structure sizes for ESP32
- [ ] Implement static arrays with MAX_INTERRUPTS limits
- [ ] Add memory usage monitoring and validation

### Phase 5: Advanced Features (Priority 3)
**Estimated Effort**: 1 week

#### 5.1 Complete Error System Integration
- [ ] Implement error trigger with CRITICAL priority
- [ ] Add error sensor for trigger evaluation
- [ ] Integrate with existing ErrorManager
- [ ] Test error panel activation and restoration

#### 5.2 Debug Error Sensor
- [ ] Verify debug error sensor integration (GPIO 34)
- [ ] Test debug builds vs release builds
- [ ] Validate debug error triggering

### Phase 6: Testing and Validation (Priority 1)
**Estimated Effort**: 1-2 weeks

#### 6.1 Test Scenarios Implementation
- [ ] Implement all test scenarios from `docs/scenarios.md`
- [ ] Create automated tests for interrupt scenarios
- [ ] Validate priority override behavior
- [ ] Test restoration chains

#### 6.2 Performance Validation  
- [ ] Measure interrupt processing performance
- [ ] Validate memory usage within ESP32 constraints
- [ ] Test under various load conditions
- [ ] Validate LVGL integration

## Risk Assessment

### High Risk Items
1. **Complete Architecture Redesign**: Current implementation needs significant changes
2. **Memory Constraints**: ESP32 has limited RAM (~250KB available)
3. **LVGL Integration**: Interrupt timing must not conflict with LVGL tasks
4. **Existing Functionality**: Must maintain all current working features

### Mitigation Strategies
1. **Incremental Development**: Implement in phases to maintain working system
2. **Extensive Testing**: Use existing test scenarios throughout development
3. **Memory Monitoring**: Continuous memory usage tracking
4. **Rollback Plan**: Maintain current working system until new system validated

## Success Criteria

### Phase 1 Success
- [ ] TriggerHandler processes GPIO sensors with dual functions
- [ ] ActionHandler processes button events with timing
- [ ] Basic priority system working
- [ ] All existing functionality preserved

### Phase 2 Success  
- [ ] Priority override logic working correctly
- [ ] Type-based restoration chains functional
- [ ] Smart restoration to user panels
- [ ] All test scenarios from docs passing

### Phase 3 Success
- [ ] All panels implement documented button behaviors
- [ ] Universal button functions working
- [ ] Panel switching and restoration working correctly

### Final Success Criteria
- [ ] All documentation requirements implemented
- [ ] All test scenarios passing
- [ ] Memory usage within ESP32 constraints (< 200KB)
- [ ] Performance targets met (interrupt processing < 100ms)
- [ ] System stable under load testing

## Development Notes

### Current Strengths to Preserve
1. Handler-owned sensors working correctly
2. BaseSensor change detection pattern solid
3. Factory pattern implementation good
4. Error system foundation exists
5. Panel management working

### Implementation Dependencies
1. TriggerHandler/ActionHandler must be implemented together
2. Priority system depends on new trigger structures
3. Panel integration depends on IActionService interface
4. Testing depends on all phases being functional

### Resource Requirements
- **Development Time**: 6-10 weeks total
- **Testing Environment**: PlatformIO with debug builds
- **Hardware**: ESP32 board with all sensors connected
- **Memory Profiling**: Tools for ESP32 memory analysis

## Conclusion

The gap between documented architecture and current implementation is significant but manageable. The current system provides a solid foundation with handler-owned sensors and basic interrupt processing. The main work involves restructuring the interrupt architecture to match the sophisticated Trigger/Action separation documented in requirements.

The phased approach ensures continuous functionality while making incremental progress toward the target architecture. Priority 1 phases focus on core functionality, while Priority 2 and 3 phases add advanced features and optimizations.

Success depends on careful testing at each phase and maintaining ESP32 memory constraints throughout development.