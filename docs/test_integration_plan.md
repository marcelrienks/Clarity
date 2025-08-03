# Test Suite Integration Plan

## Current Status

- ✅ **58 tests running successfully** with 100% pass rate
- ✅ **276+ total test functions exist** across all test files
- ❌ **Full test suite integration blocked** by architectural design conflicts

## Problem Statement

The project has evolved a monolithic `test_all.cpp` approach that works for a subset of tests, but including all test files creates symbol conflicts due to:
- Duplicate function names across test files
- Multiple `main()` functions
- Global variable redefinitions
- Include path conflicts

## Integration Plan

### Step 1: Refactor Tests to Avoid Symbol Conflicts

**Objective:** Eliminate duplicate symbols across test files

**Tasks:**
- [ ] Audit all test files for duplicate function names
- [ ] Rename conflicting test functions with unique prefixes
  - `test_timing_calculation()` → `test_ticker_timing_calculation()`, `test_simple_ticker_timing_calculation()`
  - `setUp()` → `setUp_<module_name>()`
  - `tearDown()` → `tearDown_<module_name>()`
- [ ] Move global variables into namespaces or make them static
- [ ] Standardize test function naming convention: `test_<module>_<functionality>()`
- [ ] Create unique mock object instances per test file
  - `MockGpioProvider* mockGpio` → `MockGpioProvider* <module>MockGpio`

**Estimated Effort:** 2-3 hours

### Step 2: Implement Different Test Runner Approach

**Objective:** Replace monolithic test_all.cpp with modular test execution

**Option A: Unity Test Discovery**
- [ ] Remove `test_all.cpp` completely
- [ ] Configure PlatformIO to auto-discover test functions
- [ ] Use Unity's `RUN_TEST()` macro in each individual test file
- [ ] Create per-module test runners with `main()` functions

**Option B: Centralized Test Registry**
- [ ] Create `test_registry.h` with function pointer registration
- [ ] Each test file registers its tests in the registry
- [ ] Single `test_main.cpp` iterates through registered tests
- [ ] Avoids multiple `main()` functions

**Option C: Separate Test Environments**
- [ ] Create multiple test environments in `platformio.ini`
- [ ] Group related tests into separate environments
- [ ] Run test suites independently: `sensors`, `managers`, `providers`, etc.

**Recommended:** Option C for immediate implementation, Option B for long-term

**Estimated Effort:** 3-4 hours

### Step 3: Rename Duplicate Functions Across Test Files

**Objective:** Ensure all function names are globally unique

**Current Conflicts to Resolve:**
```
Function Name                    Files with Conflicts
=====================================
test_timing_calculation()        test_all.cpp, test_simple_ticker.cpp
setUp()                         test_all.cpp, test_suite_organized.cpp  
tearDown()                      test_all.cpp, test_suite_organized.cpp
main()                          test_all.cpp, test_execution_plan.cpp, test_suite_organized.cpp
set_mock_millis()               test_common.h, test_simple_ticker.cpp
handleDynamicDelay()            test_common.h, test_simple_ticker.cpp
```

**Naming Convention:**
```cpp
// Before
void test_timing_calculation() { }

// After  
void test_<module>_timing_calculation() { }
void test_ticker_timing_calculation() { }
void test_simple_ticker_timing_calculation() { }
```

**Tasks:**
- [ ] Create conflict resolution spreadsheet
- [ ] Systematically rename functions file by file
- [ ] Update all `RUN_TEST()` calls to match new names
- [ ] Verify no remaining conflicts with global symbol search

**Estimated Effort:** 2-3 hours

### Step 4: Create Separate Test Environments

**Objective:** Isolate test suites to prevent linking conflicts

**New PlatformIO Test Environments:**
```ini
[env:test-sensors]
extends = env:test
build_src_filter = 
    +<*>
    +<../test/unit/sensors>
    +<../test/mocks>
    -<../test/unit/managers>
    -<../test/unit/providers>
    # ... other exclusions

[env:test-managers] 
extends = env:test
build_src_filter = 
    +<*>
    +<../test/unit/managers>
    +<../test/mocks>
    -<../test/unit/sensors>
    -<../test/unit/providers>
    # ... other exclusions

[env:test-providers]
extends = env:test
build_src_filter = 
    +<*>
    +<../test/unit/providers>
    +<../test/mocks>
    # ... other exclusions

[env:test-all-unified]
extends = env:test
# Only after Steps 1-3 are complete
build_src_filter = +<*> +<../test/unit>
```

**Test Execution Commands:**
```bash
# Run individual test suites
pio test -e test-sensors
pio test -e test-managers  
pio test -e test-providers

# Run all tests (after integration)
pio test -e test-all-unified
```

**Tasks:**
- [ ] Define test environment configurations
- [ ] Create test suite groupings by functionality
- [ ] Test each environment independently
- [ ] Create CI/CD pipeline to run all environments
- [ ] Document test execution workflow

**Estimated Effort:** 1-2 hours

## Implementation Priority

1. **Immediate (High Impact, Low Effort):** Step 4 - Separate Test Environments
2. **Short Term:** Step 3 - Rename Duplicate Functions  
3. **Medium Term:** Step 1 - Refactor Symbol Conflicts
4. **Long Term:** Step 2 - New Test Runner Approach

## Success Criteria

- [ ] All 276+ test functions execute successfully
- [ ] 100% pass rate maintained across all test suites
- [ ] No linking or compilation errors
- [ ] Test execution time under 2 minutes total
- [ ] Clear separation of test concerns
- [ ] Maintainable test architecture for future additions

## Risk Mitigation

- **Backup Strategy:** Keep current working `test_all.cpp` as fallback
- **Incremental Approach:** Implement changes in separate branches
- **Validation:** Verify test coverage doesn't decrease during refactoring
- **Documentation:** Update test documentation and developer guides

## Estimated Total Effort

**8-12 hours** of development work spread across multiple sessions.

---

**Status:** Planning Phase  
**Next Action:** Begin with Step 4 (Separate Test Environments) for immediate wins  
**Owner:** Development Team  
**Target Completion:** TBD based on development priorities