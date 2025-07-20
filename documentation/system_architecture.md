---
config:
  layout: dagre
---
flowchart TD
    Panel["Panel<br>(Presenter)"] -- contains many --> Component["Component<br>(View)"]
    Component -- has --> SensorModel["Sensor<br>(Model)<br>"]
    Panel -- presents data from --> SensorModel
    PanelManager@{ label: "Panel Manager<br>handles the complete lifecycle of panels including<br>- creation<br style=\"--tw-scale-x:\">- loading<br style=\"--tw-scale-x:\">- updating<br style=\"--tw-scale-x:\">- transitions" } -- manages --> Panel
    PanelManager -- creates --> Panel
    PanelManager -- loads --> Panel
    PanelManager -- switches between --> Panel
    TriggerManager@{ label: "Trigger Manager<br><span style=\"padding-left:\">handles the lifecycle of interrupt triggers<br></span>- registration<br>- evaluation" } -- manages --> Trigger["Trigger<br>- Uses Sensor<br>- Takes Action"]
    TriggerManager -- creates --> Trigger
    TriggerManager -- registers --> Trigger
    TriggerManager -- evaluates --> Trigger
    Trigger -- uses --> SensorModel
    Trigger -- requests --> PanelManager
    Trigger -- modifies --> Application["Application<br>Theme Management"]
    SensorModel -. data .-> Component
    Component -. user interaction .-> Panel
    SensorModel -. sensor data .-> Trigger
    PanelManager@{ shape: rect}
    TriggerManager@{ shape: rect}
     Panel:::mvpPresenter
     Component:::mvpView
     SensorModel:::mvpModel
     PanelManager:::manager
     TriggerManager:::manager
     Trigger:::trigger
    classDef mvpModel fill:#e1f5fe
    classDef mvpView fill:#f3e5f5
    classDef mvpPresenter fill:#e8f5e8
    classDef manager fill:#fff3e0
    classDef trigger fill:#fce4ec
    classDef action fill:#f1f8e9
