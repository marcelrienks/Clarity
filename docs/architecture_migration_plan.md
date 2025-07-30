# Dependency Injection Architecture Migration Plan

## Overview
This document outlines the complete migration from the current mixed singleton/factory pattern to a pure dependency injection (DI) architecture. This will improve testability, maintainability, and architectural consistency.

## Current State Analysis

### Current Architectural Issues
1. **Mixed Patterns**: Singleton + Factory + Global variables
2. **Implicit Dependencies**: Components access global state via `GetInstance()`
3. **Hard to Test**: Singletons make unit testing difficult
4. **Hidden Dependencies**: Class dependencies not visible in constructors
5. **Initialization Order Issues**: Singletons may be accessed before initialization

### Current Manager Dependencies
- **StyleManager**: No dependencies (self-contained)
- **PreferenceManager**: No dependencies (NVS access)
- **TriggerManager**: Depends on IGpioProvider
- **PanelManager**: Depends on IDisplayProvider, IGpioProvider

### Current Component Dependencies
- **Oil Components**: Depend on StyleManager
- **Key/Lock Components**: Depend on StyleManager
- **Clarity Component**: Depends on StyleManager
- **All Components**: Implicit dependency on properly initialized LVGL

## Target Architecture

### Core Principles
1. **Explicit Dependencies**: All dependencies injected via constructors
2. **Interface Segregation**: Use interfaces for all injectable services
3. **Single Responsibility**: Each class has one reason to change
4. **Dependency Inversion**: High-level modules don't depend on low-level modules
5. **Testability**: Easy to mock dependencies for unit tests

### Service Interfaces
```cpp
// Core service interfaces (implemented directly by managers)
interface IStyleService      // Implemented by StyleManager
interface IPreferenceService // Implemented by PreferenceManager
interface ITriggerService    // Implemented by TriggerManager
interface IPanelService      // Implemented by PanelManager

// Hardware abstraction interfaces (already exist)
interface IGpioProvider
interface IDisplayProvider

// Factory interfaces
interface IComponentFactory
interface IServiceContainer
```

## Migration Plan

## Phase 1: Service Interface Definition

### 1.1 Create Service Interfaces
- [ ] `IStyleService`: Theme management, color schemes, LVGL styles
- [ ] `IPreferenceService`: Configuration storage and retrieval
- [ ] `ITriggerService`: GPIO monitoring and trigger handling
- [ ] `IPanelService`: Panel lifecycle management
- [ ] `IComponentFactory`: Component creation with dependencies
- [ ] `IServiceContainer`: Dependency resolution container

### 1.2 Manager Interface Implementation
- [ ] Make managers directly implement service interfaces (no additional abstraction layer)
- [ ] Ensure interfaces are mockable for testing
- [ ] Define clear service boundaries
- [ ] Remove singleton patterns from managers

## Phase 2: Component Architecture Refactoring

### 2.1 Component Dependency Injection
- [ ] **Oil Components**: Inject `IStyleService` instead of accessing singleton
- [ ] **Key/Lock Components**: Inject `IStyleService`
- [ ] **Clarity Component**: Inject `IStyleService`
- [ ] **All Components**: Receive dependencies via constructor

### 2.2 Component Factory Pattern
- [ ] Create `IComponentFactory` interface
- [ ] Implement `ComponentFactory` with service dependencies
- [ ] Components created through factory with injected services

```cpp
class ComponentFactory : public IComponentFactory {
    IStyleService* styleService_;
    IDisplayProvider* displayProvider_;
    
public:
    ComponentFactory(IStyleService* style, IDisplayProvider* display);
    std::unique_ptr<IOilComponent> createOilComponent(OilComponentType type);
    std::unique_ptr<IKeyComponent> createKeyComponent();
    // ...
};
```

## Phase 3: Panel Architecture Refactoring

