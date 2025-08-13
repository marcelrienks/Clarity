# Factory Refactoring Plan: Option 4 with Dependency Injection

## Overview
Replace the current registry-based ComponentFactory and PanelFactory with direct factory methods that use dependency injection for testability. This combines compile-time type safety with runtime testability.

## Current State
- Registry-based factories with string lookups
- InitializeFactories() registration step required
- Runtime component/panel resolution via string names
- Good testability but magic strings and runtime overhead

## Target State
- Direct factory methods with compile-time type safety
- Dependency injection via interfaces for testability
- No registration step or magic strings
- Zero runtime overhead with full test coverage

## Implementation Plan

### Phase 1: Create Factory Interfaces

#### 1.1 Create IComponentFactory Interface
**File:** `include/interfaces/i_component_factory.h`
```cpp
class IComponentFactory {
public:
    virtual ~IComponentFactory() = default;
    virtual std::unique_ptr<ClarityComponent> CreateClarityComponent(IStyleService* style) = 0;
    virtual std::unique_ptr<OilPressureComponent> CreateOilPressureComponent(IStyleService* style) = 0;
    virtual std::unique_ptr<OilTemperatureComponent> CreateOilTemperatureComponent(IStyleService* style) = 0;
    virtual std::unique_ptr<ErrorComponent> CreateErrorComponent(IStyleService* style) = 0;
    virtual std::unique_ptr<KeyComponent> CreateKeyComponent(IStyleService* style) = 0;
    virtual std::unique_ptr<LockComponent> CreateLockComponent(IStyleService* style) = 0;
    virtual std::unique_ptr<ConfigComponent> CreateConfigComponent(IStyleService* style) = 0;
};
```

#### 1.2 Create IPanelFactory Interface
**File:** `include/interfaces/i_panel_factory.h`
```cpp
class IPanelFactory {
public:
    virtual ~IPanelFactory() = default;
    virtual std::unique_ptr<SplashPanel> CreateSplashPanel(IGpioProvider* gpio, IDisplayProvider* display, IStyleService* style) = 0;
    virtual std::unique_ptr<OemOilPanel> CreateOemOilPanel(IGpioProvider* gpio, IDisplayProvider* display, IStyleService* style) = 0;
    virtual std::unique_ptr<ErrorPanel> CreateErrorPanel(IGpioProvider* gpio, IDisplayProvider* display, IStyleService* style) = 0;
    virtual std::unique_ptr<ConfigPanel> CreateConfigPanel(IGpioProvider* gpio, IDisplayProvider* display, IStyleService* style) = 0;
    virtual std::unique_ptr<KeyPanel> CreateKeyPanel(IGpioProvider* gpio, IDisplayProvider* display, IStyleService* style) = 0;
    virtual std::unique_ptr<LockPanel> CreateLockPanel(IGpioProvider* gpio, IDisplayProvider* display, IStyleService* style) = 0;
};
```

### Phase 2: Update Concrete Factory Implementations

#### 2.1 Update ComponentFactory
**File:** `include/factories/component_factory.h`
```cpp
class ComponentFactory : public IComponentFactory {
public:
    static ComponentFactory& Instance();
    
    // IComponentFactory implementation
    std::unique_ptr<ClarityComponent> CreateClarityComponent(IStyleService* style) override;
    std::unique_ptr<OilPressureComponent> CreateOilPressureComponent(IStyleService* style) override;
    // ... other component methods
    
private:
    ComponentFactory() = default;
};
```

#### 2.2 Update PanelFactory
**File:** `include/factories/panel_factory.h`
```cpp
class PanelFactory : public IPanelFactory {
public:
    static PanelFactory& Instance();
    
    // IPanelFactory implementation  
    std::unique_ptr<SplashPanel> CreateSplashPanel(IGpioProvider* gpio, IDisplayProvider* display, IStyleService* style) override;
    std::unique_ptr<OemOilPanel> CreateOemOilPanel(IGpioProvider* gpio, IDisplayProvider* display, IStyleService* style) override;
    // ... other panel methods
    
private:
    PanelFactory() = default;
};
```

### Phase 3: Update Panel Constructors for Dependency Injection

#### 3.1 Update Panel Constructors
Inject IComponentFactory into panels that create components:

**Files to Update:**
- `include/panels/splash_panel.h`
- `include/panels/oem_oil_panel.h` 
- `include/panels/error_panel.h`
- `include/panels/key_panel.h`
- `include/panels/lock_panel.h`

**Example (SplashPanel):**
```cpp
class SplashPanel : public IPanel {
private:
    IComponentFactory* componentFactory_;
    
public:
    SplashPanel(IGpioProvider* gpio, IDisplayProvider* display, IStyleService* style,
                IComponentFactory* componentFactory = &ComponentFactory::Instance());
    
    void Load() override;
};
```

