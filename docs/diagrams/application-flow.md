# Application Flow Diagram

This diagram illustrates the complete application flow from startup through runtime operations, showing the coordinated interrupt processing flow.

## Flow Overview

- **Startup Sequence**: Service initialization and coordinated handler creation
- **Main Loop**: LVGL tasks with separate interrupt evaluation and execution phases
- **Evaluation Model**: Queued interrupts evaluated every loop, polled interrupts evaluated only during idle
- **Execution Model**: All interrupt execution happens only during UI idle state
- **Coordinated Processing**: Priority-based interrupt coordination with centralized restoration
- **Hybrid Execution**: Single execution function per interrupt with centralized restoration logic
- **Panel Operations**: Lifecycle management and centralized restoration
- **Error Integration**: Critical priority error handling
- **Theme Switching**: Non-blocking theme changes

For detailed architecture, see: **[Architecture Document](../architecture.md)**

```mermaid
flowchart TD
    %% Startup Flow
    Start[ESP32 Startup]
    Setup[setup]
    InitServices[Initialize Services]
    CreateProviders[Create Providers via ProviderFactory]
    CreateManagers[Create Managers via ManagerFactory with IProviderFactory injected]
    CreateHandlers[InterruptManager Creates PolledHandler and QueuedHandler]
    CreateSensors[Handlers Create and Own Specialized Sensors]
    RegisterInterrupts[Register Interrupts - POLLED to PolledHandler, QUEUED to QueuedHandler]
    InitPanels[Initialize Panel System]
    LoadSplash[Load Splash Panel]
    StartLoop[Start Main Loop]
    
    %% Main Loop Flow
    LoopStart[Main Loop Start]
    LVGLTasks[Main: LVGL Tasks]
    EvaluateQueued[InterruptManager: Evaluate Queued]
    PostQueued[InterruptManager: Post Queued]
    CheckIdle{InterruptManager: If Idle?}
    EvaluateActionPolled[InterruptManager: Evaluate and Action Polled]
    CheckQueueAction{InterruptManager: If Queue Action?}
    ActionQueued[InterruptManager: Action Queued]
    LoopEnd[Main: Loop End]
    
    %% Hybrid Interrupt Flow with Centralized Restoration
    ProcessAllInterrupts[Evaluate All Interrupts for State Changes]
    CheckStateChanges[Check Each Interrupt - evaluationFunc with context]
    StateChanged{State Changed?}
    ExecuteInterrupt[executionFunc with context - Single Execution]
    UpdateInterruptState[Update active flags]
    FindHighestPriorityAcrossHandlers[InterruptManager: Find Highest Priority Active Interrupt Across Both Handlers]
    CheckInterruptDeactivated{Panel-Loading Interrupt Deactivated?}
    CentralizedRestoration[InterruptManager HandleRestoration]
    CheckActivePanelInterrupts{Any Active Panel-Loading Interrupts?}
    ExecuteHighestPriority[Execute Highest Priority Panel Interrupt]
    LoadPanelEffect[LOAD_PANEL Effect]
    SetThemeEffect[SET_THEME Effect - Always Maintains]
    SetPrefEffect[SET_PREFERENCE Effect]
    ButtonActionEffect[BUTTON_ACTION Effect]
    ExecutePanelFunction[Execute Panel Short/Long Press Function]
    RestorePanel[Restore User Panel]
    
    %% Panel Operations
    PanelInit[Panel Init]
    CreateComponents[Panel Creates Components and Sensors]
    PanelLoad[Panel Load]
    PanelUpdate[Panel Update]
    
    %% Configuration Flow
    ConfigShortPress[Short Press - Cycle Options]
    ConfigLongPress[Long Press - Save and Apply]
    SavePreferences[Save to PreferenceManager]
    
    %% Error Flow
    ErrorOccurs[Error Occurs]
    ErrorManager[ErrorManager ReportError]
    ErrorTrigger[Error Trigger Evaluates]
    LoadErrorPanel[Load Error Panel]
    ErrorAcknowledge[User Acknowledges Errors]
    ClearErrors[Clear Error Queue]
    
    %% Theme Flow
    LightChange[Light Sensor Change]
    ThemeSwitch[StyleManager SetTheme]
    ApplyTheme[Apply to All Panels]
    
    %% Connections - Startup
    Start --> Setup
    Setup --> InitServices
    InitServices --> CreateProviders
    CreateProviders --> CreateManagers
    CreateManagers --> CreateHandlers
    CreateHandlers --> CreateSensors
    CreateSensors --> RegisterInterrupts
    RegisterInterrupts --> InitPanels
    InitPanels --> LoadSplash
    LoadSplash --> StartLoop
    
    %% Connections - Main Loop
    StartLoop --> LoopStart
    LoopStart --> LVGLTasks
    LVGLTasks --> EvaluateQueued
    EvaluateQueued --> PostQueued
    PostQueued --> CheckIdle
    CheckIdle -->|Yes| EvaluateActionPolled
    EvaluateActionPolled --> CheckQueueAction
    CheckQueueAction -->|Yes| ActionQueued
    CheckQueueAction -->|No| LoopEnd
    ActionQueued --> LoopEnd
    CheckIdle -->|No| LoopEnd
    LoopEnd --> LoopStart
    CoordinateProcessing --> ProcessPolled
    CoordinateProcessing --> ProcessQueued
    ProcessPolled --> ProcessAllInterrupts
    ProcessQueued --> ProcessAllInterrupts
    ProcessAllInterrupts --> CheckStateChanges
    CheckStateChanges --> StateChanged
    StateChanged -->|Yes| ExecuteInterrupt
    StateChanged -->|No| FindHighestPriorityAcrossHandlers
    ExecuteInterrupt --> UpdateInterruptState
    UpdateInterruptState --> CheckInterruptDeactivated
    CheckInterruptDeactivated -->|Panel Interrupt Deactivated| CentralizedRestoration
    CheckInterruptDeactivated -->|No Deactivation| FindHighestPriorityAcrossHandlers
    FindHighestPriorityAcrossHandlers --> LoadPanelEffect
    FindHighestPriorityAcrossHandlers --> SetThemeEffect
    FindHighestPriorityAcrossHandlers --> SetPrefEffect
    FindHighestPriorityAcrossHandlers --> ButtonActionEffect
    LoadPanelEffect --> Loop
    SetThemeEffect --> Loop
    SetPrefEffect --> SavePreferences
    ButtonActionEffect --> ExecutePanelFunction
    ExecutePanelFunction --> Loop
    CentralizedRestoration --> CheckActivePanelInterrupts
    CheckActivePanelInterrupts -->|Yes| ExecuteHighestPriority
    CheckActivePanelInterrupts -->|No| RestorePanel
    ExecuteHighestPriority --> Loop
    RestorePanel --> Loop
    
    %% Connections - Panel Operations
    LoadPanelEffect --> PanelInit
    PanelInit --> CreateComponents
    CreateComponents --> PanelLoad
    PanelLoad --> PanelUpdate
    PanelUpdate --> Loop
    
    %% Connections - Configuration
    SetPrefEffect --> SavePreferences
    SavePreferences --> Loop
    
    %% Connections - Error Flow
    ErrorOccurs --> ErrorManager
    ErrorManager -.->|POLLED by| ProcessPolled
    ErrorAcknowledge --> ClearErrors
    ClearErrors --> CentralizedRestoration
    
    %% Connections - Theme Flow
    LightChange -.->|POLLED by| ProcessPolled
    SetThemeEffect --> ThemeSwitch
    ThemeSwitch --> ApplyTheme
    ApplyTheme --> Loop
    
    %% Styling
    classDef startup fill:#e3f2fd,stroke:#1976d2,stroke-width:2px
    classDef mainloop fill:#f1f8e9,stroke:#689f38,stroke-width:2px
    classDef interrupt fill:#fff8e1,stroke:#f57c00,stroke-width:2px
    classDef unified fill:#fce4ec,stroke:#c2185b,stroke-width:2px
    classDef effect fill:#f3e5f5,stroke:#7b1fa2,stroke-width:2px
    classDef panel fill:#e8f5e8,stroke:#388e3c,stroke-width:2px
    classDef config fill:#e0f2f1,stroke:#00796b,stroke-width:2px
    classDef error fill:#ffebee,stroke:#d32f2f,stroke-width:2px
    classDef theme fill:#f9fbe7,stroke:#827717,stroke-width:2px
    classDef decision fill:#fff3e0,stroke:#f57c00,stroke-width:3px
    
    class Start,Setup,InitServices,CreateProviders,CreateManagers,CreateHandlers,CreateSensors,RegisterInterrupts,InitPanels,LoadSplash,StartLoop startup
    class LoopStart,LVGLTasks,EvaluateQueued,PostQueued,EvaluateActionPolled,ActionQueued,LoopEnd mainloop
    class CoordinateProcessing,ProcessPolled,ProcessQueued,ProcessAllInterrupts,CheckStateChanges,ExecuteInterrupt,UpdateInterruptState,FindHighestPriorityAcrossHandlers,CentralizedRestoration,ExecuteHighestPriority interrupt
    class LoadPanelEffect,SetThemeEffect,SetPrefEffect,ButtonActionEffect,ExecutePanelFunction effect
    class PanelInit,CreateComponents,PanelLoad,PanelUpdate panel
    class SavePreferences config
    class ErrorOccurs,ErrorManager,ErrorAcknowledge,ClearErrors error
    class LightChange,ThemeSwitch,ApplyTheme theme
    class CheckIdle,CheckQueueAction,StateChanged,CheckInterruptDeactivated,CheckActivePanelInterrupts decision
```