### 3.1 Panel Dependency Injection
- [ ] **OemOilPanel**: Inject `IComponentFactory`, `IDisplayProvider`, `IGpioProvider`
- [ ] **KeyPanel**: Inject `IComponentFactory`, `IDisplayProvider`, `IGpioProvider`
- [ ] **LockPanel**: Inject `IComponentFactory`, `IDisplayProvider`, `IGpioProvider`
- [ ] **SplashPanel**: Inject `IComponentFactory`, `IDisplayProvider`

### 3.2 Panel Factory Pattern
- [ ] Create `IPanelFactory` interface
- [ ] Implement `PanelFactory` with service dependencies
- [ ] Panels created through factory with injected services

```cpp
class PanelFactory : public IPanelFactory {
    IComponentFactory* componentFactory_;
    IDisplayProvider* displayProvider_;
    IGpioProvider* gpioProvider_;
    
public:
    PanelFactory(IComponentFactory* components, IDisplayProvider* display, IGpioProvider* gpio);
    std::unique_ptr<IPanel> createPanel(const std::string& panelType);
};
```

## Phase 4: Service Container Implementation

### 4.1 Dependency Injection Container
- [ ] Create `IServiceContainer` interface
- [ ] Implement `ServiceContainer` class
- [ ] Support singleton and transient lifetimes
- [ ] Automatic dependency resolution

```cpp
class ServiceContainer : public IServiceContainer {
public:
    template<typename T>
    void registerSingleton(std::function<std::unique_ptr<T>()> factory);
    
    template<typename T>
    void registerTransient(std::function<std::unique_ptr<T>(IServiceContainer*)> factory);
    
    template<typename T>
    T* resolve();
    
    template<typename T>
    std::unique_ptr<T> create();
};
```

### 4.2 Service Registration
- [ ] Register all services with container in main.cpp
- [ ] Define service lifetimes (singleton vs transient)
- [ ] Handle circular dependencies if any

## Phase 5: Manager Interface Implementation

### 5.1 Direct Manager Interface Implementation
- [ ] `StyleManager` directly implements `IStyleService` (no separate service class)
- [ ] `PreferenceManager` directly implements `IPreferenceService`
- [ ] `TriggerManager` directly implements `ITriggerService`
- [ ] `PanelManager` directly implements `IPanelService`

### 5.2 Remove Singleton Pattern
- [ ] Remove all `GetInstance()` methods from managers
- [ ] Remove static instances and global variables
- [ ] Add constructor injection for dependencies
- [ ] Ensure thread safety where needed

## Phase 6: Application Composition Root

### 6.1 Main Application Setup
- [ ] Configure service container in main.cpp
- [ ] Register all services and their dependencies
- [ ] Create application root that orchestrates everything

```cpp
void setup() {
    // Create service container
    auto container = std::make_unique<ServiceContainer>();
    
    // Register hardware providers
    container->registerSingleton<IGpioProvider>([]() {
        return std::make_unique<Esp32GpioProvider>();
    });
    
    container->registerSingleton<IDisplayProvider>([&]() {
        auto& device = Device::GetInstance();
        return std::make_unique<LvglDisplayProvider>(device.screen);
    });
    
    // Register managers as services (direct interface implementation)
    container->registerSingleton<IStyleService>([&]() {
        auto manager = std::make_unique<StyleManager>();
        manager->init(Themes::DAY);
        return manager;
    });
    
    container->registerSingleton<IPreferenceService>([]() {
        auto manager = std::make_unique<PreferenceManager>();
        manager->init();
        return manager;
    });
    
    container->registerSingleton<ITriggerService>([&]() {
        return std::make_unique<TriggerManager>(
            container->resolve<IGpioProvider>()
        );
    });
    
    container->registerSingleton<IPanelService>([&]() {
        return std::make_unique<PanelManager>(
            container->resolve<IDisplayProvider>(),
            container->resolve<IGpioProvider>()
        );
    });
    
    // Register factories
    container->registerSingleton<IComponentFactory>([&]() {
        return std::make_unique<ComponentFactory>(
            container->resolve<IStyleService>(),
            container->resolve<IDisplayProvider>()
        );
    });
    
    container->registerSingleton<IPanelFactory>([&]() {
        return std::make_unique<PanelFactory>(
            container->resolve<IComponentFactory>(),
            container->resolve<IDisplayProvider>(),
            container->resolve<IGpioProvider>()
        );
    });
    
    // Create application with injected manager interfaces
    auto app = std::make_unique<ClarityApplication>(
        container->resolve<IPanelService>(),    // PanelManager as IPanelService
        container->resolve<ITriggerService>(),  // TriggerManager as ITriggerService
        container->resolve<IPreferenceService>() // PreferenceManager as IPreferenceService
    );
    
    app->run();
}
```

