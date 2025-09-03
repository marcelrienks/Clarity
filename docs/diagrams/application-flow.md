# Application Flow Diagram

This diagram illustrates the complete application flow from startup through runtime operations.

## Flow Overview

- **Startup Sequence**: Multi-factory initialization with core dependency pattern (ProviderFactory → ManagerFactory), handler creation with sensor ownership, and initial panel display
- **Main Loop Integration**: LVGL tasks, interrupt processing, error handling, and panel management work together
- **Handler Ownership**: TriggerHandler creates/owns GPIO sensors, ActionHandler creates/owns button sensor during initialization
- **Trigger/Action Model**: State-based Triggers with dual functions, event-based Actions with single execution
- **Panel Management**: Self-sufficient panels create own components, Key/Lock panels are display-only (no sensors)
- **Evaluation Model**: Actions evaluated every main loop iteration for responsiveness, Triggers evaluated only during idle
- **Execution Model**: Both handlers execute only during UI idle state, Triggers processed before Actions
- **Dual-Function Triggers**: Each Trigger has activate/deactivate functions for state transitions
- **Button Actions**: Short press (50ms-2000ms) and long press (2000ms-5000ms) events through ActionHandler
- **Priority Override**: Triggers use CRITICAL > IMPORTANT > NORMAL priorities with blocking logic
- **Smart Restoration**: Automatic return to last user-driven panel when triggers deactivate
- **Theme Switching**: Non-blocking theme changes through LightsSensor trigger

For detailed architecture, see: **[Architecture Document](../architecture.md)**

```mermaid
---
config:
  layout: elk
  theme: default
  elk: {}
---
flowchart TB
 subgraph PanelManagement["Panel Management"]
        PanelUpdates["Create Panel<br>"]
        n10["Update Panel"]
        n21["Execute Short Press<br>"]
        n22["Execute Long Press<br>"]
  end
 subgraph s1["loop()"]
        n1["lvgl Tasks"]
        n8["Process Interrupts"]
        n9["Process Panels"]
        n17["Process Errors"]
  end
 subgraph s2["startup()"]
        factory1["Create Factories"]
        n2["Create Managers"]
        handlers["Create Handlers<br>"]
        n3["Initialize Styles"]
        n5["Show Initial Panel"]
  end
 subgraph s3["Interrupt Management"]
        n11["Check Actions<br>"]
        n14["Is state == IDLE"]
        n15["Process Triggers<br>"]
        n16["Execute Actions<br>"]
  end
 subgraph s4["Error Management"]
        n12["Evaluate Pending Errors"]
        n13["Show Error panel"]
  end
 subgraph s5["Action Handler"]
        n18["Evaluate Trigger Flags"]
        n20["Execute Action Function"]
        n26["Queue Trigger"]
        n27["Is Trigger Queued"]
  end
 subgraph s6["Trigger Handler"]
        n19["Evaluate Trigger States"]
        n23["Execute Trigger Functions"]
  end
 subgraph s7["Style Management"]
        n24["Set Theme"]
        n25["Get Styles"]
  end
    n2 --> handlers
    handlers --> n3
    n3 --> n5
    n5 -- creates panel --> PanelUpdates
    n9 -- uses --> n10
    n8 -- uses --> n11
    n5 --> n1
    n13 -- uses --> PanelUpdates
    n11 --> n14
    n14 -- true --> n15
    n15 -- <br> --> n16
    n15 -- uses --> n19
    n17 --> n9
    n17 -- uses --> n12
    n1 --> n8
    n8 --> n17
    n12 --> n13
    n11 -- uses --> n18
    n19 --> n23
    n20 --> n22 & n21
    n23 -- uses --> n24 & PanelUpdates
    PanelUpdates -- uses --> n25
    n10 -- uses --> n25
    factory1 --> n2
    n18 --> n26
    n16 -- uses --> n27
    n27 -- true --> n20
    n10@{ shape: rect}
    n21@{ shape: proc}
    n22@{ shape: proc}
    n8@{ shape: rect}
    n9@{ shape: rect}
    factory1@{ shape: rect}
    n2@{ shape: rect}
    handlers@{ shape: rect}
    n3@{ shape: proc}
    n5@{ shape: proc}
    n14@{ shape: diam}
    n12@{ shape: rect}
    n20@{ shape: rect}
    n27@{ shape: diam}
    classDef startup fill:#fff2cc,stroke:#d6b656,stroke-width:2px
    classDef interrupt fill:#fff2cc,stroke:#d6b656,stroke-width:2px  
    classDef mainloop fill:#d5e8d4,stroke:#82b366,stroke-width:2px
    classDef panel fill:#f8cecc,stroke:#b85450,stroke-width:2px
    classDef button fill:#ffe6cc,stroke:#d79b00,stroke-width:2px
    classDef effects fill:#e1d5e7,stroke:#9673a6,stroke-width:2px
    classDef decision fill:#fff2cc,stroke:#d6b656,stroke-width:3px
    style s3 fill:#BBDEFB
    style s4 fill:#BBDEFB
    style PanelManagement fill:#BBDEFB
    style s5 fill:#C8E6C9
    style s6 fill:#C8E6C9
    style s7 fill:#BBDEFB
```

