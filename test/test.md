# Clarity Test Suite Documentation

This document describes how to run the comprehensive testing infrastructure implemented for the Clarity project, based on the requirements in `docs/todo.md`.

## **Quick Start - Run All Tests**

```bash
# Complete test suite with ALL tests (recommended for full validation)
python3 scripts/run_tests_with_coverage.py
```

This single command now runs the **complete comprehensive test suite** including:
- All unit tests with coverage instrumentation
- All integration tests and scenario validation  
- All performance tests (timing, optimization validation)
- All memory tests (leak detection, sanitizers)
- Generates HTML coverage reports with detailed metrics
- Validates coverage thresholds (85% line, 95% function, 80% branch)
- Creates comprehensive JSON test report

### **Faster Option (Essential Tests Only)**
```bash
# Skip optional performance and memory tests for faster execution
python3 scripts/run_tests_with_coverage.py --skip-performance --skip-memory
```

## **Test Categories**

### **1. Unit Tests**
Tests individual components in isolation using extensive mocks.

```bash
# Run only unit tests
python3 scripts/run_tests_with_coverage.py --unit-only

# Or use PlatformIO directly
pio test -e test-coverage
```

**Coverage**: Managers, Sensors, Components, System classes

### **2. Integration Tests**
Tests complete workflows and scenarios from `docs/todo.md`.

```bash
# Run only integration tests
python3 scripts/run_tests_with_coverage.py --integration-only

# Or use PlatformIO directly
pio test -e test-integration
```

**Coverage**: Full system scenarios, trigger interactions, panel workflows

### **3. Performance Tests**
Optimized builds for performance benchmarking.

```bash
# Run performance tests
pio test -e test-performance
```

**Coverage**: Timing behavior, stress testing, rapid state changes

### **4. Memory Tests**
Memory leak detection and stability testing.

```bash
# Run memory tests with sanitizers
pio test -e test-memory
```

**Coverage**: Memory leaks, buffer overflows, stability testing

## **PlatformIO Test Environments**

The project includes specialized test environments in `platformio.ini`:

| Environment | Purpose | Features |
|-------------|---------|----------|
| `test-coverage` | Unit tests with coverage | `--coverage`, `-fprofile-arcs`, debug symbols |
| `test-integration` | Integration scenarios | Full system testing, scenario validation |
| `test-performance` | Performance benchmarks | `-O2` optimization, timing measurements |
| `test-memory` | Memory validation | AddressSanitizer, LeakSanitizer |

## **Running Specific Tests**

### **By Test File**
```bash
# Run specific test files
pio test -e test-coverage -f test_preference_manager
pio test -e test-coverage -f test_key_sensor
pio test -e test-integration -f test_scenario_execution
```

### **By Pattern**
```bash
# Run all manager tests
pio test -e test-coverage --filter "*manager*"

# Run all sensor tests  
pio test -e test-coverage --filter "*sensor*"

# Run all integration scenarios
pio test -e test-integration --filter "*scenario*"
```

### **Individual Test Categories**
```bash
# Manager tests (PreferenceManager, StyleManager, etc.)
pio test -e test-coverage -f test_*_manager

# Sensor tests (KeySensor, LightSensor, etc.)
pio test -e test-coverage -f test_*_sensor

# Component tests (UI components)
pio test -e test-coverage -f test_*_component

# System tests (ServiceContainer, etc.)
pio test -e test-coverage -f test_*_system
```

### **Python Script Options**
```bash
# Run individual test categories
python3 scripts/run_tests_with_coverage.py --unit-only
python3 scripts/run_tests_with_coverage.py --integration-only
python3 scripts/run_tests_with_coverage.py --performance-only
python3 scripts/run_tests_with_coverage.py --memory-only

# Skip specific categories in full run
python3 scripts/run_tests_with_coverage.py --skip-performance
python3 scripts/run_tests_with_coverage.py --skip-memory
python3 scripts/run_tests_with_coverage.py --skip-performance --skip-memory

# Fast execution without coverage
python3 scripts/run_tests_with_coverage.py --no-coverage
```

## **Coverage Reports**

### **Generate Coverage**
```bash
# Full test suite with coverage
python3 scripts/run_tests_with_coverage.py

# Skip coverage for faster execution
python3 scripts/run_tests_with_coverage.py --no-coverage
```

### **View Coverage Reports**
After running tests with coverage:

