# Testing Documentation

## Overview

Clarity uses two complementary testing approaches:

1. **Unity Tests**: Fast unit tests for core logic (101 tests)
2. **Wokwi Tests**: Hardware simulation integration tests (10 scenarios)

---

## Unity Unit Tests

### Quick Start
```bash
# Run all Unity tests
./run_unity_tests.sh
# or
./run_unity_tests.bat

# Individual command
pio test -e test-all
```

### Architecture
- **Framework**: Unity v2.6.0 
- **File**: `test/unity_tests.cpp` (single consolidated file)
- **Platform**: Native (no ESP32 hardware required)
- **Mocking**: Custom LVGL and hardware mocks

### Test Coverage (101 Tests)
- **Sensor Tests** (21): Oil pressure, temperature, key, lock, light sensors
- **Manager Tests** (15): Panel, trigger, style, preference managers  
- **Component Tests** (24): UI components, gauges, indicators
- **Integration Tests** (20): End-to-end scenarios, service container
- **Infrastructure Tests** (21): Device, factories, utilities, main app

### Key Features
- **Fast Execution**: ~2 seconds for all 101 tests
- **Mocked Dependencies**: No hardware requirements
- **Comprehensive Coverage**: All core logic paths tested
- **Cross-Platform**: Runs on Windows, Linux, macOS

---

## Wokwi Integration Tests

### Quick Start  
```bash
# Run all Wokwi integration tests
./run_wokwi_tests.sh

# Individual test
wokwi-cli test/wokwi --elf .pio/build/debug-local/firmware.elf --scenario <test-file.yaml>
```

### Prerequisites
- `WOKWI_CLI_TOKEN` environment variable
- `wokwi-cli` installed
- Firmware built (`pio run -e debug-local`)

### Test Scenarios (10 Comprehensive Tests)

| Test | Description | Coverage |
|------|-------------|----------|
| `basic_startup` | System boot and splash screen | Default startup flow |
| `oil_panel_sensors` | Sensor data and gauge animations | Real-time sensor updates |
| `theme_switching` | Day/night theme changes | Theme persistence |
| `night_startup` | Boot with night theme active | Startup theme handling |
| `key_present` | Key present trigger scenarios | Panel switching logic |
| `key_not_present` | Key not present trigger scenarios | Red icon validation |
| `lock_panel` | Lock trigger scenarios | Lock panel integration |
| `startup_triggers` | Multiple triggers at startup | Priority resolution |
| `trigger_priority` | Trigger conflict resolution | Priority handling |
| `major_scenario` | Complete 9-step system test | Full integration flow |

### Hardware Simulation
- **Platform**: Wokwi ESP32 simulator
- **Board**: NodeMCU-32S with 1.28" round display (GC9A01)
- **Controls**: DIP switches, potentiometers for trigger/sensor simulation
- **Output**: Serial logs, display screenshots, timing validation

### Test Configuration
- **Location**: `test/wokwi/`
- **Global Config**: Single `diagram.json` and `wokwi.toml` for all tests
- **Test Files**: Individual YAML scenario files
- **Results**: Saved to `test_results/` directory

---

## GPIO Pin Mapping (for Wokwi Tests)

| Component | GPIO | DIP Switch | Purpose |
|-----------|------|------------|---------|
| Key Present | 25 | #1 | Green key icon |
| Key Not Present | 26 | #2 | Red key icon |  
| Lock | 27 | #3 | Lock panel trigger |
| Lights | 33 | #4 | Day/night theme |
| Oil Pressure | 34 (VP) | pot1 | Analog pressure sensor |
| Oil Temperature | 35 (VN) | pot2 | Analog temperature sensor |

---

## Test Development

### Adding Unity Tests
1. Add test functions to `test/unity_tests.cpp`
2. Use Unity macros: `TEST_ASSERT_EQUAL()`, `TEST_ASSERT_TRUE()`, etc.
3. Include setup/teardown as needed
4. Run tests to verify

### Adding Wokwi Tests  
1. Create new `.test.yaml` file in `test/wokwi/`
2. Define test steps with `set-control`, `wait-serial`, `delay`
3. Add test to `run_wokwi_tests.sh` scenarios array
4. Test with individual wokwi-cli command first

### Test File Structure
```
test/
├── unity_tests.cpp           # All Unity tests
├── mocks/                    # Hardware/LVGL mocks
├── utilities/                # Test helpers
└── wokwi/                    # Wokwi integration tests
    ├── diagram.json          # Hardware configuration
    ├── wokwi.toml           # Simulation settings
    └── *.test.yaml          # Test scenarios
```

---

## Continuous Integration

### Local Development
1. Run Unity tests for code changes: `./run_unity_tests.sh`
2. Run Wokwi tests for integration validation: `./run_wokwi_tests.sh`  
3. Both test suites should pass before committing

### Automated Testing
- Unity tests run in CI without hardware dependencies
- Wokwi tests require token and may run in specialized CI environments
- Build verification happens before any test execution

---

## Troubleshooting

### Unity Tests
- **Build fails**: Check Unity framework installation and test environment
- **Tests fail**: Review mock implementations and test logic
- **Slow execution**: Tests should complete in ~2 seconds

### Wokwi Tests  
- **Token error**: Ensure `WOKWI_CLI_TOKEN` is set correctly
- **CLI not found**: Install wokwi-cli: `curl -L https://wokwi.com/ci/install.sh | sh`
- **Timeout issues**: Increase timeout values in YAML files
- **Firmware build fails**: Run `pio run -e debug-local` first

### Common Issues
- **Path problems**: Always run scripts from project root directory
- **Permission errors**: Ensure scripts are executable: `chmod +x *.sh`
- **Missing files**: Verify all test files exist and are properly referenced