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

### Sprint 1: Foundation (Interfaces & Container)
1. Create service interfaces
2. Implement service container
3. Create mock implementations

### Sprint 2: Component Refactoring
1. Refactor oil components for DI
2. Create component factory
3. Update component tests

### Sprint 3: Panel Refactoring
1. Refactor panels for DI
2. Create panel factory
3. Update panel tests

### Sprint 4: Manager Interface Implementation
1. Make managers implement service interfaces directly
2. Remove singleton patterns from managers
3. Update manager tests to use interfaces

### Sprint 5: Application Integration
1. Create composition root
2. Update main application
3. Integration testing

### Sprint 6: Testing & Cleanup
1. Comprehensive test coverage
2. Remove deprecated code
3. Documentation updates

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

### Gradual Migration
1. Start with least coupled managers (StyleManager → IStyleService)
2. Move to more complex managers (PanelManager → IPanelService)
3. Keep both patterns during transition
4. Remove old patterns after validation

### Backward Compatibility
- Maintain old interfaces during migration
- Create adapter patterns where needed
- Deprecate old patterns gradually

### Testing Strategy
- Test each phase independently
- Maintain existing functionality
- Add new tests for DI patterns

## Success Criteria

### Functional
- [ ] All existing functionality preserved
- [ ] No regressions in behavior
- [ ] Improved boot time (better initialization order)

### Architectural
- [ ] No singleton patterns in application code
- [ ] All dependencies explicit in constructors
- [ ] Clear service boundaries
- [ ] Mockable interfaces

### Testing
- [ ] 90%+ unit test coverage
- [ ] All components testable in isolation
- [ ] Integration tests with mocked dependencies
- [ ] Performance tests show no regression

This migration will transform the codebase into a clean, testable, and maintainable architecture following modern C++ and dependency injection best practices.