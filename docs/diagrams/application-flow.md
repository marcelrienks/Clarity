# Application Flow Diagram

This diagram illustrates the complete application flow from startup through runtime operations, showing the coordinated interrupt processing flow.

## Flow Overview

- **Startup Sequence**: Service initialization and coordinated handler creation
- **Main Loop**: LVGL tasks with interrupt processing during idle time only
- **Coordinated Processing**: Priority-based interrupt coordination
- **Effect-Based Execution**: LOAD_PANEL, SET_THEME, SET_PREFERENCE, BUTTON_ACTION effects
- **Panel Operations**: Lifecycle management and restoration
- **Error Integration**: Critical priority error handling
- **Theme Switching**: Non-blocking theme changes

For detailed architecture, see: **[Architecture Document](../architecture.md)**

```mermaid
flowchart TD
    %% Startup Flow
    Start([ESP32 Startup])
    Setup[setup()]
    InitServices[Initialize Services]
    CreateProviders[Create Providers via ProviderFactory]
    CreateManagers[Create Managers via ManagerFactory<br/>with IProviderFactory injected]
    CreateHandlers[InterruptManager Creates PolledHandler & QueuedHandler]
    CreateSensors[Handlers Create & Own Specialized Sensors]
    RegisterInterrupts[Register Interrupts (POLLED to PolledHandler, QUEUED to QueuedHandler)]
    InitPanels[Initialize Panel System]
    LoadSplash[Load Splash Panel]
    StartLoop[Start Main Loop]
    
    %% Main Loop Flow
    Loop[loop()]
    LVGLTasks[lv_task_handler()]
    CheckIdle{System Idle?}
    ProcessInterrupts[InterruptManager::Process()]
    
    %% Coordinated Interrupt Processing
    CoordinateProcessing[InterruptManager Coordinates Both Handlers]
    ProcessPolled[PolledHandler::Process()]
    ProcessQueued[QueuedHandler::Process()]
    
    %% Coordinated Interrupt Flow
    ProcessAllInterrupts[Process All Interrupts for State Changes]
    CheckStateChanges[Check Each Interrupt: evaluationFunc(sensorContext)]
    StateChanged{State Changed?}
    CheckNewState{New State Active?}
    ExecuteActivate[activateFunc(serviceContext)]
    ExecuteDeactivate[deactivateFunc(serviceContext)]
    UpdateInterruptState[Update active/previouslyActive flags]
    FindHighestPriority[Find Highest Priority Non-Maintaining Interrupt]
    CheckMaintenance{Interrupt Maintains<br/>When Inactive?}
    CheckRestoration{Any Non-Maintaining<br/>Interrupts Active?}
    LoadPanelEffect[LOAD_PANEL Effect]
    SetThemeEffect[SET_THEME Effect (Always Maintains)]
    SetPrefEffect[SET_PREFERENCE Effect]
    ButtonActionEffect[BUTTON_ACTION Effect]
    ExecutePanelFunction[Execute Panel's Short/Long Press Function]
    RestorePanel[Restore User Panel]
    
    %% Panel Operations
    PanelInit[Panel::Init()]
    CreateComponents[Panel Creates<br/>Components & Sensors]
    PanelLoad[Panel::Load()]
    PanelUpdate[Panel::Update()]
    
    %% Configuration Flow
    ConfigShortPress[Short Press:<br/>Cycle Options]
    ConfigLongPress[Long Press:<br/>Save & Apply]
    SavePreferences[Save to PreferenceManager]
    
    %% Error Flow
    ErrorOccurs[Error Occurs]
    ErrorManager[ErrorManager::ReportError()]
    ErrorTrigger[Error Trigger Evaluates]
    LoadErrorPanel[Load Error Panel]
    ErrorAcknowledge[User Acknowledges Errors]
    ClearErrors[Clear Error Queue]
    
    %% Theme Flow
    LightChange[Light Sensor Change]
    ThemeSwitch[StyleManager::SetTheme()]
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
    StartLoop --> Loop
    Loop --> LVGLTasks
    LVGLTasks --> CheckIdle
    CheckIdle -->|Yes| ProcessInterrupts
    CheckIdle -->|No| Loop
    ProcessInterrupts --> CoordinateProcessing
    CoordinateProcessing --> ProcessPolled
    CoordinateProcessing --> ProcessQueued
    ProcessPolled --> GetHighestPolled
    ProcessQueued --> GetHighestQueued
    CoordinateProcessing --> ProcessAllInterrupts
    ProcessAllInterrupts --> CheckStateChanges
    CheckStateChanges --> StateChanged
    StateChanged -->|Yes| CheckNewState
    StateChanged -->|No| FindHighestPriority
    CheckNewState -->|Active| ExecuteActivate
    CheckNewState -->|Inactive| ExecuteDeactivate
    ExecuteActivate --> UpdateInterruptState
    ExecuteDeactivate --> UpdateInterruptState
    UpdateInterruptState --> FindHighestPriority
    FindHighestPriority --> CheckMaintenance
    CheckMaintenance -->|Maintains| CheckRestoration
    CheckMaintenance -->|Doesn't Maintain| LoadPanelEffect
    CheckMaintenance -->|Doesn't Maintain| ButtonActionEffect
    LoadPanelEffect --> CheckRestoration
    ButtonActionEffect --> ExecutePanelFunction
    ExecutePanelFunction --> CheckRestoration
    SetThemeEffect --> CheckRestoration
    CheckRestoration -->|Yes| Loop
    CheckRestoration -->|No| RestorePanel
    RestorePanel --> Loop
    
    %% Connections - Panel Operations
    LoadPanelEffect --> PanelInit
    CustomEffect --> PanelInit
    PanelInit --> CreateComponents
    CreateComponents --> PanelLoad
    PanelLoad --> PanelUpdate
    PanelUpdate --> Loop
    
    %% Connections - Configuration
    SetPrefEffect --> SavePreferences
    SavePreferences --> Loop
    
    %% Connections - Error Flow
    ErrorOccurs --> ErrorManager
    ErrorManager -.->|POLLED by| POLLEDCheck
    ErrorAcknowledge --> ClearErrors
    ClearErrors --> CheckRestoration
    
    %% Connections - Theme Flow
    LightChange -.->|POLLED by| POLLEDCheck
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
    class Loop,LVGLTasks,ProcessInterrupts mainloop
    class CoordinateProcessing,ProcessPolled,ProcessQueued,ProcessAllInterrupts,CheckStateChanges,ExecuteActivate,ExecuteDeactivate,UpdateInterruptState,FindHighestPriority interrupt
    class LoadPanelEffect,SetThemeEffect,SetPrefEffect,ButtonActionEffect,ExecutePanelFunction effect
    class PanelInit,CreateComponents,PanelLoad,PanelUpdate panel
    class SavePreferences config
    class ErrorOccurs,ErrorManager,ErrorAcknowledge,ClearErrors error
    class LightChange,ThemeSwitch,ApplyTheme theme
    class CheckIdle,StateChanged,CheckNewState,CheckMaintenance,CheckRestoration decision
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
1. **LVGL Idle Check**: Interrupts processed only during idle time
2. **Coordinated Processing**: InterruptManager coordinates both handlers
3. **Priority-Based Coordination**: Highest priority interrupt processed first
4. **Panel Updates**: Operations return to main loop

### Interrupt Processing Steps
1. **State Change Detection**: Evaluation functions check current states
2. **Activate/Deactivate**: Execute functions based on state transitions
3. **Priority Coordination**: Find highest priority active interrupt
4. **Restoration Logic**: Non-maintaining interrupts block restoration

### Maintenance System
- **LOAD_PANEL Effects**: Usually don't maintain, participate in restoration blocking
- **SET_THEME Effects**: Always maintain, never block restoration
- **SET_PREFERENCE Effects**: Never maintain, immediate execution
- **BUTTON_ACTION Effects**: Execute panel functions, never maintain
- **Restoration Decision**: When no non-maintaining interrupts active

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
- **Simplified Restoration**: Clear maintenance-based rules
- **Idle Processing**: No overhead during LVGL operations

For complete architecture details, see: **[Architecture Document](../architecture.md)**