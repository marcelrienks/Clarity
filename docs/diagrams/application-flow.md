# Application Flow Diagram

This diagram illustrates the complete application flow from startup through runtime operations, showing how the coordinated interrupt system processes interrupts through specialized PolledHandler and QueuedHandler with source-based evaluation and effect-based execution.

## Flow Overview

- **Startup Sequence**: Manager creation, coordinated handlers creation with specialized sensor ownership
- **Main Loop**: LVGL tasks with coordinated interrupt processing during idle time only
- **Coordinated Processing**: InterruptManager coordinates PolledHandler and QueuedHandler by priority
- **Effect-Based Restoration**: Only LOAD_PANEL effects participate in simplified restoration logic
- **Source-Based Evaluation**: POLLED (GPIO state) and QUEUED (button events) processing
- **Error Integration**: ErrorManager with CRITICAL priority POLLED interrupt integration
- **Configuration Flow**: Preference setting via SET_PREFERENCE effect interrupts
- **Theme Switching**: Light sensor triggers SET_THEME effect interrupts (non-blocking)

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
1. **Service Initialization**: Initialize core services and prepare system
2. **Provider Creation**: ProviderFactory creates all hardware providers (GpioProvider, DisplayProvider, DeviceProvider)
3. **Manager Creation**: ManagerFactory creates all managers with IProviderFactory dependency injected
4. **Handler Creation**: InterruptManager creates PolledHandler and QueuedHandler
5. **Sensor Creation**: PolledHandler owns GPIO sensors, QueuedHandler owns button sensor
6. **Interrupt Registration**: Static callbacks registered to appropriate handlers (POLLED to PolledHandler, QUEUED to QueuedHandler)
7. **Panel System**: PanelManager ready to create panels on demand with provider injection
8. **Initial Display**: Splash panel loads with animation, creates own components

### Runtime Interrupt Processing
1. **LVGL Idle Check**: Interrupts only processed when system idle
2. **Coordinated Processing**: InterruptManager coordinates PolledHandler::Process() and QueuedHandler::Process()
3. **Priority-Based Coordination**: Highest priority interrupt across both handlers processed first
4. **Panel Updates**: All panel operations return to main loop

### Coordinated Interrupt Processing Steps
1. **State Change Processing**: InterruptManager processes all interrupts for state changes via evaluationFunc(sensorContext)
2. **Activate/Deactivate Execution**: Execute activateFunc(serviceContext) or deactivateFunc(serviceContext) based on state transitions
3. **Priority-Based Coordination**: Find highest priority non-maintaining interrupt across all handlers
4. **Maintenance-Based Restoration**: Only non-maintaining interrupts participate in restoration blocking logic

### Priority-Based Maintenance System
- **LOAD_PANEL Effects**: Usually don't maintain (maintainWhenInactive = false), participate in restoration blocking
- **SET_THEME Effects**: Always maintain (maintainWhenInactive = true), never block restoration
- **SET_PREFERENCE Effects**: Never maintain, never block restoration (immediate execution)
- **BUTTON_ACTION Effects**: Never maintain, execute panel's injected short/long press functions
- **Restoration Decision**: Only when NO non-maintaining interrupts active
- **Universal Panel Functions**: Every panel provides short/long press function pointers that get injected into button interrupts

### Error System Integration
- **Error Reporting**: Components report errors to ErrorManager singleton
- **POLLED Integration**: Error interrupt with CRITICAL priority and POLLED source
- **Panel Override**: Error panel LOAD_PANEL effect takes precedence over all other panels
- **User Interaction**: Error acknowledgment clears queue and triggers restoration logic

### Memory Safety Patterns
- **Static Callbacks**: All interrupt callbacks use function pointers with void* context
- **Union-Based Data**: Effect-specific data stored in memory-efficient union structures
- **Specialized Ownership**: GPIO sensors owned by PolledHandler, button sensor owned by QueuedHandler
- **Change Detection**: BaseSensor prevents state corruption through templates

### Performance Optimizations
- **State Change Based Processing**: Interrupts only execute activate/deactivate functions when state actually changes
- **Priority-Based Coordination**: Highest priority non-maintaining interrupt processed, maintaining ones run transparently
- **Custom Function Injection**: Panel-specific functions injected via void* pointers for memory efficiency
- **Maintenance-Based Restoration**: Clear maintenance rules eliminate complex restoration decision trees
- **Universal Button System**: Single button interrupt system works with all panels via injected functions
- **Idle Processing**: No interrupt overhead during LVGL operations