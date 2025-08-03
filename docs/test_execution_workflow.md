# Test Execution Workflow

## Overview

This document provides comprehensive guidance for running tests in the Clarity project, covering local development, CI/CD pipeline execution, and troubleshooting.

## Current Test Status

### Test Suite Metrics

| Metric | Value | Status |
|--------|-------|--------|
| **Active Tests** | 57/58 | ✅ 98.3% Pass Rate |
| **Symbol Conflicts** | 0 | ✅ Resolved |
| **Commented Tests** | 219+ | 🔄 Available for Integration |
| **Total Test Functions** | 276+ | 🎯 Discovery Complete |
| **Test Environments** | 8 configured | ✅ Available |

### Test Categories Covered

- ✅ **Sensor Logic Tests** (key, lock, light, oil pressure, oil temperature)
- ✅ **Manager Tests** (preference, trigger, panel, style)  
- ✅ **Provider Tests** (GPIO, LVGL display)
- ✅ **Factory Tests** (manager factory, UI factory)
- ✅ **Utility Tests** (ticker, simple ticker)
- ✅ **System Tests** (service container)
- ✅ **Performance Benchmarks** (1 expected failure)

## Local Test Execution

### Quick Test Commands

```bash
# Run main test suite (recommended)
pio test -e test

# Run with verbose output
pio test -e test --verbose

# Clean build and test
rm -rf .pio/build/test && pio test -e test

# Check available environments
pio test --list-tests
```

### Build-Only Verification

```bash
# Test environment configurations without execution
pio run -e test-sensors --target compiledb
pio run -e test-managers --target compiledb  
pio run -e test-providers --target compiledb
pio run -e test-factories --target compiledb
pio run -e test-utilities --target compiledb
pio run -e test-components --target compiledb
pio run -e test-panels --target compiledb
```

### Expected Results

**Successful Test Run Output:**
```
================== 58 test cases: 57 succeeded in 00:00:XX.XXX ==================
```

**Expected Failure:**
- `test_*_performance_benchmark` (1 test) - Performance benchmarks may fail based on system resources

## CI/CD Pipeline

### Workflow Triggers

- **Push to main/develop branches**
- **Pull requests to main branch**  
- **Manual workflow dispatch**

### Pipeline Jobs

#### 1. Main Test Suite
- **Purpose:** Validate core 57/58 test functionality
- **Environment:** `test` 
- **Expected:** 98.3% pass rate
- **Duration:** ~30 seconds

#### 2. Modular Environment Validation  
- **Purpose:** Verify all 7 modular environments can compile
- **Environments:** `test-sensors`, `test-managers`, `test-providers`, `test-factories`, `test-utilities`, `test-components`, `test-panels`
- **Expected:** Configuration validation (compilation only)
- **Duration:** ~2 minutes (parallel execution)

#### 3. Integration Tests
- **Purpose:** End-to-end scenario testing
- **Environment:** Custom integration setup
- **Duration:** ~1 minute

#### 4. Comprehensive Coverage
- **Purpose:** Combined test execution with coverage reporting
- **Dependencies:** Main tests + Integration tests
- **Output:** Coverage artifacts, badges, PR comments
- **Duration:** ~45 seconds

#### 5. Quality Gate
- **Purpose:** Enforce quality thresholds
- **Criteria:**
  - Line coverage ≥ 85%
  - Function coverage ≥ 95%  
  - Branch coverage ≥ 80%
  - Main tests pass
  - Integration tests pass

### Artifacts Generated

| Artifact | Description | Retention |
|----------|-------------|-----------|
| `main-test-results` | Core test execution logs | 30 days |
| `modular-environment-status-*` | Environment validation status | 7 days |
| `integration-test-results` | Integration test outputs | 30 days |
| `coverage-report` | Coverage analysis and HTML reports | 90 days |
| `mutation-testing-report` | Mutation testing results | 30 days |

## Troubleshooting

