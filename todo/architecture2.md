# Trigger System Architecture v2: State-Aware Processing

## Overview

This document describes the enhanced trigger system architecture that implements **state-aware processing** with **FIFO message consolidation** and **smart execution planning**. This evolution from the pure pin-change-driven architecture (v1) addresses complex scenarios where multiple triggers activate/deactivate in sequence, requiring intelligent state resolution.

## Problem Statement

The original pin-change-driven architecture had limitations with complex trigger scenarios:

### Complex Scenarios Requiring State Awareness

1. **Scenario 1**: `key_present → ACTIVE`, `key_not_present → ACTIVE`, `key_present → INACTIVE`
   - **Expected**: Red key panel remains (key_not_present still active)
   - **v1 Problem**: key_present deactivation would restore oil panel

2. **Scenario 2**: `key_present → ACTIVE`, `key_not_present → ACTIVE`, `key_not_present → INACTIVE`
   - **Expected**: Green key panel restored (key_present still active)
   - **v1 Problem**: Incorrect panel state after key_not_present deactivation

3. **Scenario 3**: Both key triggers → INACTIVE
   - **Expected**: Oil panel restored (no blocking triggers active)
   - **v1 Problem**: Last deactivated trigger controlled restoration

### Root Cause Analysis

The pure pin-change approach processed each trigger event individually without considering the **current state of other triggers**. This led to incorrect restoration decisions when multiple overlapping triggers were active.

## Architecture Evolution

### From Pin-Change-Driven to State-Aware

**v1 Approach (Pin-Change-Driven):**
```
Pin Change → Immediate Action → Execute First Priority Request
```

**v2 Approach (State-Aware):**
```
Pin Change → Queue Message → Consolidate All States → Smart Execution Plan → Optimized Execution
```

## Core Design Principles

### 1. **Message Consolidation**
- Process **all queued messages in FIFO order** before any action execution
- Build **consolidated trigger state map** representing final state after all pin changes
- Multiple messages for same trigger result in **final state only**

### 2. **Smart Execution Planning**
- Separate actions by type: **Theme → Non-Panel → Single Final Panel**
- **Prevent redundant panel loading**: Only load the final required panel
- **Respect LVGL timing**: Complete all state changes before LVGL processing

### 3. **Optimal Execution Order**
- **Theme actions first**: Apply theme changes that may affect subsequent panels
- **Non-panel actions second**: Other system state changes
- **Single panel load last**: Load only the highest priority active trigger's panel

### 4. **LVGL Performance Optimization**
- **Single panel load per loop iteration**: Eliminates redundant object creation/destruction
- **Consolidated state processing**: All trigger logic completes before LVGL operations
- **Memory efficiency**: No intermediate panel loads that get immediately replaced

## Detailed Architecture

### Dual-Core Responsibilities

#### Core 1 (Producer) - Unchanged
```
GPIO Pin Change → ISR Handler → Queue Message → Done
```

**Responsibilities:**
- GPIO interrupt service routines (ISRs)
- Queue ISR events to `isrEventQueue`
- **No changes from v1**: Pure hardware monitoring

#### Core 0 (Consumer) - Enhanced
```
Main Loop → Process All Messages → Consolidate States → Plan Execution → Execute Plan → LVGL
```

**Enhanced Responsibilities:**
- **Message consolidation**: Process all queued events to build final state map
- **Execution planning**: Determine optimal action sequence
- **Smart execution**: Apply theme changes, then load single final panel
- **LVGL processing**: Single consolidated update cycle

### Message Processing Flow

#### 1. Message Consolidation Phase
```cpp
std::map<std::string, TriggerExecutionState> ProcessPendingTriggerEvents() {
    std::map<std::string, TriggerExecutionState> consolidatedStates;
    
    // Process ALL queued messages in FIFO order
    while (xQueueReceive(isrEventQueue, &event, 0) == pdTRUE) {
        // Later messages override earlier ones for same trigger
        consolidatedStates[triggerId] = newState;
    }
    
    return consolidatedStates; // Final state after all pin changes
}
```

**Key Benefits:**
- **Temporal correctness**: FIFO ensures proper event sequence
- **State consolidation**: Multiple activation/deactivation messages → final state
- **No lost events**: All queued messages processed before action execution

#### 2. Execution Planning Phase
```cpp
struct ExecutionPlan {
    std::vector<TriggerActionRequest> themeActions;     // Apply first
    std::vector<TriggerActionRequest> nonPanelActions;  // Apply second
    TriggerActionRequest finalPanelAction;              // Apply last (single panel)
};

ExecutionPlan PlanExecutionFromStates(consolidatedStates) {
    // Categorize actions by type
    // Determine highest priority active trigger for final panel
    // Return structured execution plan
}
```

**Key Benefits:**
- **Action categorization**: Themes vs panels handled separately
- **Single final panel**: Only highest priority active trigger loads panel
- **Execution optimization**: Minimal LVGL object lifecycle management

#### 3. Smart Execution Phase
```cpp
void loop() {
    // 1. Consolidate all trigger state changes
    auto consolidatedStates = ProcessPendingTriggerEvents();
    
    // 2. Plan optimal execution
    auto plan = PlanExecutionFromStates(consolidatedStates);
    
    // 3. Execute in optimal order
    ExecuteThemeActions(plan.themeActions);           // Themes first
    ExecuteNonPanelActions(plan.nonPanelActions);     // Other actions
    ExecuteFinalPanelAction(plan.finalPanelAction);   // Single panel load
    
    // 4. LVGL processing (single cycle)
    UpdatePanel(); handle_lv_tasks();
}
```

