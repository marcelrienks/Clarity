# Testing

## Overview

Two testing approaches:
1. **Unity Tests**: Unit tests for core logic (101 tests, ~2 seconds)
2. **Wokwi Tests**: Hardware simulation integration tests (11 scenarios)

## Unity Tests

### Quick Start

```bash
# Run all tests (101 tests)
./run_unity_tests.sh        # Linux/Mac
run_unity_tests.bat         # Windows

# Or using PlatformIO directly
pio test -e test-all
```

### Run Individual Test Suites

```bash
# Sensor Tests (21 tests)
pio test -e test-sensors

# Manager Tests (15 tests)  
pio test -e test-managers

# Component Tests (24 tests)
pio test -e test-components

# Integration Tests (20 tests)
pio test -e test-integration

# Infrastructure Tests (21 tests)
pio test -e test-infrastructure
```

### Test Coverage (101 tests total)
- **Sensors (21)**: Oil pressure/temperature, key, lock, light sensors
- **Managers (15)**: Panel manager, trigger manager, style/preference services
- **Components (24)**: OEM oil gauges, key/lock indicators, Clarity branding
- **Integration (20)**: Scenario tests, system integration, end-to-end flows
- **Infrastructure (21)**: Device layer, factories, utilities, main app startup

Single file: `test/unity_tests.cpp` with hardware mocks.

## Wokwi Integration Tests

```bash
# Run all tests
./run_wokwi_tests.sh        # Linux/Mac
./run_wokwi_tests.bat       # Windows

# Run the basic startup test:
wokwi-cli test/wokwi --elf .pio/build/debug-local/firmware.elf --timeout 60000 --scenario basic_startup.test.yaml
```

### Test Scenarios

| Test | Purpose |
|------|---------|
| `basic_startup` | Boot and splash screen |
| `oil_panel_sensors` | Gauge animations |
| `theme_switching` | Day/night themes |
| `key_present` | Key panel (green) |
| `key_not_present` | Key panel (red) |
| `lock_panel` | Lock trigger |
| `trigger_priority` | Conflict resolution |
| `major_scenario` | Full integration |
| `startup_triggers_simple` | Simple trigger test |
| `night_startup` | Night theme on boot |

### GPIO Mapping

| Sensor | GPIO | Control |
|--------|------|---------|
| Key Present | 25 | DIP #1 |
| Key Not Present | 26 | DIP #2 |
| Lock | 27 | DIP #3 |
| Light | 33 | DIP #4 |
| Oil Pressure | 34 | pot1 |
| Oil Temperature | 35 | pot2 |

## Adding Tests

### Unity
Add functions to `test/unity_tests.cpp` using Unity macros.

### Wokwi
1. Create `.test.yaml` in `test/wokwi/`
2. Add to `run_wokwi_tests.sh` scenarios
3. Use: `set-control`, `wait-serial`, `delay`

## Troubleshooting

- **Token error**: Set `WOKWI_CLI_TOKEN`
- **Build fails**: Run `pio run -e debug-local`
- **Slow tests**: Check timeout values