### Common Issues

#### 1. Symbol Conflicts (Should Be Resolved)

**Symptoms:**
```
multiple definition of `function_name`
```

**Resolution:**
Symbol conflicts were resolved in Step 1. If they reoccur:
- Check for duplicate function names
- Ensure static variables are properly scoped
- Verify function prefixes follow module naming conventions

#### 2. Missing Dependencies

**Symptoms:**
```
undefined reference to `ClassName::method`
```

**Resolution:**
```bash
# Clean and rebuild
rm -rf .pio/build/test
pio test -e test
```

#### 3. Test Discovery Issues

**Symptoms:**
```
Error: Nothing to build. Please put your test suites to the 'test' folder
```

**Resolution:**
- Verify `test_all.cpp` exists in `/test` directory
- Check `platformio.ini` test environment configuration
- Ensure `test_build_src = yes` is set

#### 4. Performance Test Failures

**Symptoms:**
Performance benchmark tests failing on slower systems.

**Resolution:**
This is expected behavior. Performance tests are system-dependent and may fail on resource-constrained CI runners.

### Debugging Test Failures

#### Enable Verbose Output
```bash
pio test -e test --verbose
```

#### Check Build Logs
```bash
pio run -e test --verbose
```

#### Examine Test Details
```bash
# View specific test file
cat test/test_all.cpp | grep -A 10 "failing_test_name"
```

## Advanced Testing

### Custom Test Execution

#### Run Specific Test Groups
By modifying `test_all.cpp`, you can selectively run test groups:

```cpp
// In test_all.cpp main function
int main(int argc, char **argv) {
    UNITY_BEGIN();
    
    // Run only sensor tests
    runKeySensorTests();
    runLockSensorTests();
    runLightSensorTests();
    runOilPressureSensorTests();
    runOilTemperatureSensorTests();
    
    return UNITY_END();
}
```

#### Performance Profiling
```bash
# Build with profiling
pio run -e test --verbose

# Analyze performance
# (Custom profiling scripts can be added)
```

### Integration with Development Workflow

#### Pre-commit Testing
```bash
# Add to pre-commit hook
pio test -e test
if [ $? -ne 0 ]; then
    echo "Tests failed. Commit aborted."
    exit 1
fi
```

#### IDE Integration
Most IDEs support PlatformIO test execution:
- **VSCode:** PlatformIO extension with test runner
- **CLion:** CMake integration with custom test targets

## Future Enhancements

### Planned Improvements

1. **Test Integration Pipeline**
   - Gradual integration of 219+ commented tests
   - Systematic resolution of remaining conflicts
   - Target: 276+ tests with 100% pass rate

2. **Enhanced Coverage**
   - Line coverage target: 85% → 90%
   - Branch coverage improvements
   - Function coverage optimization

3. **Performance Testing**
   - Baseline establishment for performance benchmarks
   - Regression detection automation
   - Memory usage monitoring

4. **Modular Testing Evolution**
   - Research alternative approaches for true modular testing
   - Docker-based test isolation
   - Custom build script development

## Quick Reference

### Essential Commands
```bash
# Standard test execution
pio test -e test

# Environment validation
pio run -e test-sensors --target compiledb

# Clean rebuild
rm -rf .pio/build && pio test -e test

# List all test environments
pio test --list-tests
```

### File Locations
- **Main test file:** `/test/test_all.cpp`
- **Test configuration:** `/platformio.ini` (lines 40-411)
- **Test documentation:** `/docs/` (test integration files)
- **CI/CD workflow:** `/.github/workflows/comprehensive_testing.yml`

### Key Metrics to Monitor
- Test pass rate: Should maintain ≥57/58 (98.3%)
- Build time: Should remain under 30 seconds
- Coverage: Line coverage ≥85%
- No symbol conflicts in build logs

This workflow ensures reliable, maintainable testing while providing a clear path toward integrating all 276+ discovered tests.