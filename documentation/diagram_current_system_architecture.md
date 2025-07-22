---
config:
  layout: dagre
---
flowchart TD
    subgraph "ESP32 Core 0 (APP_CPU)"
        PanelManager@{ label: "Panel Manager<br>Core 0 UI Management<br>- Panel lifecycle<br>- Queue processing<br>- State-based throttling<br>- Action execution" }
        Panel["Panel<br>(Presenter)"] 
        Component["Component<br>(View)"]
        LVGL["LVGL<br>UI Rendering"]
        UIState["UI State<br>IDLE/UPDATING/<br>LOADING/LVGL_BUSY"]
    end
    
    subgraph "ESP32 Core 1 (PRO_CPU)" 
        TriggerManager@{ label: "Trigger Manager<br>Core 1 Hardware Control<br>- GPIO interrupt handling<br>- State-aware decisions<br>- Message queue management<br>- Application state tracking" }
        GPIOInterrupts["GPIO Interrupts<br>- Key Present/Absent<br>- Lock State<br>- Theme Switch"]
        ISRHandlers["ISR Handlers<br>IRAM_ATTR functions"]
    end
    
    subgraph "Inter-Core Communication"
        HighPriorityQueue["High Priority Queue<br>Critical triggers<br>(Key presence, safety)"]
        MediumPriorityQueue["Medium Priority Queue<br>Important triggers<br>(Lock state, modes)"] 
        LowPriorityQueue["Low Priority Queue<br>Non-critical triggers<br>(Theme changes)"]
        StateMutex["State Mutex<br>Thread-safe<br>synchronization"]
    end
    
    subgraph "Shared State"
        AppState["Application State<br>- Current Panel<br>- Current Theme<br>- Pending Messages"]
    end
    
    subgraph "Hardware Layer"
        GPIO["GPIO Pins<br>- Pin 25: Key Present<br>- Pin 26: Key Not Present<br>- Pin 27: Lock State"]
        SensorModel["Sensor<br>(Model)<br>Data acquisition"]
    end

    %% Core 0 relationships
    PanelManager -- manages --> Panel
    PanelManager -- processes queues --> HighPriorityQueue
    PanelManager -- processes queues --> MediumPriorityQueue  
    PanelManager -- processes queues --> LowPriorityQueue
    PanelManager -- sets --> UIState
    PanelManager -- notifies --> StateMutex
    Panel -- contains --> Component
    Panel -- renders via --> LVGL
    Component -- displays data from --> SensorModel
    
    %% Core 1 relationships
    GPIO -- triggers --> GPIOInterrupts
    GPIOInterrupts -- calls --> ISRHandlers
    ISRHandlers -- invokes --> TriggerManager
    TriggerManager -- posts to --> HighPriorityQueue
    TriggerManager -- posts to --> MediumPriorityQueue
    TriggerManager -- posts to --> LowPriorityQueue
    TriggerManager -- tracks --> AppState
    TriggerManager -- synchronizes via --> StateMutex
    
    %% Inter-core communication
    StateMutex -- protects --> AppState
    TriggerManager -- reads/writes --> AppState
    PanelManager -- reads/writes --> AppState
    
    %% Message flow (Core 1 → Core 0)
    HighPriorityQueue -. "LoadPanel<br>RestorePanel" .-> PanelManager
    MediumPriorityQueue -. "LoadPanel<br>RestorePanel" .-> PanelManager
    LowPriorityQueue -. "ChangeTheme" .-> PanelManager
    
    %% State synchronization (Core 0 → Core 1)
    PanelManager -. "Panel/Theme<br>state updates" .-> TriggerManager
    
    %% Hardware data flow
    SensorModel -- reads --> GPIO
    
    %% Styling
    PanelManager:::core0
    Panel:::mvpPresenter
    Component:::mvpView
    LVGL:::core0
    UIState:::core0
    
    TriggerManager:::core1
    GPIOInterrupts:::core1
    ISRHandlers:::core1
    
    HighPriorityQueue:::communication
    MediumPriorityQueue:::communication
    LowPriorityQueue:::communication
    StateMutex:::communication
    
    AppState:::shared
    SensorModel:::mvpModel
    GPIO:::hardware
    
    classDef mvpModel fill:#e1f5fe
    classDef mvpView fill:#f3e5f5
    classDef mvpPresenter fill:#e8f5e8
    classDef core0 fill:#fff3e0
    classDef core1 fill:#e8eaf6
    classDef communication fill:#f1f8e9
    classDef shared fill:#fce4ec
    classDef hardware fill:#efebe9