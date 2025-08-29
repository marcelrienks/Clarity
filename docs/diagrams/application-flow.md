# Application Flow Diagram

This diagram illustrates the complete application flow from startup through runtime operations, showing the coordinated interrupt processing flow.

## Flow Overview

- **Startup Sequence**: Dual factory pattern initialization, handler creation with sensor ownership, and initial panel display
- **Main Loop Integration**: LVGL tasks, interrupt processing, error handling, and panel management work together
- **Handler Ownership**: PolledHandler creates/owns GPIO sensors, QueuedHandler creates/owns button sensor during initialization
- **Interrupt Processing**: Continuous queued interrupt evaluation with idle-only polled interrupt processing and execution
- **Panel Management**: Self-sufficient panels create own components, Key/Lock panels are display-only (no sensors)
- **Evaluation Model**: Queued interrupts evaluated continuously, polled interrupts evaluated and actioned only during idle
- **Execution Model**: All interrupt execution happens only during UI idle state, polled processed before queued
- **Single Execution Function**: Each interrupt has one execution function (no separate activate/deactivate)
- **Button Actions**: Short press and long press execution through interrupt handlers
- **Style Management**: Theme setting and style retrieval for panels
- **Error Integration**: Separate error evaluation and panel display flow
- **Theme Switching**: Non-blocking theme changes through polled interrupts

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
        n11["Evaluate Queued<br>"]
        n14["if state == IDLE"]
        n15["Process Polled<br>"]
        n16["Process Queued<br>"]
  end
 subgraph s4["Error Management"]
        n12["Evaluate"]
        n13["Show Error panel"]
  end
 subgraph s5["QueuedHandler"]
        n18["Evaluate GPIO States"]
        n20["Action queued Interrupt"]
  end
 subgraph s6["PolledHandler"]
        n19["Evaluate GPIO States"]
        n23["Action changed state Interrupts"]
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
    n16 -- uses --> n20
    n19 --> n23
    n20 --> n22 & n21
    n23 -- uses --> n24 & PanelUpdates
    PanelUpdates -- uses --> n25
    n10 -- uses --> n25
    factory1 --> n2
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
    classDef startup fill:#fff2cc,stroke:#d6b656,stroke-width:2px
    classDef interrupt fill:#fff2cc,stroke:#d6b656,stroke-width:2px  
    classDef mainloop fill:#d5e8d4,stroke:#82b366,stroke-width:2px
    classDef panel fill:#f8cecc,stroke:#b85450,stroke-width:2px
    classDef button fill:#ffe6cc,stroke:#d79b00,stroke-width:2px
    classDef effects fill:#e1d5e7,stroke:#9673a6,stroke-width:2px
    classDef decision fill:#fff2cc,stroke:#d6b656,stroke-width:3px
```

## Key Flow Details

### Startup Sequence
1. **Factory Creation**: 
   - ProviderFactory (implements IProviderFactory) creates hardware providers
   - ManagerFactory receives IProviderFactory for dependency injection
2. **Service Initialization**: ManagerFactory creates all managers (Interrupt, Panel, Style, Preference, Error)
3. **Handler Creation with Sensor Ownership**:
   - InterruptManager creates PolledHandler → owns GPIO sensors (Key, Lock, Lights)
   - InterruptManager creates QueuedHandler → owns ActionButtonSensor
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
   - **Evaluate Queued Interrupts**: Continuously check button and queued events
   - **Check UI Idle State**: Determine if UI is idle for further processing
   - **If IDLE**: Process interrupts (polled before queued)
     - Evaluate and execute polled interrupts first (GPIO sensors → single execution function)
     - Then execute queued interrupts (button short/long press → single execution function)
     - Processing order ensures polled interrupts have priority over queued
3. **Process Errors**: Evaluate error conditions and show error panel if needed
4. **Process Panels**: Update current panel display and handle transitions

**Interrupt Processing Flow**:
- **Handler Ownership Model**:
  - PolledHandler owns all GPIO sensors (created during handler initialization)
  - QueuedHandler owns ActionButtonSensor (created during handler initialization)
- **Queued Interrupt Handler**: 
  - Evaluates queued interrupts continuously
  - Executes button presses via single execution function when idle
- **Polled Interrupt Handler**:
  - Evaluates GPIO-based interrupts only when idle
  - Executes via single execution function (no activate/deactivate split)
- **Processing Priority**: Polled interrupts execute before queued when both pending

**Key Architecture Benefits**:
- **Continuous Evaluation**: Queued interrupts always evaluated regardless of UI state
- **Idle-Only Execution**: Polled interrupts and all actions occur only during idle
- **Proper timing**: Accurate button press duration measurement
- **Clean separation**: Evaluation vs execution phases clearly defined

### Interrupt Processing Steps
1. **Evaluate Queued Interrupts**: Always check for button state changes (every loop)
2. **Check Idle State**: Determine if UI is idle before processing
3. **If Idle - Process Polled First**: 
   - Evaluate GPIO-based interrupts via handler-owned sensors
   - Execute via single execution function (no separate activate/deactivate)
4. **If Idle - Then Process Queued**: 
   - Execute button events via single execution function
   - Polled interrupts have already been processed (priority order)
5. **Execute Actions**: Single execution function per interrupt triggers effects

### Panel and Style Integration
- **Panel Self-Sufficiency**: 
  - Panels create their own components internally during initialization
  - Data panels (Oil) create own data sensors
  - Display-only panels (Key/Lock) create no sensors
- **Panel Creation**: PanelManager creates and loads panels on demand
- **Style Retrieval**: Panels get styles from StyleManager during creation and updates
- **Theme Changes**: StyleManager handles theme switching through SET_THEME effects
- **Button Actions**: Short and long press execute via single function pointer
- **Panel Updates**: Regular panel updates use current styles from StyleManager

### Error System
- **Error Evaluation**: Separate error evaluation in main loop
- **Error Panel Display**: Dedicated error panel shown when errors detected
- **Error Integration**: Process Errors step evaluates and shows error panel as needed

### Key System Components
- **Dual Factory Pattern**: 
  - ProviderFactory implements IProviderFactory for hardware abstraction
  - ManagerFactory uses dependency injection for testability
- **Interrupt Handlers with Ownership**: 
  - PolledHandler owns GPIO sensors (created during initialization)
  - QueuedHandler owns button sensor (created during initialization)
- **Panel Management**: Centralized panel creation, loading, and updates
- **Self-Sufficient Panels**: Create own components, Key/Lock are display-only
- **Style Management**: Theme setting and style retrieval for consistent UI
- **Single Execution Model**: One execution function per interrupt (simplified design)

### Performance Features
- **Processing Order**: Polled interrupts execute before queued when both pending
- **Idle-Based Processing**: Polled interrupts and all execution only during UI idle
- **Continuous Evaluation**: Queued interrupts always evaluated for responsiveness
- **Efficient Flow**: Main loop integrates LVGL, interrupts, errors, and panels
- **Clean Architecture**: Clear separation between evaluation and execution phases
- **Memory Optimized**: Single execution function reduces memory overhead (ESP32 320KB RAM constraint)

For complete architecture details, see: **[Architecture Document](../architecture.md)**