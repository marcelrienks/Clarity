# int_three Implementation Plan

## Executive Summary

This document outlines a comprehensive plan to bring the Clarity ESP32 automotive gauge system codebase into alignment with the documented architecture. The current implementation deviates significantly from the documented handler-based interrupt architecture, requiring major structural changes to achieve the intended design.

## Current State Analysis

### Architecture Gaps Identified

#### 1. **Critical Gap: Handler-Based Interrupt System Missing**
- **Current**: Traditional TriggerManager with direct sensor polling
- **Required**: TriggerHandler and ActionHandler implementing IHandler interface with function pointer architecture
- **Impact**: Complete interrupt system redesign needed

#### 2. **Critical Gap: Split Sensor Architecture Missing** 
- **Current**: Single KeySensor handles both GPIO pins (25 & 26)
- **Required**: Independent KeyPresentSensor and KeyNotPresentSensor classes
- **Impact**: Sensor duplication prevention and resource conflict resolution required

#### 3. **Critical Gap: Function Pointer Architecture Missing**
- **Current**: std::function-based callbacks causing potential heap fragmentation
- **Required**: Static function pointer architecture for ESP32 memory safety
- **Impact**: Complete callback system redesign for memory stability

#### 4. **Critical Gap: Change-Based Trigger System Missing**
- **Current**: State-based trigger evaluation
- **Required**: Change-detection with phase-based processing using hasChangedFunc/getCurrentStateFunc separation
- **Impact**: Performance optimization and corruption prevention needed

#### 5. **Critical Gap: BaseSensor Change Detection Missing**
- **Current**: No consistent change detection pattern
- **Required**: BaseSensor base class with DetectChange template method
- **Impact**: All sensors need inheritance restructure

## Implementation Strategy

Each phase is designed to be independently buildable and testable, allowing for incremental development with manual validation at each stage.

### Phase 1: Foundation Infrastructure and Factory Pattern (Week 1)

**Goal**: Establish the architectural foundation with proper factory pattern while maintaining existing functionality

#### 1.1 Create Handler Interface Architecture
**Priority**: Critical
**Risk**: High - Core architectural change

**Tasks**:
1. Create `include/interfaces/i_handler.h`:
```cpp
class IHandler {
public:
    virtual ~IHandler() = default;
    virtual void Process() = 0;
};
```

2. Add interrupt struct definitions to `include/utilities/types.h`:
```cpp
struct TriggerInterrupt {
    const char* id;
    TriggerPriority priority;
    bool (*hasChangedFunc)();
    bool (*getCurrentStateFunc)();
    void (*executionFunc)();
};

struct ActionInterrupt {
    const char* id;
    Priority priority;
    bool (*evaluationFunc)();
    void (*executionFunc)();
    ActionType actionType;
};
```

3. Create dummy implementation stubs that maintain existing behavior
4. Update InterruptManager to support both old and new interfaces (compatibility layer)
5. Remove ComponentFactory references and update factory pattern

**Build/Test Verification**:
- Project builds successfully with new interfaces and factory pattern
- Existing TriggerManager functionality unchanged
- InterruptManager can register both old and new handler types
- ManagerFactory creates only managers and providers
- Panels and managers create their own dependencies
- All existing panels continue to work normally

#### 1.2 Implement BaseSensor Architecture
**Priority**: Critical
**Risk**: Medium - Affects all sensors

**Tasks**:
1. Create `include/sensors/base_sensor.h`:
```cpp
class BaseSensor {
protected:
    bool initialized_ = false;
    template<typename T>
    bool DetectChange(T currentValue, T& previousValue);
};
```

2. Add change detection methods to ISensor interface (optional overrides)
3. Create compatibility wrappers for existing sensors
4. Update sensor creation to be handler/panel responsibility
5. Add proper factory pattern sensor ownership documentation

**Build/Test Verification**:
- BaseSensor template compiles with all data types
- Existing sensors build without modification
- Change detection template works for boolean, int32_t, and double
- Memory usage remains stable

### Phase 2: Split Sensor Implementation (Week 2)

**Goal**: Implement independent sensor classes while maintaining backward compatibility

#### 2.1 Implement Independent Key Sensors
**Priority**: Critical  
**Risk**: High - Hardware interface changes

**Tasks**:
1. Create `include/sensors/key_present_sensor.h`:
```cpp
class KeyPresentSensor : public ISensor, public BaseSensor {
public:
    KeyPresentSensor(IGpioProvider* gpio);
    ~KeyPresentSensor(); // Proper GPIO cleanup
    void Init() override;
    Reading GetReading() override;
    bool HasStateChanged();
    bool GetKeyPresentState();
private:
    IGpioProvider* gpioProvider_;
    bool previousState_ = false;
};
```