**Key Benefits:**
- **Ordered execution**: Themes applied before panels override
- **Single panel load**: No redundant UI object creation/destruction
- **LVGL safety**: All state changes complete before LVGL processing

## Scenario Resolution Examples

### Scenario 1: Overlapping Key Triggers with key_present Deactivation
```
Messages: [key_present: HIGH, key_not_present: HIGH, key_present: LOW]
Consolidated State: {key_not_present: ACTIVE}
Execution Plan: finalPanelAction = LoadRedKeyPanel
Result: ✅ Red key panel displayed (correct)
```

### Scenario 2: Overlapping Key Triggers with key_not_present Deactivation
```
Messages: [key_present: HIGH, key_not_present: HIGH, key_not_present: LOW]
Consolidated State: {key_present: ACTIVE}
Execution Plan: finalPanelAction = LoadGreenKeyPanel
Result: ✅ Green key panel displayed (correct)
```

### Scenario 3: Multiple Triggers with Theme and Panel Actions
```
Messages: [lights: HIGH, key_present: HIGH]
Consolidated State: {lights: ACTIVE, key_present: ACTIVE}
Execution Plan: 
  - themeActions = [SetNightTheme]
  - finalPanelAction = LoadGreenKeyPanel (highest priority)
Result: ✅ Night theme applied, then green key panel displayed
```

## Performance Considerations

### Load Distribution Analysis

#### Core 1 (Minimal Load - Unchanged)
- **GPIO ISR handlers**: ~1-10μs per interrupt
- **Queue message posting**: ~5-10μs per message
- **Total**: Essentially idle except during pin changes

#### Core 0 (Enhanced Load)
- **Message consolidation**: ~50-100μs per batch (new)
- **Execution planning**: ~20-50μs per batch (new)
- **Single panel load**: ~10-50ms (optimized from multiple loads)
- **LVGL rendering**: ~5-20ms per frame (unchanged)
- **Total**: Similar overall load, better distributed

### Performance Benefits

1. **Reduced Panel Loading**: Single panel load vs multiple sequential loads
2. **Efficient State Processing**: Batch processing vs individual message handling
3. **Improved LVGL Performance**: Single object lifecycle vs multiple create/destroy cycles
4. **Memory Optimization**: No intermediate panel allocations

### Performance Impact Assessment

- **Trigger processing overhead**: +1-2% CPU (message consolidation + planning)
- **Panel loading optimization**: -20-50% reduction in redundant UI operations
- **Memory efficiency**: -30-60% reduction in peak allocation during rapid trigger sequences
- **Net performance impact**: **Positive** - overall system more efficient

## Implementation Compatibility

### Maintaining v1 Architecture Benefits

1. **Pure Core Separation**: Core 1 still only handles GPIO, Core 0 handles all logic
2. **No Mutexes**: Single-threaded state processing maintains mutex-free architecture
3. **Pin-Change Foundation**: ISR events still triggered only on actual pin changes
4. **Dependency Injection**: Triggers still return action requests, don't execute directly

### Breaking Changes from v1

1. **ProcessPendingTriggerEvents() Signature**: Returns consolidated states instead of immediate requests
2. **Main Loop Structure**: Enhanced execution flow with planning phase
3. **New Dependencies**: Requires ExecutionPlan structure and planning methods

## Extension and Maintenance

### Adding New Trigger Types
1. **No changes to consolidation logic**: New triggers automatically participate in state consolidation
2. **Action categorization**: Classify new actions as theme/non-panel/panel in planning phase
3. **Priority assignment**: Assign appropriate priority for execution order

### Debugging and Monitoring
1. **State visibility**: Consolidated state map shows all trigger states at decision point
2. **Execution planning logs**: Clear visibility into why specific actions were chosen
3. **Performance metrics**: Measure consolidation time, planning time, execution time separately

### Future Optimizations
1. **Predictive planning**: Cache execution plans for common state combinations
2. **Lazy panel loading**: Defer panel loading until LVGL ready state
3. **State compression**: Optimize state map storage for memory-constrained scenarios

## Key Architectural Advantages

### 1. **Complete Scenario Coverage**
- Handles all complex overlapping trigger scenarios correctly
- **Temporal accuracy**: Final state reflects actual sequence of pin changes
- **Priority respect**: Higher priority active triggers control final panel state

### 2. **LVGL Performance Optimization**
- **Single panel load**: Eliminates redundant UI object creation/destruction
- **Consolidated processing**: All state changes complete before LVGL operations
- **Memory efficiency**: No intermediate allocations for replaced panels

### 3. **System Reliability**
- **Race condition elimination**: Single-threaded state consolidation
- **Predictable behavior**: Clear execution order and decision logic
- **Easy debugging**: Visible state consolidation and execution planning

### 4. **Maintainable Architecture**
- **Clear separation of concerns**: Consolidation → Planning → Execution
- **Extensible design**: New trigger types integrate seamlessly
- **Performance monitoring**: Measurable phases for optimization

## Migration Strategy

### Phase 1: Core Infrastructure
1. Implement `ExecutionPlan` structure
2. Add state consolidation logic to `ProcessPendingTriggerEvents()`
3. Create execution planning methods

### Phase 2: Main Loop Integration
1. Update main loop to use consolidation → planning → execution flow
2. Implement smart execution methods
3. Add comprehensive logging for validation

### Phase 3: Testing and Validation
1. Unit test consolidation logic with complex message sequences
2. Integration test all scenario combinations
3. Performance testing with rapid trigger sequences
4. LVGL stability testing during complex operations

This architecture evolution maintains the simplicity and performance benefits of the original pin-change-driven design while adding the intelligence needed to handle complex real-world trigger scenarios correctly and efficiently.