## Phase 7: Testing Infrastructure

### 7.1 Mock Implementations
- [ ] Create mock implementations of all service interfaces
- [ ] `MockStyleManager` (implements `IStyleService`)
- [ ] `MockPreferenceManager` (implements `IPreferenceService`)
- [ ] `MockTriggerManager` (implements `ITriggerService`)
- [ ] `MockPanelManager` (implements `IPanelService`)
- [ ] `MockComponentFactory`
- [ ] `MockPanelFactory`

### 7.2 Test Utilities
- [ ] Create test service container
- [ ] Create test builders for complex object graphs
- [ ] Create test fixtures with pre-configured mocks

```cpp
class TestServiceContainer {
public:
    template<typename Interface, typename Mock>
    void registerMock(std::unique_ptr<Mock> mock);
    
    template<typename T>
    T* resolve();
    
    void reset();
};

class OilComponentTestBuilder {
    TestServiceContainer container_;
    
public:
    OilComponentTestBuilder& withMockStyle(std::unique_ptr<MockStyleManager> mock);
    OilComponentTestBuilder& withMockDisplay(std::unique_ptr<MockDisplayProvider> mock);
    std::unique_ptr<OilComponent> build();
};
```

## Implementation Order

### Sprint 1: Foundation (Interfaces & Container) ✅
**Goal:** Create foundation interfaces and container infrastructure
**Testability:** Each step should compile and existing functionality preserved

1. **Step 1.1:** Create service interfaces ✅
   - *Testing:* Code compiles, interfaces are well-defined
   - *Manual Test:* Build project with `pio run -e debug-local`
   
2. **Step 1.2:** Implement service container
   - *Testing:* Container unit tests pass, registration/resolution works
   - *Manual Test:* Container can register and resolve test services
   
3. **Step 1.3:** Create mock implementations
   - *Testing:* Mock services implement interfaces correctly
   - *Manual Test:* Test suite can use mocks for isolated testing

### Sprint 2: Component Refactoring
**Goal:** Convert components to use dependency injection
**Testability:** Components render correctly with injected dependencies

1. **Step 2.1:** Refactor oil components for DI
   - *Testing:* Oil panel displays correctly, gauges render as arcs
   - *Manual Test:* Load oil panel via Wokwi simulator, verify gauge rendering
   
2. **Step 2.2:** Create component factory with DI
   - *Testing:* Factory creates components with proper dependencies
   - *Manual Test:* All component types can be created via factory
   
3. **Step 2.3:** Update component tests
   - *Testing:* All component unit tests pass with new DI approach
   - *Manual Test:* Test coverage maintained, no regression in functionality

### Sprint 3: Panel Refactoring
**Goal:** Convert panels to use dependency injection
**Testability:** Panel transitions work correctly with injected services

1. **Step 3.1:** Refactor panels for DI
   - *Testing:* All panels load correctly, transitions smooth
   - *Manual Test:* Navigate between panels (splash → oil → key → lock)
   
2. **Step 3.2:** Create panel factory with DI
   - *Testing:* Factory creates panels with proper dependencies
   - *Manual Test:* Panel switching works via trigger system
   
3. **Step 3.3:** Update panel tests
   - *Testing:* All panel unit tests pass with new DI approach
   - *Manual Test:* Integration tests verify panel behavior

### Sprint 4: Manager Interface Implementation
**Goal:** Make managers implement service interfaces directly
**Testability:** Application works identically to before, but with interfaces