- **HTML Report**: Open `coverage/html/index.html` in browser
- **JSON Report**: `coverage/test_report.json` (machine-readable)
- **LCOV File**: `coverage/coverage_filtered.info` (for CI tools)

### **Coverage Thresholds**
The test suite enforces these minimum coverage requirements:
- **Line Coverage**: 85%
- **Function Coverage**: 95%  
- **Branch Coverage**: 80%

Tests will fail if coverage drops below these thresholds.

## **Test Infrastructure**

### **Mock System**
Comprehensive mocks for embedded dependencies:
- **Arduino/ESP32**: Hardware state, GPIO, timers, interrupts
- **LVGL**: UI objects, styling, widgets, screen management
- **ArduinoJson**: JSON serialization/deserialization
- **Services**: All application interfaces (Panel, Style, Trigger, etc.)

### **Test Fixtures**
Reusable test setup classes in `test/utilities/test_fixtures.h`:
- **BaseTestFixture**: Basic service container setup
- **SensorTestFixture**: Hardware simulation utilities
- **ManagerTestFixture**: Manager testing tools
- **ComponentTestFixture**: LVGL component testing
- **IntegrationTestFixture**: Full scenario execution
- **PerformanceTestFixture**: Timing and memory measurement

### **Scenario Testing**
Complete implementation of scenarios from `docs/todo.md`:
- **Major Scenario**: Full system test with all trigger interactions
- **Individual Scenarios**: Key workflows, theme changes, startup conditions
- **Stress Testing**: Rapid trigger changes, performance validation

## **CI/CD Integration**

### **GitHub Actions**
The project includes `.github/workflows/comprehensive_testing.yml` with:
- Parallel unit and integration test execution
- Coverage report generation and validation
- Quality gate enforcement
- PR comments with coverage metrics
- Mutation testing (on main branch)

### **Local CI Simulation**
```bash
# Run the same tests as CI/CD
python3 scripts/run_tests_with_coverage.py

# Check exit code for pass/fail
echo $?  # 0 = success, 1 = failure
```

## **Troubleshooting**

### **Common Issues**

1. **Long Build Times**
   ```bash
   # Use faster debug build for testing
   pio test -e debug-local --target size
   ```

2. **Missing Dependencies**
   ```bash
   # Install coverage tools (Linux/macOS)
   sudo apt-get install lcov gcov  # Ubuntu/Debian
   brew install lcov               # macOS
   ```

3. **Python Path Issues**
   ```bash
   # Use full path if python3 not in PATH
   /usr/bin/python3 scripts/run_tests_with_coverage.py
   ```

### **Debug Test Failures**
```bash
# Run with verbose output
pio test -e test-coverage --verbose

# Run single test for debugging
pio test -e test-coverage -f test_preference_manager --verbose
```

### **Reset Test Environment**
```bash
# Clean build artifacts
pio run --target clean

# Remove coverage data
rm -rf coverage/
rm -rf .pio/build/test*/

# Rebuild and test
python3 scripts/run_tests_with_coverage.py
```

## **Test Statistics**

The comprehensive test suite includes:
- **100+ test cases** across all categories
- **15+ tests per manager** (PreferenceManager, StyleManager, etc.)
- **16+ tests per sensor** (KeySensor, LightSensor, etc.)
- **10+ integration scenarios** validating complete workflows
- **Full scenario coverage** from `docs/todo.md` requirements

## **Development Workflow**

### **Before Committing**
```bash
# Run full test suite
python3 scripts/run_tests_with_coverage.py

# Ensure all tests pass and coverage thresholds met
echo $?  # Must be 0
```

### **During Development**
```bash
# Quick unit test feedback
pio test -e test-coverage -f test_your_component

# Test specific functionality
python3 scripts/run_tests_with_coverage.py --unit-only
```

### **Before Release**
```bash
# Full comprehensive validation
python3 scripts/run_tests_with_coverage.py

# Performance validation
pio test -e test-performance

# Memory validation
pio test -e test-memory
```

## **Architecture Validation**

The test suite confirms the excellent testability of the Clarity architecture:
- ✅ **MVP Pattern**: Clean separation enables comprehensive mocking
- ✅ **Dependency Injection**: Service container supports easy test setup
- ✅ **Interface Abstractions**: Hardware dependencies fully mockable
- ✅ **Service Architecture**: All services independently testable

This comprehensive testing infrastructure ensures high code quality and enables confident development of the Clarity digital gauge system.