# Test 2.1: Oil Panel with Sensor Data

## Objective
Test oil panel functionality with sensor data, verify gauge needle animations and positioning.

## Test Setup
- **Pot1 (Pressure)**: Set to ~512 (halfway, ~5 Bar expected)
- **Pot2 (Temperature)**: Set to ~512 (halfway, ~60°C expected)
- **DIP Switches**: All OFF (00000000)
- **Expected Theme**: Day theme

## Expected Behavior
1. **System Startup**: Normal splash screen sequence
2. **Oil Panel Loading**:
   - Transitions to oil panel with sensor data
   - Both gauges should display with day theme styling
   - White scale ticks and oil icons
3. **Gauge Animations**:
   - Pressure needle animates from 0 to ~5 Bar
   - Temperature needle animates from 0 to ~60°C
   - Smooth animation transitions (not instant jumps)
4. **Final State**:
   - Both needles positioned at approximately halfway marks
   - Accurate readings displayed
   - No animation glitches or stuck needles

## Interactive Test Steps
1. **Start Simulation**: Observe initial startup
2. **Verify Animations**: Watch needle movements during loading
3. **Adjust Potentiometers**: 
   - Slowly turn Pot1 (pressure) - needle should follow
   - Slowly turn Pot2 (temperature) - needle should follow
   - Verify smooth updates and correct value mapping

## Pass Criteria
- [ ] Splash screen completes normally
- [ ] Oil panel loads with day theme (white ticks/icons)
- [ ] Pressure gauge animates to ~5 Bar position
- [ ] Temperature gauge animates to ~60°C position
- [ ] Real-time potentiometer changes update needles smoothly
- [ ] Serial monitor shows pressure/temperature change logs
- [ ] No stuck animations or display artifacts

## Expected Serial Output
```
[INFO] Pressure reading changed to 5 Bar (ADC: ~2048)
[INFO] Temperature reading changed to 60°C (ADC: ~2048)
```

## Test 2.2: Dynamic Oil Sensor Value Changes

### Objective
Test real-time sensor response to dynamic potentiometer changes during runtime.

### Extended Interactive Testing
1. **Baseline Reading**: Start with pots at halfway (~512)
2. **Pressure Sweep Test**:
   - Slowly rotate Pot1 from minimum to maximum
   - **Verify**: Needle follows smoothly from 0 to 10 Bar
   - **Verify**: No lag or jumping in needle movement
   - **Verify**: Serial output shows pressure changes

3. **Temperature Sweep Test**:
   - Slowly rotate Pot2 from minimum to maximum  
   - **Verify**: Needle follows smoothly from 0 to 120°C
   - **Verify**: Smooth temperature needle tracking
   - **Verify**: Serial output shows temperature changes

4. **Boundary Condition Testing**:
   - **Minimum Values**: Both pots to 0 → Verify needles at minimum positions
   - **Maximum Values**: Both pots to 1023 → Verify needles at maximum positions
   - **Rapid Changes**: Quick pot adjustments → Verify responsive updates

5. **Simultaneous Changes**:
   - Adjust both potentiometers simultaneously
   - **Verify**: Both needles update independently and smoothly
   - **Verify**: No interference between pressure and temperature updates

### Additional Pass Criteria
- [ ] **Real-time Response**: Needle movements follow pot changes within 1 second
- [ ] **Smooth Tracking**: No jerky or stepped needle movements
- [ ] **Full Range**: Needles reach true minimum and maximum positions
- [ ] **Independent Operation**: Pressure and temperature updates don't interfere
- [ ] **Boundary Handling**: System handles min/max values correctly
- [ ] **Serial Accuracy**: Logged values match visual needle positions

### Expected Serial Output for Dynamic Changes
```
[INFO] Pressure reading changed to 0 Bar (ADC: ~0)
[INFO] Pressure reading changed to 5 Bar (ADC: ~2048) 
[INFO] Pressure reading changed to 10 Bar (ADC: ~4095)
[INFO] Temperature reading changed to 0°C (ADC: ~0)
[INFO] Temperature reading changed to 60°C (ADC: ~2048)
[INFO] Temperature reading changed to 120°C (ADC: ~4095)
```

## Notes
- ADC values around 2048 should map to halfway values
- Needle animations should be smooth and complete within 2-3 seconds
- Interactive testing allows verification of real-time sensor response
- Dynamic testing validates the complete sensor-to-display pipeline
- Boundary testing ensures proper handling of extreme values