## Key Flow Details

### Startup Sequence
1. **Service Initialization**: Core services and system preparation
2. **Factory Creation**: ProviderFactory and ManagerFactory setup
3. **Handler Creation**: InterruptManager creates specialized handlers
4. **Sensor Creation**: Handlers create and own their sensors
5. **Interrupt Registration**: Static callbacks registered to handlers
6. **Panel System**: PanelManager ready for on-demand panel creation
7. **Initial Display**: Splash panel loads with animation

### Runtime Processing
**Exact Main Loop Flow Sequence**:

1. **Main Loop Start**: Begin new loop iteration
2. **Main: LVGL Tasks**: Process UI updates and rendering
3. **InterruptManager: Evaluate Queued**: Always check button state changes 
4. **InterruptManager: Post Queued**: Queue button events if state changed
5. **InterruptManager: If Idle**: Check if UI is in idle state
   - **If UI NOT Idle**: Skip to step 8 (Loop End)
   - **If UI IS Idle**: Continue to step 6
6. **InterruptManager: Evaluate and Action Polled**: Check GPIO sensors and execute polled interrupts
7. **InterruptManager: If Queue Action**: Check if queued interrupt needs execution
   - **If Queue Action Needed**: Execute queued interrupt
   - **If No Queue Action**: Skip queued execution  