1. **Step 4.1:** StyleManager implements IStyleService
   - *Testing:* Themes apply correctly, styles render properly
   - *Manual Test:* Switch between day/night themes, verify style changes
   
2. **Step 4.2:** PanelManager implements IPanelService
   - *Testing:* Panel management works identically to before
   - *Manual Test:* Panel loading, updating, and transitions function correctly
   
3. **Step 4.3:** TriggerManager implements ITriggerService
   - *Testing:* GPIO triggers work correctly, panel switching responsive
   - *Manual Test:* Test trigger-driven panel changes, verify responsiveness
   
4. **Step 4.4:** PreferenceManager implements IPreferenceService
   - *Testing:* Configuration saves/loads correctly, defaults work
   - *Manual Test:* Change settings, restart device, verify persistence
   
5. **Step 4.5:** Remove singleton GetInstance() methods
   - *Testing:* Application compiles, no more singleton access
   - *Manual Test:* Full application functionality preserved

### Sprint 5: Application Integration
**Goal:** Wire everything together with service container
**Testability:** Complete application works with full dependency injection

1. **Step 5.1:** Create composition root in main.cpp
   - *Testing:* Application starts correctly with DI container
   - *Manual Test:* Device boots, loads default panel, functions normally
   
2. **Step 5.2:** Remove global manager variables
   - *Testing:* No global state, all dependencies injected
   - *Manual Test:* Application behavior identical to singleton version
   
3. **Step 5.3:** Integration testing with real hardware
   - *Testing:* All integration tests pass, no regression
   - *Manual Test:* Full device testing - display, GPIO, triggers, persistence

### Sprint 6: Testing & Cleanup
**Goal:** Comprehensive testing and code cleanup
**Testability:** Full test coverage with clean, maintainable code

1. **Step 6.1:** Comprehensive unit test coverage
   - *Testing:* 90%+ code coverage, all critical paths tested
   - *Manual Test:* Test suite runs quickly, provides clear feedback
   
2. **Step 6.2:** Remove deprecated singleton code
   - *Testing:* Clean compile, no deprecated warnings
   - *Manual Test:* Code review shows clean architecture
   
3. **Step 6.3:** Performance testing and optimization
   - *Testing:* No performance regression, startup time acceptable
   - *Manual Test:* Device feels responsive, no noticeable slowdown
   
4. **Step 6.4:** Documentation updates
   - *Testing:* Documentation matches implementation
   - *Manual Test:* New developers can understand and extend the system

## Benefits After Migration

### Testability
- Easy to unit test components in isolation
- Mock dependencies for focused testing
- No global state to manage in tests

### Maintainability
- Clear dependency relationships
- Easy to modify behavior through interface implementations
- Compile-time dependency checking

### Flexibility
- Easy to swap implementations (e.g., different display providers)
- Support for multiple configurations
- Plugin architecture possible

## Risks & Mitigation

### Memory Overhead
- **Risk**: Increased memory usage from DI container
- **Mitigation**: Use efficient container implementation, profile memory usage

### Complexity
- **Risk**: Increased initial complexity
- **Mitigation**: Good documentation, clear examples, gradual migration

### Performance
- **Risk**: Potential performance impact from indirect calls
- **Mitigation**: Profile critical paths, inline where necessary

## Migration Strategy

### Gradual Migration with Continuous Testing
1. **Start with least coupled managers** (StyleManager → IStyleService)
   - *Test after each change:* Verify themes and styling work correctly
   - *Manual verification:* Load application, switch themes, verify visual changes
   
2. **Move to more complex managers** (PanelManager → IPanelService)
   - *Test after each change:* Verify panel loading and transitions
   - *Manual verification:* Navigate between all panels, test trigger responses
   
3. **Keep both patterns during transition**
   - *Validate continuously:* Ensure no functionality loss during migration
   - *Performance check:* Monitor startup time and responsiveness
   
4. **Remove old patterns only after full validation**
   - *Comprehensive testing:* Full test suite passes before removal
   - *Integration testing:* Real hardware testing confirms no regression

