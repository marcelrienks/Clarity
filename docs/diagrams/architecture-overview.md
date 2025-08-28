# Architecture Overview Diagram

This diagram shows the high-level component relationships in the Clarity system with the implemented 8-step interrupt flow and button timing differentiation.

## Key Architectural Elements

- **8-Step Interrupt Flow**: InterruptManager implements precise main loop flow with evaluation/execution separation
- **Button Timing Logic**: Built-in short press (50ms-2000ms) and long press (2000ms-5000ms) differentiation
- **Always vs Idle Processing**: Queued evaluation always runs, polled evaluation only during UI idle
- **Dual Factory Pattern**: ProviderFactory creates providers, ManagerFactory creates managers with dependency injection
- **Interface-Based Factories**: IProviderFactory enables testability with mock provider injection
- **Handler-Owned Sensors**: Handlers create and own their respective sensors internally
- **Panel Self-Sufficiency**: Panels create their own components and data sensors internally
- **Split Sensor Design**: Independent KeyPresentSensor and KeyNotPresentSensor classes
- **Static Callbacks**: InterruptCallbacks utility with single execution function per interrupt
- **Centralized Restoration**: InterruptManager::HandleRestoration() manages all panel restoration decisions
- **Display-Only GPIO Panels**: Key/Lock panels don't create sensors, only display state
- **Hardware Abstraction**: Providers isolate hardware dependencies

