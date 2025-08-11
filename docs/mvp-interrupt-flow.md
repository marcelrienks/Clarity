# MVP Pattern with Interrupt Integration Flow

This diagram shows how the MVP (Model-View-Presenter) pattern functions in the Clarity system and how interrupts are integrated into the architecture.

## System Flow Diagram

```
                    ┌─────────────────────────────────────────────────────────────┐
                    │                    INTERRUPT LAYER                          │
                    └─────────────────────────────────────────────────────────────┘
                                                    │
                                                    │ LVGL Idle Time
                                                    ▼
                    ┌─────────────────────────────────────────────────────────────┐
                    │              InterruptManager                               │
                    │  ┌─────────────────────────┐ ┌─────────────────────────────┐│
                    │  │    1. Check Triggers    │ │   2. Check Actions          ││
                    │  │     (if none active)    │ │    (if no triggers)         ││
                    │  └─────────────────────────┘ └─────────────────────────────┘│
                    └─────────────────────────────────────────────────────────────┘
                                  │                              │
                                  ▼                              ▼
                    ┌─────────────────────────────────────────────────────────────┐
                    │                    MVP LAYER                                │
                    └─────────────────────────────────────────────────────────────┘

    ┌───────────────────┐              ┌─────────────────────────────────────────────┐
    │   TriggerManager  │              │            ActionManager                    │
    │    (Triggers)     │              │             (Actions)                      │
    │                   │              │                                             │
    │ ┌───────────────┐ │              │ ┌─────────────────────────────────────────┐ │
    │ │ KeySensor     │ │              │ │        Button Input                     │ │
    │ │ LockSensor    │ │────────────▶ │ │    ┌─────────────────────┐            │ │
    │ │ LightSensor   │ │   Panel      │ │    │  1. RegisterPanel() │            │ │
    │ │ ErrorSensor   │ │   Switch     │ │    │  2. Button Press    │            │ │
    │ └───────────────┘ │              │ │    │  3. Get Action      │            │ │
    └───────────────────┘              │ │    │  4. Execute         │            │ │
                                       │ │    └─────────────────────┘            │ │
                                       │ └─────────────────────────────────────────┘ │
                                       └─────────────────────────────────────────────┘
                                                            │
                                                            │ Panel Registration
                                                            ▼
    ┌─────────────────────────────────────────────────────────────────────────────────┐
    │                              PRESENTER LAYER                                    │
    │                                                                                 │
    │  ┌─────────────────────┐  manages  ┌───────────────────────────────────────────┐│
    │  │   PanelManager      │◄──────────┤            Active Panel                   ││
    │  │                     │           │        (IPanel + IActionService)          ││
    │  │ ┌─────────────────┐ │ injects   │                                           ││
    │  │ │ CreateAndLoad() │ │───────────┤ ┌───────────────────────────────────────┐ ││
    │  │ │ UpdatePanel()   │ │    'this' │ │          OemOilPanel                  │ ││
    │  │ │ GetCurrent()    │ │           │ │                                       │ ││
    │  │ └─────────────────┘ │           │ │ GetShortPressAction() ────────────────┼─┤│
    │  └─────────────────────┘           │ │   └─ panelService_->CreateAndLoad()   │ ││
    │                                    │ │                                       │ ││
    │                                    │ │ GetLongPressAction() ─────────────────┼─┤│
    │                                    │ │   └─ styleService_->SetTheme()        │ ││
    │                                    │ └───────────────────────────────────────┘ ││
    │                                    └───────────────────────────────────────────┘│
    └─────────────────────────────────────────────────────────────────────────────────┘
                                                            │
                                                            │ Coordinates
                                                            ▼
    ┌─────────────────────────────────────────────────────────────────────────────────┐
    │                          MODEL + VIEW LAYERS                                    │
    │                                                                                 │
    │ ┌─────────────────────┐           ┌─────────────────────────────────────────────┐│
    │ │      MODELS         │           │                 VIEWS                       ││
    │ │    (Sensors)        │           │              (Components)                   ││
    │ │                     │           │                                             ││
    │ │ ┌─────────────────┐ │  data     │ ┌─────────────────────────────────────────┐ ││
    │ │ │ OilPressure     │ │ ────────▶ │ │        OemOilPressure                   │ ││
    │ │ │ Sensor          │ │           │ │        Component                        │ ││
    │ │ │                 │ │           │ │                                         │ ││
    │ │ │ GetReading()    │ │           │ │ ┌─────────────────────────────────────┐ │ ││
    │ │ └─────────────────┘ │           │ │ │          LVGL Objects               │ │ ││
    │ │                     │           │ │ │        (Arc, Label, etc)            │ │ ││
    │ │ ┌─────────────────┐ │  data     │ │ │                                     │ │ ││
    │ │ │ OilTemperature  │ │ ────────▶ │ │ │ lv_arc_set_value()                  │ │ ││
    │ │ │ Sensor          │ │           │ │ │ lv_label_set_text()                 │ │ ││
    │ │ │                 │ │           │ │ │ lv_obj_set_style()                  │ │ ││
    │ │ │ GetReading()    │ │           │ │ └─────────────────────────────────────┘ │ ││
    │ │ └─────────────────┘ │           │ └─────────────────────────────────────────┘ ││
    │ └─────────────────────┘           └─────────────────────────────────────────────┘│
    └─────────────────────────────────────────────────────────────────────────────────┘
```