## Key Flow Details

### Startup Sequence
1. **Core Factory Dependency Pattern Implementation**: 
   - ProviderFactory created first (implements IProviderFactory for hardware abstraction)
   - ProviderFactory creates hardware providers (Device, GPIO, Display)
   - ManagerFactory created with ProviderFactory dependency injection (Provider → Manager pattern)
2. **Service Initialization Order**: 
   - ManagerFactory uses injected ProviderFactory to obtain providers
   - Creates PreferenceManager (for user settings)
   - Creates StyleManager (with theme from preferences)  
   - Creates InterruptManager (with GPIO provider)
   - Creates PanelManager (with all dependencies)
   - Creates ErrorManager (singleton pattern)
3. **Handler Creation with Sensor Ownership**:
   - InterruptManager creates TriggerHandler → owns GPIO sensors (Key, Lock, Lights)
   - InterruptManager creates ActionHandler → owns ButtonSensor
   - Handlers register their Triggers and Actions with InterruptManager
4. **Initialize Styles**: Setup visual styles and themes for the UI
5. **Show Initial Panel**: 
   - PanelManager creates initial panel
   - Panel creates own components internally (self-sufficient design)
   - Key/Lock panels are display-only (no sensor creation)
6. **Enter Main Loop**: Begin LVGL tasks and runtime processing

### Runtime Processing
**Main Loop Flow (loop())**:

1. **LVGL Tasks**: Process LVGL rendering and animation tasks
2. **Process Interrupts**: Handle interrupt evaluation and execution
   - **Evaluate Actions**: Continuously check button events for responsiveness
   - **Check UI Idle State**: Determine if UI is idle for further processing
   - **If IDLE**: Process interrupts (Triggers before Actions)
     - Evaluate and execute Triggers with priority override logic
     - Then execute Actions (button short/long press events)
     - Processing order ensures state changes have priority over events
3. **Process Errors**: Evaluate error conditions and show error panel if needed
4. **Process Panels**: Update current panel display and handle transitions

**Interrupt Processing Flow**:
- **Handler Ownership Model**:
  - TriggerHandler owns all GPIO sensors (created during handler initialization)
  - ActionHandler owns ButtonSensor (created during handler initialization)
- **Action Handler (Events)**: 
  - Evaluates button events every main loop iteration for responsiveness
  - Detects short (50ms-2000ms) and long (2000ms-5000ms) press durations
  - Executes button actions only when UI is idle
- **Trigger Handler (States)**:
  - Evaluates GPIO state changes only when idle
  - Executes dual functions: activate on HIGH, deactivate on LOW
  - Implements priority-based override logic (CRITICAL > IMPORTANT > NORMAL)