2. Create `include/sensors/key_not_present_sensor.h` (independent implementation)
3. Update existing KeySensor to use new sensors internally (adapter pattern)
4. Implement proper destructors with DetachInterrupt() calls

**Build/Test Verification**:
- Both new sensor classes compile independently
- KeySensor continues to work with existing code
- GPIO resource conflicts are prevented
- Destructors properly clean up GPIO interrupts
- Can manually test each sensor independently via GPIO manipulation

#### 2.2 Update Remaining Sensors
**Priority**: High
**Risk**: Medium - Systematic changes

**Tasks**:
1. Update LockSensor to inherit from BaseSensor (add HasStateChanged method)
2. Update LightsSensor to inherit from BaseSensor
3. Update ActionButtonSensor for change detection compliance
4. Create sensor unit injection interfaces (without breaking existing functionality)

**Build/Test Verification**:
- All sensors inherit from BaseSensor without breaking existing functionality
- Each sensor can be tested independently
- Change detection methods return consistent results
- Sensor readings remain accurate and stable

### Phase 3: Handler Implementation with Sensor Creation (Week 3)

**Goal**: Create working handler implementations with proper sensor creation that can coexist with existing managers

#### 3.1 Implement TriggerHandler
**Priority**: Critical
**Risk**: High - Complex phase-based processing

**Tasks**:
1. Create `include/handlers/trigger_handler.h`:
```cpp
class TriggerHandler : public IHandler {
public:
    TriggerHandler(IGpioProvider* gpio); // Creates own sensors
    void RegisterTrigger(const TriggerInterrupt& trigger);
    void Process() override;
private:
    // Owned sensors created in constructor
    std::unique_ptr<KeyPresentSensor> keyPresentSensor_;
    std::unique_ptr<KeyNotPresentSensor> keyNotPresentSensor_;
    std::unique_ptr<LockSensor> lockSensor_;
    std::unique_ptr<LightsSensor> lightsSensor_;
    
    std::vector<TriggerInterrupt> triggers_;
    void ProcessPhase1_ChangeDetection();
    void ProcessPhase2_StateEvaluation();
    void ProcessPhase3_ActionExecution();
    void ProcessPhase4_RestoreHandling();
};
```

2. Implement sensor creation in constructor with proper ownership
3. Implement phase-based processing with extensive logging for verification
4. Create test trigger registrations using owned sensors
5. Implement priority-based restoration blocking

**Build/Test Verification**:
- TriggerHandler compiles and instantiates without errors
- TriggerHandler creates and owns all trigger sensors internally
- Can register test triggers and verify phase processing
- Each processing phase can be manually verified through logging
- Priority system works correctly with test scenarios
- Handler can run alongside existing TriggerManager for comparison
- No GPIO resource conflicts between handlers and existing code

#### 3.2 Implement ActionHandler  
**Priority**: High
**Risk**: Medium - Simpler than TriggerHandler

**Tasks**:
1. Create `include/handlers/action_handler.h` implementing IHandler
2. ActionHandler constructor creates and owns ActionButtonSensor
3. Create test action registrations for button functionality
4. Implement action evaluation that respects trigger precedence
5. Add comprehensive logging for action processing

**Build/Test Verification**:
- ActionHandler compiles and processes actions correctly
- ActionHandler creates and owns ActionButtonSensor internally
- Button press detection works with correct timing
- Actions only execute when no triggers are active
- Can manually test button behavior independently
- No GPIO resource conflicts with ActionButtonSensor creation

### Phase 4: Static Callback System (Week 4)

**Goal**: Replace function objects with static function pointers for memory safety

#### 4.1 Create Static Callback Architecture
**Priority**: Critical
**Risk**: High - Memory stability critical for ESP32

**Tasks**:
1. Create `include/utilities/interrupt_callbacks.h`:
```cpp
struct InterruptCallbacks {
    // Key sensor callbacks
    static bool KeyPresentChanged(void* context);
    static bool KeyPresentState(void* context);
    static void LoadKeyPanel(void* context);
    
    // Add callbacks for all sensors/actions
};
```

2. Update TriggerInterrupt and ActionInterrupt structs to use context parameters
3. Create test registrations using static callbacks
4. Add memory monitoring utilities

**Build/Test Verification**:
- Static callbacks compile and execute correctly
- Context parameter passing works for all sensor types
- Memory usage remains stable during callback execution
- Can compare memory usage before/after static callback implementation

