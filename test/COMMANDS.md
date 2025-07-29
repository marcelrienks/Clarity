# Test Command Reference

Quick reference for all test execution commands and options.

## Primary Test Commands

### PlatformIO (Recommended)

```bash
# Run all tests (recommended)
pio test -e test

# Run with verbose output
pio test -e test -v

# Run specific test file
pio test -e test --filter test_trigger_system
pio test -e test --filter test_panel_manager
pio test -e test --filter test_sensors
pio test -e test --filter test_scenarios_integration
```

### Shell Scripts

```bash
# From project root
cd test && ./run_tests.sh

# From test directory
./run_tests.sh              # Complete test suite
./run_quick_tests.sh         # Quick development tests
```

## Test Execution Modes

### Complete Test Suite
```bash
# All tests including integration and performance
pio test -e test
./run_tests.sh
```
**Includes:**
- Unit tests (trigger system, panel manager, sensors)
- Integration scenarios (all S1-S5 scenarios)
- Performance and stress tests
- Memory usage validation

### Quick Development Tests
```bash
# Fast subset for development workflow
./run_quick_tests.sh
```
**Includes:**
- Essential trigger system tests
- Basic functionality validation
- Fast compilation and execution

### Individual Test Categories

#### Trigger System Tests Only
```bash
pio test -e test --filter test_trigger_system
```
**Coverage:**
- All scenarios S1.1-S5.3 from docs/scenarios.md
- GPIO trigger logic
- Priority and FIFO behavior
- State restoration chains

#### Panel Manager Tests Only
```bash
pio test -e test --filter test_panel_manager
```
**Coverage:**
- Panel lifecycle management
- Panel switching and cleanup
- Memory management
- Integration with trigger system

#### Sensor Tests Only
```bash
pio test -e test --filter test_sensors
```
**Coverage:**
- Oil pressure/temperature sensors
- ADC reading and conversion
- Timing and performance
- Error handling

#### Integration Tests Only
```bash
pio test -e test --filter test_scenarios_integration
```
**Coverage:**
- End-to-end system scenarios
- Complete trigger → panel → sensor integration
- Long-running stability tests
- System recovery scenarios

## Scenario-Specific Commands

### Startup Scenarios (S1.1-S1.5)
```bash
# Custom test runner for startup scenarios only
# Add to test_main.cpp and compile:
g++ -std=c++17 -I. -I../include -DUNIT_TESTING \
    test_utilities.cpp test_trigger_system.cpp test_main.cpp \
    -DSTARTUP_TESTS_ONLY -lunity -o ../build/startup_tests && \
    ../build/startup_tests
```

### Multi-Trigger Scenarios (S3.1-S3.5)
```bash
# Test complex trigger interactions
pio test -e test --filter test_trigger_system -v | grep "S3"
```

### Edge Case Scenarios (S4.1-S4.5)
```bash
# Test boundary conditions and error handling
pio test -e test --filter test_trigger_system -v | grep "S4"
```

### Performance Scenarios (S5.1-S5.3)
```bash
# Test system performance under load
pio test -e test -v | grep "Performance\|S5"
```

## Debug and Diagnostic Commands

### Verbose Output
```bash
# Detailed test execution logs
pio test -e test -v

# With Unity verbose output
pio test -e test -v --verbose
```

### Memory and Performance Analysis
```bash
# Run with memory tracking
valgrind --leak-check=full ./build/test/clarity_tests

# Performance profiling
gprof ./build/test/clarity_tests gmon.out > performance_report.txt
```

### Debug Compilation
```bash
# Compile with debug symbols
g++ -std=c++17 -g3 -O0 -I. -I../include -DUNIT_TESTING \
    test_utilities.cpp test_trigger_system.cpp test_main.cpp \
    -lunity -o ../build/debug_tests

# Run with GDB
gdb ../build/debug_tests
```

## Development Workflow Commands

### Test-Driven Development (TDD)
```bash
# 1. Quick test during development
./run_quick_tests.sh

# 2. Test specific component after changes
pio test -e test --filter test_trigger_system

# 3. Full validation before commit
pio test -e test
```

### Continuous Integration
```bash
# Pre-commit validation
pio test -e test --filter test_trigger_system
if [ $? -eq 0 ]; then git commit; fi

# Build verification
pio test -e test && echo "All tests passed"
```

### Performance Monitoring
```bash
# Baseline performance measurement
time pio test -e test > performance_baseline.log 2>&1

# Compare performance
time pio test -e test > performance_current.log 2>&1
diff performance_baseline.log performance_current.log
```

## Compilation Commands

### Manual Compilation (Alternative to PlatformIO)

#### Complete Test Suite
```bash
mkdir -p ../build/test
g++ -std=c++17 -I. -I../include -DUNIT_TESTING \
    test_utilities.cpp \
    test_trigger_system.cpp \
    test_panel_manager.cpp \
    test_sensors.cpp \
    test_scenarios_integration.cpp \
    test_main.cpp \
    -lunity -o ../build/test/clarity_tests && \
    ../build/test/clarity_tests
```

#### Quick Development Build
```bash
mkdir -p ../build
g++ -std=c++17 -I. -I../include -DUNIT_TESTING \
    test_utilities.cpp test_trigger_system.cpp test_main.cpp \
    -lunity -o ../build/quick_tests && \
    ../build/quick_tests
```

