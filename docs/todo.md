# TODO:
## 1. Test Coverage Improvement Plan

**Status**: ✅ **95 tests implemented and passing**, ⚠️ **26.80% overall coverage** - needs improvement

### Current Test Status
- **95 unit tests** all passing in ~15 seconds
- **Comprehensive mock infrastructure** for embedded dependencies
- **Coverage reporting** with gcov integration
- **Streamlined test environment** (`pio test -e test`)

### Coverage Analysis

**✅ Perfect Coverage (100%)**:
- All sensor implementations (key, light, lock, oil pressure/temperature)
- Ticker utility component
- **Action**: Maintain existing high-quality coverage

**⚠️ Good Coverage (77-93%)**:
- **PreferenceManager**: 77.55% - config persistence and JSON handling
- **StyleManager**: 93.06% - theme management and LVGL styling
- **Action**: Enhance to 95%+ coverage

**❌ Zero Coverage (0%)**:
- **PanelManager**: Core UI management (tests exist but excluded)
- **TriggerManager**: Event handling system (tests exist but excluded)
- **ServiceContainer**: Dependency injection system (tests exist but excluded)
- **Action**: Enable existing tests and fix interface issues

### Implementation Phases

#### **Phase 1: Enable Existing Tests (Quick Wins)**
**Target**: 50-60% overall coverage in 1-2 days

1. **Re-enable PanelManager Tests**
   - Remove exclusion from `platformio.ini`
   - Fix any interface mismatches
   - Expected gain: +15-20% coverage

2. **Re-enable TriggerManager Tests**  
   - Remove exclusion from `platformio.ini`
   - Address interface compatibility issues
   - Expected gain: +10-15% coverage

3. **Re-enable ServiceContainer Tests**
   - Remove exclusion from `platformio.ini`
   - Fix dependency injection interface mismatches
   - Expected gain: +5-10% coverage

#### **Phase 2: Improve Moderate Coverage (Enhancement)**
**Target**: 70-80% overall coverage in 1 week

1. **Enhance PreferenceManager Coverage (77.55% → 95%+)**
   - Add error injection tests for NVS failures
   - Test malformed JSON configuration handling
   - Add boundary value tests for configuration limits
   - Test configuration migration scenarios
   - Expected gain: +12-15%

2. **Enhance StyleManager Coverage (93.06% → 98%+)**
   - Add tests for LVGL allocation failures
   - Test rapid theme switching scenarios
   - Add cleanup and memory management tests
   - Expected gain: +5-7%

#### **Phase 3: Add Missing Component Coverage**
**Target**: 85-95% overall coverage in 2-3 weeks

1. **Add Provider Tests**
   - Include LvglDisplayProvider in build
   - Create comprehensive provider test suites
   - Mock additional hardware dependencies
   - Expected gain: +10-15%

2. **Add Factory Tests**
   - Include ManagerFactory and UIFactory in build
   - Create factory instantiation tests
   - Test dependency injection scenarios
   - Expected gain: +5-10%

3. **Add Panel and Component Tests**
   - Include panel implementations (Key, Lock, Splash, OemOil)
   - Include UI components (Clarity, Key, Lock, OEM oil)
   - Create comprehensive UI interaction tests
   - Expected gain: +15-25%

### Immediate Action Items

```ini
# Edit platformio.ini - Remove these exclusions:
# -<../test/unit/managers/test_trigger_manager.cpp>
# -<../test/unit/managers/test_config_logic.cpp>
# -<../test/unit/system/test_service_container.cpp>
```

```bash
# Run tests to identify issues
pio test -e test -v

# Generate coverage report
pio test -e test && gcov src/*/*.gcda
```

### Success Metrics

| Phase | Target Coverage | Components | Timeline |
|-------|----------------|------------|----------|
| **Current** | 26.80% | Working sensors + utilities | ✅ Complete |
| **Phase 1** | 50-60% | + Manager tests enabled | 1-2 days |
| **Phase 2** | 70-80% | + Enhanced coverage | 1 week |
| **Phase 3** | 85-95% | + All components | 2-3 weeks |

---

## 2. Architecture Documentation

**Status**: ✅ **Implemented** - `docs/architecture.md` created and linked

The architecture documentation has been completed with comprehensive coverage of:

### Completed Sections
- **MVP Pattern Implementation** - Detailed relationships and responsibilities
- **Layered Architecture** - Device → Display → Panels → Components structure  
- **Service Architecture** - Dependency injection and lifecycle management
- **Hardware Abstraction** - GPIO, display, and sensor interfaces
- **Dynamic Panel Switching** - Trigger-based navigation logic
- **Configuration Management** - Persistent settings and JSON serialization

### Architecture Validation
✅ **Confirmed**: The codebase architecture excellently supports comprehensive testing through:
- **Dependency injection** pattern enabling easy mocking
- **Interface abstractions** decoupling hardware dependencies  
- **Service container** managing component lifecycle
- **MVP pattern** separating business logic from UI concerns

The architecture design directly enables the high test coverage goals outlined in Section 1.

---

## 3. Scenario Documentation

**Status**: ✅ **Implemented** - `docs/scenario.md` created and linked

The scenario documentation has been completed with comprehensive test scenarios:

### Major Scenario (Full System Test)

> App starts with pressure and temperature values set to halfway  
> → Splash animates with day theme (white text)  
> → Oil panel loads with day theme (white scale ticks and icon)  
> → Oil panel needles animate  
> → Lights trigger high  
> → Oil panel does NOT reload, theme changes to night (red scale ticks and icon)  
> → Lock trigger high  
> → Lock panel loads  
> → Key not present trigger high  
> → Key panel loads (present = false → red icon)  
> → Key present trigger high  
> → Lock panel loads (both key present and key not present true = invalid state)  
> → Key not present trigger low  
> → Key panel loads (present = true → green icon, key present trigger still high)  
> → Key present trigger low  
> → Lock panel loads (lock trigger still high)  
> → Lock trigger low  
> → Oil panel loads with night theme (red scale ticks and icon, lights trigger still active)  
> → Oil panel needles animate  
> → Lights trigger low  
> → Oil panel does NOT reload, theme changes to day (white scale ticks and icon)

### Implementation Notes

**Scenario Test Integration**: These scenarios can be validated through:
- **Unit tests** for individual component behavior
- **Integration tests** for trigger sequences (Section 4)
- **Wokwi simulations** for end-to-end validation (Section 5)

**Success Criteria Defined**: Each scenario step includes specific validation criteria:
- Component loading confirmation
- Theme application verification  
- Animation completion checking
- State transition validation

The scenario documentation provides the foundation for comprehensive integration testing outlined in the coverage improvement plan.

---

## 4. Integration Tests

Create integration tests to validate full scenario flows.  
> Is it possible to do so to prove scenarios?

---

## 5. Wokwi Tests

Create Wokwi-based simulations for the documented scenarios.

## 6. General

* review and remove all null checks before use, if an object is null, the application should fail, not just continue with the error
* update scenarios, to detail what the success criteria is for something like 'oil panel load'
It would be that both oil components are loaded, that needles animate, that scales are visible, that the correct theme is applied etc.