#### 4.2 Convert Handler Registrations
**Priority**: High
**Risk**: Medium - Systematic callback updates

**Tasks**:
1. Update all TriggerHandler registrations to use static callbacks
2. Update all ActionHandler registrations to use static callbacks
3. Create registration helper macros to reduce boilerplate
4. Add callback validation utilities

**Build/Test Verification**:
- All handler registrations use function pointer architecture
- Callback patterns are consistent and maintainable
- Context passing works correctly for all scenarios
- Manual testing verifies all callbacks execute properly

### Phase 5: Manager Replacement (Week 5)

**Goal**: Replace TriggerManager with handler system while maintaining functionality

#### 5.1 Handler Integration
**Priority**: Critical
**Risk**: High - System-wide changes

**Tasks**:
1. Update ManagerFactory to create handlers alongside existing managers
2. Update InterruptManager to use both old and new systems in parallel
3. Create feature flag to switch between old and new interrupt processing
4. Add extensive comparison logging between old and new systems

**Build/Test Verification**:
- Both old and new interrupt systems can run in parallel
- Handler system produces identical behavior to manager system
- Can switch between systems via feature flag for comparison
- All existing functionality works with handler system

#### 5.2 Complete Factory Pattern Migration
**Priority**: High  
**Risk**: Medium - Resource management critical

**Tasks**:
1. Remove all sensor creation from ManagerFactory
2. Update InterruptManager to create handlers with sensor ownership
3. Update PanelManager to create panels on demand
4. Ensure trigger panels remain display-only but create own components
5. Add resource conflict detection and reporting

**Build/Test Verification**:
- ManagerFactory creates only managers and providers
- Each GPIO has exactly one sensor instance owned by handlers/panels
- Resource conflicts are detected and prevented
- Trigger panels work correctly with direct GPIO reads
- Data panels create own sensors and components
- Memory leaks are prevented through proper cleanup verification

### Phase 6: Finalization and Optimization (Week 6)

**Goal**: Remove legacy code and optimize performance

#### 6.1 Legacy System Removal
**Priority**: Medium
**Risk**: Low - Cleanup phase

**Tasks**:
1. Remove TriggerManager and associated legacy code
2. Remove compatibility layers and feature flags
3. Clean up unused interfaces and methods (including ComponentFactory)
4. Update all references to use handler system exclusively
5. Verify proper factory pattern implementation throughout codebase

**Build/Test Verification**:
- System builds cleanly without legacy code and ComponentFactory
- All functionality works correctly with handlers only
- Proper factory pattern implemented throughout
- No unused code or interfaces remain
- Performance characteristics meet requirements

#### 6.2 Performance Optimization
**Priority**: Medium
**Risk**: Low - Performance improvement

**Tasks**:
1. Optimize change detection performance monitoring
2. Verify single evaluation rule compliance
3. Add performance metrics reporting
4. Optimize memory usage patterns

**Build/Test Verification**:
- Theme changes occur maximum 2 times per second
- CPU usage during idle periods is minimized
- Interrupt processing time is consistent
- Memory fragmentation is eliminated

## Risk Assessment and Mitigation

### High-Risk Areas

#### 1. Function Pointer Architecture Implementation
**Risk**: System crashes due to incorrect callback patterns
**Mitigation**: 
- Implement and test one callback type at a time with manual verification
- Create simple test functions to verify callback signatures
- Use compiler warnings to catch function signature mismatches

#### 2. Sensor Resource Management
**Risk**: GPIO conflicts and resource leaks
**Mitigation**:
- Implement resource tracking utilities for manual verification
- Test resource cleanup manually using GPIO state inspection
- Use RAII patterns for automatic resource management

#### 3. Phase-Based Processing Complexity
**Risk**: Race conditions and state corruption
**Mitigation**:
- Implement comprehensive logging for manual state validation
- Create simple test scenarios for deterministic verification
- Add state verification utilities for manual inspection

### Medium-Risk Areas

#### 1. Memory Usage Changes
**Risk**: ESP32 memory constraints exceeded
**Mitigation**:
- Monitor memory usage manually at each phase
- Create memory reporting utilities for manual inspection
- Test on actual hardware at each phase completion

#### 2. Performance Changes
**Risk**: System responsiveness degradation
**Mitigation**:
- Establish performance baselines before each phase
- Monitor interrupt processing times manually
- Test with simulated sensor data at each phase

## Manual Testing Approach

### Per-Phase Testing Requirements

Each phase includes specific manual testing procedures:

#### Phase 1 Testing
- Verify new interfaces compile without breaking existing code
- Manually test all existing panel functionality remains intact
- Use serial logging to verify both old and new systems can coexist