#### Individual Test Files
```bash
# Trigger system only
g++ -std=c++17 -I. -I../include -DUNIT_TESTING \
    test_utilities.cpp test_trigger_system.cpp \
    -lunity -o ../build/trigger_tests

# Panel manager only
g++ -std=c++17 -I. -I../include -DUNIT_TESTING \
    test_utilities.cpp test_panel_manager.cpp \
    -lunity -o ../build/panel_tests

# Sensors only
g++ -std=c++17 -I. -I../include -DUNIT_TESTING \
    test_utilities.cpp test_sensors.cpp \
    -lunity -o ../build/sensor_tests
```

## Environment Setup Commands

### Ubuntu/Debian
```bash
# Install dependencies
sudo apt-get update
sudo apt-get install build-essential libunity-dev

# Install PlatformIO
pip3 install platformio

# Verify installation
pio --version
g++ --version
pkg-config --modversion unity
```

### macOS
```bash
# Install dependencies
brew install unity

# Install PlatformIO
pip3 install platformio

# Verify installation
pio --version
g++ --version
```

### Windows (WSL)
```bash
# Install dependencies in WSL
sudo apt-get update
sudo apt-get install build-essential libunity-dev

# Install PlatformIO
pip3 install platformio

# Run tests
cd /mnt/c/path/to/clarity/test
./run_tests.sh
```

## Troubleshooting Commands

### Unity Framework Issues
```bash
# Check Unity installation
pkg-config --modversion unity
pkg-config --cflags unity
pkg-config --libs unity

# Manual Unity compilation if needed
git clone https://github.com/ThrowTheSwitch/Unity.git
cd Unity && make
```

### Compilation Issues
```bash
# Check compiler version
g++ --version  # Should be 7.0+ for C++17

# Verbose compilation
g++ -v -std=c++17 -I. -I../include -DUNIT_TESTING \
    test_utilities.cpp test_main.cpp -lunity -o test_debug

# Check include paths
echo | g++ -E -Wp,-v -
```

### Runtime Issues
```bash
# Check library paths
ldd ../build/test/clarity_tests

# Run with detailed error reporting
LD_DEBUG=libs ../build/test/clarity_tests

# Memory leak detection
valgrind --leak-check=full --show-leak-kinds=all \
    ../build/test/clarity_tests
```

## Performance Benchmarking Commands

### Timing Analysis
```bash
# Basic timing
time pio test -e test

# Detailed timing breakdown
/usr/bin/time -v pio test -e test

# Multiple runs for average
for i in {1..5}; do 
    echo "Run $i:"
    time pio test -e test 2>&1 | grep real
done
```

### Memory Analysis
```bash
# Memory usage during testing
/usr/bin/time -v pio test -e test 2>&1 | grep -E "(Maximum|Average)"

# Detailed memory profiling
valgrind --tool=massif pio test -e test
ms_print massif.out.* > memory_profile.txt
```

### System Resource Monitoring
```bash
# Monitor during test execution
top -p $(pgrep clarity_tests) &
pio test -e test
killall top

# IO monitoring
iotop -p $(pgrep clarity_tests) &
pio test -e test
```

## CI/CD Integration Commands

### GitHub Actions
```yaml
# .github/workflows/test.yml
- name: Run Tests
  run: |
    cd test
    pio test -e test
    ./run_tests.sh
```

### Docker Integration
```bash
# Build test container
docker build -t clarity-tests -f test/Dockerfile .

# Run tests in container
docker run --rm clarity-tests pio test -e test
```

### Pre-commit Hooks
```bash
# Install pre-commit hook
cat > .git/hooks/pre-commit << 'EOF'
#!/bin/bash
cd test && pio test -e test --filter test_trigger_system
EOF
chmod +x .git/hooks/pre-commit
```

## Custom Test Configurations

### Environment Variables
```bash
# Enable verbose Unity output
export UNITY_VERBOSE=1
pio test -e test

# Custom test timeout
export TEST_TIMEOUT=60
pio test -e test

# Memory debugging
export MALLOC_CHECK_=2
./run_tests.sh
```

### Compiler Flags
```bash
# Debug build
export CXXFLAGS="-g3 -O0 -DDEBUG"
./run_tests.sh

# Optimized build
export CXXFLAGS="-O2 -DNDEBUG"
./run_tests.sh

# Static analysis
export CXXFLAGS="-Wall -Wextra -Werror"
./run_tests.sh
```

### Test Filtering
```bash
# Run only startup tests
pio test -e test --filter "*startup*"

# Run only performance tests
pio test -e test --filter "*performance*"

# Exclude integration tests
pio test -e test --filter "*" --ignore "*integration*"
```

## Summary of Key Commands

| Purpose | Command | Notes |
|---------|---------|--------|
| **All Tests** | `pio test -e test` | Recommended primary command |
| **Quick Tests** | `./run_quick_tests.sh` | Fast development workflow |
| **Trigger Tests** | `pio test -e test --filter test_trigger_system` | Scenario-based testing |
| **Debug Mode** | `pio test -e test -v` | Verbose output |
| **Manual Build** | `./run_tests.sh` | Alternative to PlatformIO |
| **Performance** | `time pio test -e test` | Timing measurement |
| **Memory Check** | `valgrind ./build/test/clarity_tests` | Memory leak detection |

For detailed test coverage and methodology, see `README.md` and `TESTING.md` in this directory.