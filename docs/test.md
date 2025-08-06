# Testing

## Overview

Two testing approaches:
1. **Unity Tests**: Unit tests for core logic (101 tests, ~2 seconds)
2. **Wokwi Tests**: Hardware simulation integration tests (10 scenarios)

## Unity Tests

```bash
./run_unity_tests.sh    # Linux/Mac
run_unity_tests.bat     # Windows
```

### Coverage (101 tests)
- Sensors (21)
- Managers (15)  
- Components (24)
- Integration (20)
- Infrastructure (21)

Single file: `test/unity_tests.cpp` with hardware mocks.

## Wokwi Integration Tests

```bash
./run_wokwi_tests.sh    # Requires WOKWI_CLI_TOKEN
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