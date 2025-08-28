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
    CreateManagers[Create Managers via ManagerFactory]
    CreateHandlers[InterruptManager Creates PolledHandler and QueuedHandler]
    CreateSensors[Handlers Create and Own Specialized Sensors]
    RegisterInterrupts[Register Interrupts]
    InitPanels[Initialize Panel System]
    LoadSplash[Load Splash Panel]
    StartLoop[Start Main Loop]
    
    %% Main Loop - 8 Step Flow
    Step1[Step 1: Main Loop Start]
    Step2[Step 2: Main LVGL Tasks]
    Step3[Step 3: InterruptManager Evaluate Queued - ALWAYS]
    Step4[Step 4: InterruptManager Post Queued - ALWAYS]
    Step5{Step 5: InterruptManager If Idle?}
    Step6[Step 6: InterruptManager Evaluate and Action Polled - IDLE ONLY]
    Step7{Step 7: InterruptManager If Queue Action? - IDLE ONLY}
    Step8[Step 8: Main Loop End]
    
    %% Queue Action Execution
    ExecuteQueuedAction[Execute Queued Action]
    
    %% Button Processing Detail
    ButtonPressStart[Button Press Start]
    ButtonRelease[Button Release]
    CalcDuration[Calculate Press Duration]
    ShortPress[Short Press: 50ms-2000ms]
    LongPress[Long Press: 2000ms-5000ms]
    FlagForProcessing[Flag for Processing]
    
    %% Panel Effects
    LoadPanel[Load Panel - Polled Interrupts]
    SetTheme[Set Theme - Polled Interrupts]
    ExecuteShortPress[Execute Short Press Function]
    ExecuteLongPress[Execute Long Press Function]
    
    %% Startup Connections
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
    
    %% Main Loop Connections - 8 Step Flow
    StartLoop --> Step1
    Step1 --> Step2
    Step2 --> Step3
    Step3 --> Step4
    Step4 --> Step5
    Step5 -->|UI Idle| Step6
    Step5 -->|UI Busy| Step8
    Step6 --> Step7
    Step7 -->|Queue Action Needed| ExecuteQueuedAction
    Step7 -->|No Queue Action| Step8
    ExecuteQueuedAction --> Step8
    Step8 --> Step1
    
    %% Button Processing Connections - Step 3 Always Runs
    Step3 --> ButtonPressStart
    Step3 --> ButtonRelease
    ButtonRelease --> CalcDuration
    CalcDuration --> ShortPress
    CalcDuration --> LongPress
    ShortPress --> FlagForProcessing
    LongPress --> FlagForProcessing
    FlagForProcessing --> Step4
    
    %% Queue Action Execution - Step 7 Conditional
    ExecuteQueuedAction --> ExecuteShortPress
    ExecuteQueuedAction --> ExecuteLongPress
    
    %% Polled Effect Connections - Step 6 Idle Only
    Step6 --> LoadPanel
    Step6 --> SetTheme
    
    %% Styling
    classDef startup fill:#e3f2fd,stroke:#1976d2,stroke-width:2px
    classDef mainloop fill:#f1f8e9,stroke:#689f38,stroke-width:2px
    classDef button fill:#fff3e0,stroke:#f57c00,stroke-width:2px
    classDef effect fill:#f3e5f5,stroke:#7b1fa2,stroke-width:2px
    classDef decision fill:#fff3e0,stroke:#f57c00,stroke-width:3px
    
    class Start,Setup,InitServices,CreateProviders,CreateManagers,CreateHandlers,CreateSensors,RegisterInterrupts,InitPanels,LoadSplash,StartLoop startup
    class Step1,Step2,Step3,Step4,Step6,Step8 mainloop
    class ButtonPressStart,ButtonRelease,CalcDuration,ShortPress,LongPress,FlagForProcessing button
    class ExecuteQueuedAction,ExecuteShortPress,ExecuteLongPress button
    class LoadPanel,SetTheme effect
    class Step5,Step7 decision
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
**Exact 8-Step Main Loop Flow**:

1. **Main Loop Start**: Begin new iteration
2. **Main: LVGL Tasks**: Process UI updates and rendering
3. **InterruptManager: Evaluate Queued** (ALWAYS): 
   - Detect button press start/release events
   - Calculate press duration on release
   - Determine short (50ms-2000ms) vs long press (2000ms-5000ms)
4. **InterruptManager: Post Queued** (ALWAYS):
   - Flag button events for processing during idle time
5. **InterruptManager: If Idle**: Check UI state
   - **If UI NOT Idle**: Skip to step 8 (ensures button detection continues)
   - **If UI IS Idle**: Continue to step 6
6. **InterruptManager: Evaluate and Action Polled** (IDLE ONLY):
   - Check GPIO sensors (key, lock, lights, errors)
   - Execute polled interrupts (load panels, set themes)
7. **InterruptManager: If Queue Action** (IDLE ONLY):
   - **If Queue Action Needed**: Execute appropriate button function (short/long press)
   - **If No Queue Action**: Skip to step 8
8. **Main: Loop End**: Complete iteration, return to step 1

**Key Architecture Benefits**:
- **Steps 3-4 ALWAYS execute**: Button presses never missed regardless of UI state
- **Steps 6-7 IDLE ONLY**: Protects UI performance during animations/rendering
- **Proper timing**: Accurate button press duration measurement
- **Clean separation**: Evaluation (always) vs execution (idle only)

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