```mermaid
graph TB
    %% Main Application
    Main[main.cpp]
    
    %% Factories (Interface-based)
    ProviderFactory[ProviderFactory<br/>implements IProviderFactory]
    ManagerFactory[ManagerFactory<br/>future: implements IManagerFactory]
    
    %% Core Managers (IManager interface)
    InterruptManager[InterruptManager<br/>implements IManager<br/>8-Step Flow with Button Timing]
    PanelManager[PanelManager<br/>implements IManager]
    StyleManager[StyleManager<br/>implements IManager]
    PreferenceManager[PreferenceManager<br/>implements IManager]
    ErrorManager[ErrorManager<br/>implements IManager]
    
    %% Coordinated Handlers (IHandler implementations)  
    PolledHandler[PolledHandler<br/>implements IHandler<br/>GPIO state monitoring - IDLE ONLY]
    QueuedHandler[QueuedHandler<br/>implements IHandler<br/>Button timing & execution - ALWAYS + IDLE]
    
    %% Sensors (Models - ISensor + BaseSensor)
    KeyPresentSensor[KeyPresentSensor<br/>implements ISensor<br/>GPIO 25]
    KeyNotPresentSensor[KeyNotPresentSensor<br/>implements ISensor<br/>GPIO 26]
    LockSensor[LockSensor<br/>implements ISensor<br/>GPIO 27]
    LightsSensor[LightsSensor<br/>implements ISensor<br/>GPIO 33]
    ActionButtonSensor[ActionButtonSensor<br/>implements ISensor<br/>GPIO 32 - State Only]
    OilPressureSensor[OilPressureSensor<br/>implements ISensor<br/>ADC]
    OilTemperatureSensor[OilTemperatureSensor<br/>implements ISensor<br/>ADC]
    BaseSensor[BaseSensor<br/>Change Detection Template]
    
    %% Panels (Presenters - IPanel implementations)
    SplashPanel[SplashPanel<br/>implements IPanel]
    OilPanel[OilPanel<br/>implements IPanel]
    KeyPanel[KeyPanel<br/>implements IPanel<br/>Display Only]
    LockPanel[LockPanel<br/>implements IPanel<br/>Display Only]
    ErrorPanel[ErrorPanel<br/>implements IPanel]
    ConfigPanel[ConfigPanel<br/>implements IPanel]
    
    %% Components (Views - IComponent implementations)
    OilGaugeComponent[OilGaugeComponent<br/>implements IComponent]
    SplashComponent[SplashComponent<br/>implements IComponent]
    KeyStatusComponent[KeyStatusComponent<br/>implements IComponent]
    LockStatusComponent[LockStatusComponent<br/>implements IComponent]
    ErrorListComponent[ErrorListComponent<br/>implements IComponent]
    ConfigListComponent[ConfigListComponent<br/>implements IComponent]
    
    %% Providers (Hardware Abstraction)
    GpioProvider[GpioProvider<br/>implements IGpioProvider]
    DisplayProvider[DisplayProvider<br/>implements IDisplayProvider]
    DeviceProvider[DeviceProvider<br/>implements IDeviceProvider<br/>Hardware Driver]
    
    %% Utilities
    Types[types.h<br/>Structs & Enums]
    InterruptCallbacks[InterruptCallbacks<br/>Static Functions]
    
    %% Main Creates Both Factories
    Main --> ProviderFactory
    Main --> ManagerFactory
    
    %% Provider Factory Creates Providers
    ProviderFactory --> GpioProvider
    ProviderFactory --> DisplayProvider
    ProviderFactory --> DeviceProvider
    
    %% Manager Factory Creates Managers (with injected providers)
    ManagerFactory --> InterruptManager
    ManagerFactory --> PanelManager
    ManagerFactory --> StyleManager
    ManagerFactory --> PreferenceManager
    ManagerFactory --> ErrorManager
    
    %% Provider Dependencies
    DisplayProvider --> DeviceProvider
    
    %% Factory Dependency Injection
    Main -.->|injects IProviderFactory| ManagerFactory
    
    %% Managers Create Their Dependencies
    InterruptManager --> PolledHandler
    InterruptManager --> QueuedHandler
    
    %% Specialized Handlers Create and Own Their Sensors
    PolledHandler --> KeyPresentSensor
    PolledHandler --> KeyNotPresentSensor
    PolledHandler --> LockSensor
    PolledHandler --> LightsSensor
    QueuedHandler --> ActionButtonSensor
    
    %% Panel Manager Creates Panels On Demand
    PanelManager --> SplashPanel
    PanelManager --> OilPanel
    PanelManager --> KeyPanel
    PanelManager --> LockPanel
    PanelManager --> ErrorPanel
    PanelManager --> ConfigPanel
    
    %% Panels Create Their Own Components and Data Sensors
    OilPanel --> OilPressureSensor
    OilPanel --> OilTemperatureSensor
    OilPanel --> OilGaugeComponent
    SplashPanel --> SplashComponent
    KeyPanel --> KeyStatusComponent
    LockPanel --> LockStatusComponent
    ErrorPanel --> ErrorListComponent
    ConfigPanel --> ConfigListComponent
    
    %% Hardware Dependencies
    KeyPresentSensor --> GpioProvider
    KeyNotPresentSensor --> GpioProvider
    LockSensor --> GpioProvider
    LightsSensor --> GpioProvider
    ActionButtonSensor --> GpioProvider
    OilPressureSensor --> GpioProvider
    OilTemperatureSensor --> GpioProvider
    
    %% Panel Provider Dependencies
    SplashPanel -.->|uses| DisplayProvider
    OilPanel -.->|uses| DisplayProvider
    OilPanel -.->|uses| GpioProvider
    KeyPanel -.->|uses| DisplayProvider
    KeyPanel -.->|uses| GpioProvider
    LockPanel -.->|uses| DisplayProvider
    LockPanel -.->|uses| GpioProvider
    ErrorPanel -.->|uses| DisplayProvider
    ConfigPanel -.->|uses| DisplayProvider
    
    %% Inheritance
    KeyPresentSensor --> BaseSensor
    KeyNotPresentSensor --> BaseSensor
    LockSensor --> BaseSensor
    LightsSensor --> BaseSensor
    ActionButtonSensor --> BaseSensor
    OilPressureSensor --> BaseSensor
    OilTemperatureSensor --> BaseSensor
    
    %% Static Callbacks and Context
    PolledHandler -.->|registers POLLED interrupts with context| InterruptCallbacks
    QueuedHandler -.->|registers QUEUED interrupts with context| InterruptCallbacks
    InterruptCallbacks --> Types
    
    %% Manager Dependencies and Provider Injection
    ManagerFactory -.->|gets providers from| ProviderFactory
    PanelManager -.->|injects providers to| SplashPanel
    PanelManager -.->|injects providers to| OilPanel
    PanelManager -.->|injects providers to| KeyPanel
    PanelManager -.->|injects providers to| LockPanel
    PanelManager -.->|injects providers to| ErrorPanel
    PanelManager -.->|injects providers to| ConfigPanel
    PreferenceManager -.->|provides config to| PanelManager
    PreferenceManager -.->|provides config to| OilPanel
    
    %% Coordinated Interrupt System Integration
    PolledHandler -.->|SET_THEME effects via| StyleManager
    PolledHandler -.->|LOAD_PANEL effects via| PanelManager
    QueuedHandler -.->|LOAD_PANEL effects via| PanelManager
    QueuedHandler -.->|SET_PREFERENCE effects via| PreferenceManager
    ErrorManager -.->|POLLED by| PolledHandler
    
    %% Styling
    classDef factory fill:#e1f5fe,stroke:#0277bd,stroke-width:2px
    classDef manager fill:#f3e5f5,stroke:#7b1fa2,stroke-width:2px
    classDef handler fill:#ffecb3,stroke:#f57f17,stroke-width:2px
    classDef sensor fill:#e8f5e8,stroke:#388e3c,stroke-width:2px
    classDef panel fill:#fce4ec,stroke:#c2185b,stroke-width:2px
    classDef component fill:#fff3e0,stroke:#f57c00,stroke-width:2px
    classDef provider fill:#e0f2f1,stroke:#00796b,stroke-width:2px
    classDef utility fill:#f5f5f5,stroke:#616161,stroke-width:2px
    
    class ProviderFactory,ManagerFactory factory
    class InterruptManager,PanelManager,StyleManager,PreferenceManager,ErrorManager manager
    class PolledHandler,QueuedHandler handler
    class KeyPresentSensor,KeyNotPresentSensor,LockSensor,LightsSensor,ActionButtonSensor,OilPressureSensor,OilTemperatureSensor,BaseSensor sensor
    class SplashPanel,OilPanel,KeyPanel,LockPanel,ErrorPanel,ConfigPanel panel
    class OilGaugeComponent,SplashComponent,KeyStatusComponent,LockStatusComponent,ErrorListComponent,ConfigListComponent component
    class GpioProvider,DisplayProvider,DeviceProvider provider
    class Types,InterruptCallbacks utility
```