8. **Main: Loop End**: Complete loop iteration, return to step 1

### Interrupt Processing Steps
1. **State Change Detection**: Evaluation functions check current states
2. **Single Execution**: Execute function called once per state transition
3. **Priority Coordination**: Find highest priority active interrupt
4. **Centralized Restoration**: InterruptManager::HandleRestoration() manages all restoration decisions

### Centralized Restoration System
- **LOAD_PANEL Effects**: Participate in centralized restoration logic
- **SET_THEME Effects**: Never affect restoration decisions
- **SET_PREFERENCE Effects**: Immediate execution, no restoration impact
- **BUTTON_ACTION Effects**: Execute panel functions, no restoration impact
- **Restoration Decision**: InterruptManager queries all active panel-loading interrupts centrally

### Error System
- **Error Reporting**: Components report to ErrorManager
- **Critical Priority**: Error interrupts override other panels
- **User Interaction**: Error acknowledgment enables restoration

### Memory Safety
- **Static Callbacks**: Function pointers with void* context
- **Union-Based Data**: Memory-efficient effect data storage
- **Specialized Ownership**: Clear sensor ownership model
- **Change Detection**: BaseSensor template prevents corruption

### Performance Features
- **Change-Based Processing**: Functions execute only on state transitions
- **Priority Coordination**: Efficient highest-priority processing
- **Function Injection**: Memory-efficient panel function calls
- **Centralized Restoration**: Single restoration function eliminates distributed complexity
- **Memory Optimization**: Single execution function saves 28 bytes total
- **Idle Processing**: No overhead during LVGL operations

For complete architecture details, see: **[Architecture Document](../architecture.md)**