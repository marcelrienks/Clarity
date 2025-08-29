# Application Flow Diagram

This diagram illustrates the complete application flow from startup through runtime operations, showing the coordinated interrupt processing flow.

## Flow Overview

- **Startup Sequence**: Service initialization, styles setup, and initial panel display
- **Main Loop Integration**: LVGL tasks, interrupt processing, error handling, and panel management work together
- **Interrupt Processing**: Continuous queued interrupt evaluation with idle-only polled interrupt processing and execution
- **Panel Management**: Create/load panels, updates, and button action execution integrated with main loop
- **Evaluation Model**: Queued interrupts evaluated continuously, polled interrupts evaluated and actioned only during idle
- **Execution Model**: All interrupt execution happens only during UI idle state
- **Button Actions**: Short press and long press execution through interrupt handlers
- **Style Management**: Theme setting and style retrieval for panels
- **Error Integration**: Separate error evaluation and panel display flow
- **Theme Switching**: Non-blocking theme changes through polled interrupts

For detailed architecture, see: **[Architecture Document](../architecture.md)**

```mermaid
---
config:
  layout: fixed
  theme: default
  elk: {}
id: 063444bd-5464-486a-8d8d-72cb41573cb5
---
flowchart TB
 subgraph PanelManagement["Panel Management"]
        PanelUpdates["Create and Load Panel"]
        n10["Update Panel"]
        n21["Execute Short Press"]
        n22["Execute Long Press"]
  end
 subgraph s1["loop()"]
        n1["lvgl Tasks"]
        n8["Process Interrupts"]
        n9["Process Panels"]
        n17["Process Errors"]
  end
 subgraph s2["startup()"]
        n2@{ label: "<span style=\"color:\">initialize Services</span>" }
        n3["Initialize Styles"]
        n4["Initialize Styles"]
        n5["Show Panels"]
  end
 subgraph s3["Interrupt Management"]
        n11["Evaluate Queued Interrupts"]
        n14["if state == IDLE"]
        n15["Process Polled Interrupts"]
        n16["Process Queued Interrupts"]
  end
 subgraph s4["Error Management"]
        n12["Evaluate"]
        n13["Show Error panel"]
  end
 subgraph s5["Queued Interrupt Handler"]
        n18["Evaluate Queued Interrupts"]
        n20["Action Queued Interrupts"]
  end
 subgraph s6["Polled Interrupt Handler"]
        n19["Evaluate Polled Interrupts"]
        n23["Action Polled Interrupts"]
  end
 subgraph s7["Style Management"]
        n24["Set Theme"]
        n25["Get Styles"]
  end
    n2 --> n3
    n3 --> n4
    n4 --> n5
    n5 -- uses --> PanelUpdates
    n9 -- uses --> n10
    n8 -- uses --> n11
    n5 --> n1
    n13 -- uses --> PanelUpdates
    n11 --> n14
    n14 -- true --> n15
    n15 --> n16
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
    n10@{ shape: rect}
    n21@{ shape: proc}
    n22@{ shape: proc}
    n8@{ shape: rect}
    n9@{ shape: rect}
    n2@{ shape: rect}
    n3@{ shape: proc}
    n4@{ shape: proc}
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
1. **Service Initialization**: Core services and system preparation
2. **Factory Creation**: ProviderFactory and ManagerFactory setup with dependency injection
3. **Initialize Styles**: Setup visual styles and themes for the UI
4. **Handler Creation**: InterruptManager creates PolledHandler and QueuedHandler
5. **Sensor Creation**: Handlers create and own their respective sensors
6. **Show Panels**: PanelManager creates and loads initial panel display
7. **Enter Main Loop**: Begin LVGL tasks and runtime processing

### Runtime Processing
**Main Loop Flow (loop())**:

1. **LVGL Tasks**: Process LVGL rendering and animation tasks
2. **Process Interrupts**: Handle interrupt evaluation and execution
   - **Evaluate Queued Interrupts**: Continuously check button and queued events
   - **Check UI Idle State**: Determine if UI is idle for further processing
   - **If IDLE**: Process both polled and queued interrupts
     - Evaluate polled interrupts (GPIO sensors)
     - Action polled interrupts (theme changes, panel loads)
     - Action queued interrupts (button short/long press)
3. **Process Errors**: Evaluate error conditions and show error panel if needed
4. **Process Panels**: Update current panel display and handle transitions

**Interrupt Processing Flow**:
- **Queued Interrupt Handler**: 
  - Evaluates queued interrupts continuously
  - Actions button presses (short and long) when idle
- **Polled Interrupt Handler**:
  - Evaluates GPIO-based interrupts only when idle
  - Actions panel loads and theme changes

**Key Architecture Benefits**:
- **Continuous Evaluation**: Queued interrupts always evaluated regardless of UI state
- **Idle-Only Execution**: Polled interrupts and all actions occur only during idle
- **Proper timing**: Accurate button press duration measurement
- **Clean separation**: Evaluation vs execution phases clearly defined

### Interrupt Processing Steps
1. **Evaluate Interrupts**: Check for state changes in queued and polled interrupts
2. **Check Idle State**: Determine if UI is idle before processing
3. **Process Polled Interrupts**: Evaluate and action GPIO-based interrupts (idle only)
4. **Process Queued Interrupts**: Action button events and other queued interrupts (idle only)
5. **Execute Actions**: Trigger appropriate effects (panel loads, theme changes, button actions)

### Panel and Style Integration
- **Panel Creation**: PanelManager creates and loads panels on demand
- **Style Retrieval**: Panels get styles from StyleManager during creation and updates
- **Theme Changes**: StyleManager handles theme switching through SET_THEME effects
- **Button Actions**: Short and long press actions execute panel-specific functions
- **Panel Updates**: Regular panel updates use current styles from StyleManager

### Error System
- **Error Evaluation**: Separate error evaluation in main loop
- **Error Panel Display**: Dedicated error panel shown when errors detected
- **Error Integration**: Process Errors step evaluates and shows error panel as needed

### Key System Components
- **Interrupt Handlers**: Separate handlers for queued and polled interrupts
- **Panel Management**: Centralized panel creation, loading, and updates
- **Style Management**: Theme setting and style retrieval for consistent UI
- **Button Processing**: Short and long press detection with action execution

### Performance Features
- **Idle-Based Processing**: Polled interrupts and actions only during UI idle
- **Continuous Evaluation**: Queued interrupts always evaluated for responsiveness
- **Efficient Flow**: Main loop integrates LVGL, interrupts, errors, and panels
- **Clean Architecture**: Clear separation between evaluation and action phases

For complete architecture details, see: **[Architecture Document](../architecture.md)**