## Flow Description

### 1. **Interrupt Processing** (Top Layer)
- `InterruptManager` is called during LVGL idle time
- **Sequential evaluation**: Triggers first, then Actions only if no triggers active
- No priority system - simple ordered evaluation

### 2. **Trigger Flow** (Left Path)
```
GPIO State Change → TriggerManager → PanelManager.CreateAndLoadPanel()
```
- Sensors detect hardware changes (Key, Lock, Light, Error states)
- `TriggerManager` directly calls `PanelManager` to switch panels
- Immediate panel switching based on hardware state

### 3. **Action Flow** (Right Path)
```
Button Press → ActionManager → Panel.GetAction() → Execute Function
```
- `ActionManager` detects button timing (short/long press)
- Calls current panel's `GetShortPressAction()` or `GetLongPressAction()`
- Panel returns function pointer to injected manager methods
- `ActionManager` executes the returned function

### 4. **MVP Pattern Integration**

#### **Presenter (Panels)**
- **Role**: Orchestrate sensors and components, handle business logic
- **Injection**: `PanelManager` injects itself as `IPanelService*` 
- **Actions**: Return function pointers to injected managers:
  - `panelService_->CreateAndLoadPanel("CONFIG")`
  - `styleService_->SetTheme("NIGHT")`

#### **Models (Sensors)**
- **Role**: Abstract hardware data acquisition
- **Examples**: `OilPressureSensor`, `OilTemperatureSensor`
- **Interface**: `GetReading()` returns typed data

#### **Views (Components)**
- **Role**: Render UI elements, no business logic
- **Examples**: `OemOilPressureComponent`, `OemOilTemperatureComponent` 
- **Technology**: LVGL objects (arcs, labels, styling)

### 5. **Key Design Principles**

1. **Separation of Concerns**: Interrupts → MVP → Hardware
2. **Dependency Injection**: Managers injected into panels via constructors
3. **Function Pointers**: Actions directly reference manager methods
4. **Sequential Processing**: Triggers override actions when active
5. **Idle Integration**: All processing during LVGL idle time to prevent conflicts

### 6. **Example Complete Flow**

**Trigger-Driven Panel Switch:**
```
Key Inserted → KeySensor detects HIGH → TriggerManager → PanelManager → Load KeyPanel
```

**Action-Driven Panel Switch:**
```
Long Button Press → ActionManager → OilPanel.GetLongPressAction() → 
panelService_->CreateAndLoadPanel("CONFIG") → PanelManager → Load ConfigPanel
```

**MVP Data Flow:**
```
OilPressureSensor.GetReading() → OilPanel.Update() → OilPressureComponent.Refresh() → 
lv_arc_set_value() → Display Update
```