#### Phase 2 Testing  
- Manually toggle GPIO pins to test individual sensor responses
- Verify change detection works by observing sensor state changes
- Test sensor independence by activating sensors separately

#### Phase 3 Testing
- Use logging to manually verify phase-based processing order
- Test trigger priorities by activating multiple triggers simultaneously  
- Manually verify action execution timing with button presses

#### Phase 4 Testing
- Monitor memory usage before/after callback conversion
- Manually verify all callbacks execute with correct context
- Test extended operation to verify memory stability

#### Phase 5 Testing
- Compare old vs new system behavior side-by-side using feature flag
- Manually verify all trigger/action scenarios work identically
- Test sensor ownership enforcement by attempting resource conflicts

#### Phase 6 Testing
- Verify clean build after legacy code removal
- Manually test complete system functionality
- Monitor performance metrics to verify optimization goals

## Success Criteria

### Functional Requirements (Per Phase)
- [ ] **Phase 1**: All existing panel switching functionality works correctly after interface additions
- [ ] **Phase 2**: Split sensors operate independently without conflicts  
- [ ] **Phase 3**: Handlers process interrupts correctly alongside existing managers
- [ ] **Phase 4**: Static callbacks execute without memory issues
- [ ] **Phase 5**: Handler system provides identical functionality to manager system
- [ ] **Phase 6**: System operates correctly with only handler architecture

### Performance Requirements (Final Phase)
- [ ] Theme changes occur maximum 2 times per second
- [ ] Interrupt processing time is consistent regardless of trigger states
- [ ] CPU usage during idle periods is minimized
- [ ] Change detection eliminates redundant operations

### Architecture Requirements (Final State)
- [ ] TriggerHandler and ActionHandler implement IHandler interface
- [ ] All sensors inherit from BaseSensor for change detection
- [ ] KeyPresentSensor and KeyNotPresentSensor are independent classes
- [ ] Function pointer architecture eliminates heap fragmentation
- [ ] Phase-based processing prevents change detection corruption

### Quality Requirements (Ongoing)
- [ ] Code follows project naming standards consistently
- [ ] All public APIs have comprehensive documentation
- [ ] Memory management follows ESP32 best practices
- [ ] Error handling provides graceful degradation

## Timeline and Milestones

### Week 1: Foundation Infrastructure
- **Milestone**: IHandler interface and BaseSensor architecture complete
- **Deliverable**: Buildable code with new architectural foundation
- **Manual Test**: Verify existing functionality unchanged

### Week 2: Split Sensor Implementation  
- **Milestone**: Independent key sensors operational
- **Deliverable**: KeyPresentSensor and KeyNotPresentSensor working independently
- **Manual Test**: GPIO manipulation shows independent sensor responses

### Week 3: Handler Implementation
- **Milestone**: TriggerHandler and ActionHandler operational
- **Deliverable**: Handlers processing interrupts correctly in parallel with managers
- **Manual Test**: Side-by-side behavior comparison via logging

### Week 4: Static Callback System
- **Milestone**: Function pointer architecture implemented
- **Deliverable**: Memory-safe callback system operational
- **Manual Test**: Extended operation shows stable memory usage

### Week 5: Manager Replacement
- **Milestone**: Handler system replaces manager system
- **Deliverable**: Feature-complete handler architecture
- **Manual Test**: All scenarios work identically between old/new systems

### Week 6: Finalization and Optimization
- **Milestone**: Legacy code removed, performance optimized
- **Deliverable**: Production-ready handler-based architecture
- **Manual Test**: Complete system validation meets all requirements

## Resource Requirements

### Development Resources
- 1 Embedded systems developer (full-time, 6 weeks)
- Access to ESP32 hardware for manual testing each phase
- PlatformIO development environment
- Wokwi emulator for initial development

### Manual Testing Resources
- Serial console access for logging verification
- GPIO manipulation tools (switches, buttons) for sensor testing
- Basic multimeter for GPIO state verification
- Timing measurement tools for performance validation

## Conclusion

This implementation plan addresses the significant architectural gaps between the current codebase and the documented design. The handler-based interrupt architecture with function pointers is critical for ESP32 memory stability and system reliability. The phased approach minimizes risk while ensuring comprehensive testing at each stage.

The most critical aspects are:
1. Function pointer architecture for memory safety
2. Split sensor design for resource conflict prevention  
3. Phase-based processing for change detection accuracy
4. Proper sensor ownership model for system stability

Success depends on careful implementation of each phase with thorough testing before proceeding to the next stage. The final system will align with the documented architecture while maintaining the reliability and performance requirements of the ESP32 platform.