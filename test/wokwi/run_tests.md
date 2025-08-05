# Wokwi Test Execution Guide

This guide provides step-by-step instructions for executing the complete Wokwi integration test suite.

## Pre-Test Setup

### 1. Build Debug Firmware
```bash
cd /path/to/Clarity
pio run -e debug-local
```
Verify the build completes successfully and generates:
- `firmware.bin` in `.pio/build/debug-local/`
- `firmware.elf` in `.pio/build/debug-local/`

### 2. Prepare Test Environment
- Ensure Wokwi access (browser or VS Code extension)
- Have serial monitor capability ready
- Prepare test tracking sheet (optional)

## Test Execution Sequence

### Phase 1: Basic Functionality Tests

#### Test 1: Basic System Startup
**Directory**: `test/wokwi/basic_startup/`
**Duration**: ~5 minutes
**Focus**: System initialization and default behavior

1. Open `diagram.json` in Wokwi (uses shared `wokwi.toml` from parent directory)
2. Start simulation
3. Monitor serial output for initialization logs
4. Verify splash screen animation (white text)
5. Confirm transition to oil panel
6. Check gauge positions (should be at minimum)

**Success Criteria**: ✅ All initialization logs present, smooth animations, clean panel transition

#### Test 2: Oil Panel with Sensors  
**Directory**: `test/wokwi/oil_panel_sensors/`
**Duration**: ~7 minutes
**Focus**: Sensor integration and gauge animations

1. Open `diagram.json` in Wokwi (pots at halfway)
2. Start simulation and observe gauge animations
3. Interactive test: Adjust Pot1 (pressure) during runtime
4. Interactive test: Adjust Pot2 (temperature) during runtime
5. Verify needle responses are smooth and accurate

**Success Criteria**: ✅ Needles animate to ~5 Bar/~60°C, real-time pot adjustments work

### Phase 2: Theme and Panel Switching

#### Test 3: Theme Switching
**Directory**: `test/wokwi/theme_switching/`  
**Duration**: ~5 minutes
**Focus**: Day/night theme without panel reload

1. Wait for oil panel to fully load (day theme - white)
2. Toggle DIP Switch #4 ON → Verify night theme (red) with NO panel reload
3. Toggle DIP Switch #4 OFF → Verify day theme (white) restoration
4. Repeat several times to confirm consistency

**Success Criteria**: ✅ Instant color changes, no panel interruption, theme persistence

#### Test 4: Key Present Panel
**Directory**: `test/wokwi/key_present/`
**Duration**: ~5 minutes  
**Focus**: Key present trigger and green icon

1. Wait for oil panel load, note gauge positions
2. Toggle DIP Switch #1 ON → Verify switch to Key panel with GREEN icon
3. Toggle DIP Switch #1 OFF → Verify return to Oil panel
4. Confirm gauge values restored correctly

**Success Criteria**: ✅ Clean panel transitions, green key icon, oil panel restoration

#### Test 5: Key Not Present Panel
**Directory**: `test/wokwi/key_not_present/`
**Duration**: ~5 minutes
**Focus**: Key not present trigger and red icon

1. Wait for oil panel load with low sensor readings
2. Toggle DIP Switch #2 ON → Verify switch to Key panel with RED icon  
3. Toggle DIP Switch #2 OFF → Verify return to Oil panel
4. Confirm red icon clearly distinguishable from green

**Success Criteria**: ✅ Clean panel transitions, red key icon, clear color distinction

### Phase 3: Complete System Integration

#### Test 6: Major Scenario (Complete Integration)
**Directory**: `test/wokwi/major_scenario/`
**Duration**: ~15 minutes
**Focus**: All triggers, priorities, and system integration

**Critical Test - Follow exact sequence:**

1. **Setup**: Wait for oil panel with halfway sensor readings
2. **Step 1**: DIP #4 ON → Night theme (red), no panel reload
3. **Step 2**: DIP #3 ON → Lock panel loads  
4. **Step 3**: DIP #2 ON → Key panel (red icon)
5. **Step 4**: DIP #1 ON → Lock panel (conflict resolution)
6. **Step 5**: DIP #2 OFF → Key panel (green icon)  
7. **Step 6**: DIP #1 OFF → Lock panel
8. **Step 7**: DIP #3 OFF → Oil panel (night theme)
9. **Step 8**: DIP #4 OFF → Day theme, no panel reload

**Success Criteria**: ✅ All 9 steps execute correctly, proper priority handling, theme persistence

## Test Results Documentation

### Pass/Fail Tracking
Create a simple tracking sheet:

| Test | Status | Notes | Issues |
|------|--------|-------|--------|
| basic_startup | ✅/❌ | | |
| oil_panel_sensors | ✅/❌ | | |
| theme_switching | ✅/❌ | | |
| key_present | ✅/❌ | | |
| key_not_present | ✅/❌ | | |
| major_scenario | ✅/❌ | | |

### Serial Output Verification
Monitor these key log patterns:
- Trigger state changes: `[INFO] Trigger X: INACTIVE -> ACTIVE`
- Panel switches: `[INFO] Executing panel action: Load XPanel`
- Theme changes: `[DEBUG] Switching application theme to: X`
- Sensor readings: `[INFO] Pressure/Temperature reading changed to X`

### Common Issues and Troubleshooting

#### Display Issues
- **Symptom**: Garbled or missing display
- **Solution**: Refresh simulation, check firmware build
- **Prevention**: Ensure clean build before testing

#### Trigger Response Issues  
- **Symptom**: DIP switches don't trigger panel changes
- **Solution**: Check switch positions, verify GPIO connections
- **Debug**: Monitor serial for trigger state logs

#### Animation Problems
- **Symptom**: Stuck or glitchy needle animations
- **Solution**: Allow more time between steps, check for timing issues
- **Debug**: Monitor animation completion logs

#### Theme Issues
- **Symptom**: Colors don't change or wrong colors displayed
- **Solution**: Verify DIP switch #4 operation, check theme state logs
- **Debug**: Look for theme switching messages in serial output

## Test Completion Summary

### Full Test Suite Results
After completing all tests, summarize:
- **Total Tests**: 6
- **Passed**: ___/6  
- **Failed**: ___/6
- **Issues Found**: ___
- **Critical Failures**: ___

### Next Steps
- **All Pass**: System ready for hardware testing
- **Minor Issues**: Document for future improvement
- **Critical Failures**: Investigate and fix before hardware deployment

### Test Evidence
Consider capturing:
- Screenshots of key visual states (splash, panels, icons)
- Serial log excerpts for trigger verification
- Notes on any timing or behavior observations

This systematic approach ensures comprehensive validation of the Clarity system before hardware deployment.