### Backward Compatibility with Validation
- **Maintain old interfaces during migration**
  - *Step-by-step testing:* Each interface change tested independently
  - *Rollback capability:* Can revert to previous step if issues found
  
- **Create adapter patterns where needed**
  - *Isolated testing:* Adapters tested separately from main logic
  - *Behavioral verification:* Ensure adapters preserve exact behavior
  
- **Deprecate old patterns gradually**
  - *Warning phase:* Mark deprecated but keep functional
  - *Testing phase:* Verify new patterns work before removing old ones

### Comprehensive Testing Strategy
- **Test each step independently**
  - *Unit tests:* Each service interface has comprehensive test coverage
  - *Integration tests:* Manager-to-interface conversion tested thoroughly
  - *Manual testing:* Real device testing after each major step
  
- **Maintain existing functionality**
  - *Regression testing:* Automated tests prevent functionality loss
  - *Visual testing:* Screenshots/recordings verify UI behavior unchanged
  - *Performance testing:* Ensure no performance degradation
  
- **Add new tests for DI patterns**
  - *Mock testing:* Verify components work with mocked dependencies
  - *Injection testing:* Confirm proper dependency injection flow
  - *Container testing:* Service container registration/resolution tested

## Success Criteria

### Functional (Verified at Each Step)
- [ ] **All existing functionality preserved**
  - *Per-step verification:* Manual testing after each major change
  - *Automated verification:* Integration tests pass continuously
  - *Visual verification:* Screenshots show identical UI behavior
  
- [ ] **No regressions in behavior**
  - *Regression testing:* Automated test suite prevents functionality loss
  - *Performance monitoring:* No slowdown in critical operations
  - *User experience:* Device feels identical to singleton version
  
- [ ] **Improved boot time (better initialization order)**
  - *Measurement:* Track startup time at each step
  - *Optimization:* Dependency injection enables optimized initialization
  - *Validation:* Boot time equal or better than original

### Architectural (Validated Incrementally)
- [ ] **No singleton patterns in application code**
  - *Step-by-step removal:* Each GetInstance() method removed and tested
  - *Code review:* Static analysis confirms no global state access
  - *Testing verification:* All tests work without singletons
  
- [ ] **All dependencies explicit in constructors**
  - *Interface compliance:* Every service follows DI constructor pattern
  - *Documentation:* Constructor parameters clearly document dependencies
  - *Testing:* Mock injection proves explicit dependency pattern
  
- [ ] **Clear service boundaries**
  - *Interface design:* Each service has focused, single-responsibility interface
  - *Dependency mapping:* Clear dependency graph with no circular references
  - *Testing isolation:* Services can be tested in complete isolation
  
- [ ] **Mockable interfaces**
  - *Mock implementation:* Every interface has corresponding mock
  - *Test coverage:* Unit tests use mocks for all dependencies
  - *Verification:* Components work correctly with mocked services

### Testing (Comprehensive Verification)
- [ ] **90%+ unit test coverage**
  - *Incremental measurement:* Coverage tracked and improved at each step
  - *Critical path focus:* All essential functionality covered by tests
  - *Quality gates:* No step proceeds without maintaining coverage threshold
  
- [ ] **All components testable in isolation**
  - *Individual verification:* Each component tested with mocked dependencies
  - *Fast test execution:* Isolated tests run quickly for rapid feedback
  - *Dependency injection:* Easy to inject test doubles for all dependencies
  
- [ ] **Integration tests with mocked dependencies**
  - *System-level testing:* Full application flow tested with controlled inputs
  - *Realistic scenarios:* Tests cover real user interaction patterns
  - *Reliable execution:* Tests are deterministic and repeatable
  
- [ ] **Performance tests show no regression**
  - *Benchmark establishment:* Current performance metrics documented
  - *Continuous monitoring:* Performance tracked throughout migration
  - *Regression detection:* Automated alerts if performance degrades

This migration will transform the codebase into a clean, testable, and maintainable architecture following modern C++ and dependency injection best practices.