- **Processing Priority**: Triggers execute before Actions when both pending

**Key Architecture Benefits**:
- **Trigger/Action Separation**: Clear distinction between state-based and event-based interrupts
- **Continuous Responsiveness**: Actions always evaluated for button event detection
- **Idle-Only Execution**: Both handlers execute only during idle for UI performance
- **Priority Override System**: Sophisticated blocking logic for critical triggers
- **Smart Restoration**: Automatic return to last user-driven panel

### Interrupt Processing Steps
1. **Evaluate Actions**: Always check for button events (every loop)
2. **Check Idle State**: Determine if UI is idle before processing
3. **If Idle - Process Triggers First**: 
   - Evaluate GPIO state changes via TriggerHandler sensors
   - Execute activate/deactivate functions based on state
   - Apply priority-based override logic
4. **If Idle - Then Execute Actions**: 
   - Execute pending button actions (short/long press)
   - Triggers have already been processed (priority order)
5. **Smart Restoration**: PanelManager tracks and restores last user panel when appropriate

### Panel and Style Integration
- **Panel Self-Sufficiency**: 
  - Panels create their own components internally during initialization
  - Data panels (Oil) create own data sensors
  - Display-only panels (Key/Lock) create no sensors
- **Panel Creation**: PanelManager creates and loads panels via Trigger activation functions
- **Style Retrieval**: Panels get styles from StyleManager during creation and updates
- **Theme Changes**: LightsSensor trigger handles theme switching (NIGHT on activate, DAY on deactivate)
- **Button Actions**: Short and long press events execute through ActionHandler
- **Panel Restoration**: Smart restoration to last user-driven panel when triggers deactivate

### Error System
- **Error Evaluation**: Separate error evaluation in main loop
- **Error Trigger**: Error sensor activates error trigger with CRITICAL priority
- **Error Panel Display**: Error trigger activation function loads error panel
- **Error Recovery**: Deactivation function handles restoration to previous panel

### Key System Components
- **Multi-Factory Architecture**: 
  - **Core Dependency Pattern**: ProviderFactory → ManagerFactory chain
    - ProviderFactory implements IProviderFactory for hardware abstraction
    - Creates DeviceProvider, GpioProvider, DisplayProvider
    - ManagerFactory accepts IProviderFactory for dependency injection
    - Uses injected factory to obtain providers for manager creation
  - **UI Singleton Factories**: PanelFactory and ComponentFactory operate independently
    - PanelFactory creates panels on demand from managers
    - ComponentFactory creates UI components from panels
  - **Complete Testability**: Mock provider factory injection enables full testing
- **Handlers with Ownership**: 
  - TriggerHandler owns GPIO sensors (created during initialization)
  - ActionHandler owns ButtonSensor (created during initialization)
- **Trigger/Action Architecture**:
  - Triggers: State-based with dual activate/deactivate functions
  - Actions: Event-based with single execution function
  - Priority system: CRITICAL > IMPORTANT > NORMAL (Triggers only)
- **Panel Management**: Centralized panel creation, loading, and smart restoration
- **Self-Sufficient Panels**: Create own components, Key/Lock are display-only
- **Style Management**: Theme setting via LightsSensor trigger

### Performance Features
- **Processing Order**: Triggers execute before Actions when both pending
- **Idle-Based Execution**: Both handlers execute only during UI idle
- **Continuous Evaluation**: Actions always evaluated for button responsiveness
- **Priority Override**: Sophisticated blocking logic prevents unnecessary panel switches
- **Efficient Flow**: Main loop integrates LVGL, interrupts, errors, and panels
- **Clean Architecture**: Clear Trigger/Action separation with appropriate complexity
- **Memory Optimized**: Direct singleton calls eliminate context pointers for ESP32 efficiency

For complete architecture details, see: **[Architecture Document](../architecture.md)**