## Component Responsibilities

### Factories
- **ProviderFactory**: Implements IProviderFactory interface, creates all hardware providers (GpioProvider, DisplayProvider, DeviceProvider)
- **ManagerFactory**: Creates all managers, receives IProviderFactory for dependency injection

### Managers
- **InterruptManager**: Implements 8-step interrupt flow with evaluation/execution separation, manages button press timing (50ms-2000ms short, 2000ms-5000ms long), coordinates PolledHandler and QueuedHandler with centralized restoration logic
- **PanelManager**: Creates panels on demand, manages lifecycle, switching, and restoration tracking
- **StyleManager**: Theme management (Day/Night) based on LightsSensor
- **PreferenceManager**: Persistent settings storage
- **ErrorManager**: Error collection with coordinated interrupt system integration

### Coordinated Handlers (IHandler Interface)
- **PolledHandler**: Creates and owns GPIO sensors for state monitoring, processes POLLED interrupts during UI IDLE time only, manages GPIO state changes and panel loading effects
- **QueuedHandler**: Creates and owns button sensor, provides button state to InterruptManager for timing logic, executes queued button actions during UI IDLE time only
- **Centralized Processing**: InterruptManager handles button press detection and timing logic directly, handlers focus on their specialized interrupt types

### Sensors (ISensor + BaseSensor)
- **GPIO Interrupt Sensors**: Created and owned by PolledHandler for GPIO state monitoring (Key, Lock, Lights sensors)
- **Button State Sensor**: Created and owned by QueuedHandler, provides GPIO state only - timing logic handled by InterruptManager
- **Data Sensors**: Created by data panels for continuous measurement (Oil pressure, temperature)
- **BaseSensor**: Provides change detection template for all sensors

### Panels (IPanel Interface)
- **Display-Only Panels**: Display-only, create own components but no sensors (Key, Lock)
- **Data Panels**: Create own sensors and components for data acquisition (Oil)
- **Utility Panels**: Create own components for system functions (Splash, Error, Config)

### Critical Architecture Constraints
- **8-Step Interrupt Flow**: Precise main loop sequence with evaluation/execution separation for optimal UI performance
- **Button Timing Integration**: Built-in press duration measurement (50ms-2000ms short, 2000ms-5000ms long) in InterruptManager
- **Always vs Idle Processing**: Queued interrupt evaluation (button detection) runs every loop, polled evaluation only during UI idle
- **Interface-Based Design**: All major components implement interfaces for testability and loose coupling
- **Dual Factory Pattern**: Separate factories for providers (ProviderFactory) and managers (ManagerFactory)
- **Dependency Injection**: IProviderFactory interface enables test mocking and clean separation
- **Generic Factory Support**: Template-based IFactory<T> for type-safe component creation
- **Centralized Restoration**: InterruptManager::HandleRestoration() eliminates distributed restoration complexity
- **Effect-Based Execution**: Interrupts categorized by effect (LOAD_PANEL, SET_THEME, BUTTON_ACTION) for simplified logic
- **Specialized Ownership**: PolledHandler owns GPIO sensors, QueuedHandler owns button sensor
- **Memory Safety**: Designed for ESP32 320KB RAM constraint with optimized data structures
- **Change Detection**: POLLED interrupts fire on state transitions only, button timing handled by InterruptManager