#### 3.2 Update Panel Implementations
Replace string-based component creation with direct method calls:

**Before:**
```cpp
component_ = ComponentFactory::CreateComponent("Clarity", styleService_);
```

**After:**
```cpp
component_ = componentFactory_->CreateClarityComponent(styleService_);
```

### Phase 4: Update PanelManager for Dependency Injection

#### 4.1 Update PanelManager Constructor
**File:** `include/managers/panel_manager.h`
```cpp
class PanelManager : public IPanelService {
private:
    IPanelFactory* panelFactory_;
    IComponentFactory* componentFactory_;
    
public:
    PanelManager(IDisplayProvider* display, IGpioProvider* gpio, IStyleService* styleService,
                 IActionManager* actionManager, IPreferenceService* preferenceService,
                 IPanelFactory* panelFactory = &PanelFactory::Instance(),
                 IComponentFactory* componentFactory = &ComponentFactory::Instance());
};
```

#### 4.2 Update CreatePanel Method
Replace string-based panel creation:

**Before:**
```cpp
std::unique_ptr<IPanel> uniquePanel = PanelFactory::CreatePanel(
    panelName, gpioProvider_, displayProvider_, styleService_);
```

**After:**
```cpp
std::unique_ptr<IPanel> uniquePanel;
if (strcmp(panelName, PanelNames::SPLASH) == 0) {
    uniquePanel = panelFactory_->CreateSplashPanel(gpioProvider_, displayProvider_, styleService_);
} else if (strcmp(panelName, PanelNames::OIL) == 0) {
    uniquePanel = panelFactory_->CreateOemOilPanel(gpioProvider_, displayProvider_, styleService_);
}
// ... other panel types
```

### Phase 5: Update Main.cpp and Remove Registry System

#### 5.1 Update Service Creation
**File:** `src/main.cpp`
```cpp
// Remove InitializeFactories() call
// Update PanelManager creation to inject factories if needed
panelManager = ManagerFactory::createPanelManager(displayProvider.get(), gpioProvider.get(), 
                                                  styleManager.get(), actionManager.get(), 
                                                  preferenceManager.get());
```

#### 5.2 Remove Registry Files
**Files to Remove:**
- `include/factories/factory_registration.h`
- `src/factories/factory_registration.cpp`

### Phase 6: Testing Support

#### 6.1 Create Mock Factories
**File:** `test/mocks/mock_component_factory.h`
```cpp
class MockComponentFactory : public IComponentFactory {
public:
    MOCK_METHOD(std::unique_ptr<ClarityComponent>, CreateClarityComponent, (IStyleService*), (override));
    MOCK_METHOD(std::unique_ptr<OilPressureComponent>, CreateOilPressureComponent, (IStyleService*), (override));
    // ... other mock methods
};
```

#### 6.2 Update Existing Tests
Update panel tests to inject mock factories for isolated testing.

## Benefits of This Approach

### Compile-Time Benefits
- **Type Safety**: No magic strings, compile-time method resolution
- **IDE Support**: Autocomplete, refactoring, go-to-definition
- **Performance**: Direct method calls, no hash map lookups

### Runtime Benefits  
- **Zero Registration**: No initialization step required
- **Predictable**: No runtime configuration or string parsing
- **Debuggable**: Clear stack traces with actual method names

### Testing Benefits
- **Mockable**: Easy to inject test doubles via interfaces
- **Isolated**: Test panels without real component dependencies
- **Flexible**: Can verify factory method calls and return custom implementations

### Maintenance Benefits
- **Explicit**: Clear what components/panels exist from factory interface
- **Familiar**: Similar to previous UIFactory pattern
- **Simple**: Less abstraction than registry-based approach

## Migration Strategy

1. **Backward Compatible**: Implement new interfaces alongside existing factories
2. **Incremental**: Update one panel at a time to use dependency injection
3. **Test Coverage**: Ensure each panel has tests with mock factories before migration
4. **Remove Registry**: Only remove registry system after all panels migrated

## Files Affected

### New Files
- `include/interfaces/i_component_factory.h`
- `include/interfaces/i_panel_factory.h` 
- `test/mocks/mock_component_factory.h`
- `test/mocks/mock_panel_factory.h`

### Modified Files
- `include/factories/component_factory.h`
- `src/factories/component_factory.cpp`
- `include/factories/panel_factory.h`
- `src/factories/panel_factory.cpp`
- `include/managers/panel_manager.h`
- `src/managers/panel_manager.cpp`
- `src/main.cpp`
- All panel header/implementation files

### Removed Files
- `include/factories/factory_registration.h`
- `src/factories/factory_registration.cpp`

## Success Criteria
- [ ] All panels compile with type-safe factory method calls
- [ ] No magic strings in component/panel creation
- [ ] Full test coverage with mock factory injection
- [ ] Zero runtime registration or initialization overhead
- [ ] Build time and runtime performance maintained or improved