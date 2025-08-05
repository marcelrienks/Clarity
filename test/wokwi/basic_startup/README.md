# Test 1.1: Basic System Startup

## Objective
Verify default startup sequence with splash screen animation and transition to oil panel.

## Test Setup
- **Potentiometers**: Both set to 0 (minimum position)
- **DIP Switches**: All OFF (00000000)
- **Expected Theme**: Day theme (white text/icons)

## Expected Behavior
1. **Boot Sequence**: ESP32 boots and initializes
2. **Splash Screen**: 
   - Displays "Clarity" logo/text with day theme (white text)
   - Fade-in animation completes
   - Screen displays for ~3 seconds
   - Fade-out animation begins
3. **Oil Panel Transition**:
   - Transitions to oil panel automatically
   - Displays dual gauges (pressure + temperature)
   - Needles should be at minimum positions (0 Bar, 0°C)
   - Day theme styling (white scale ticks and icons)

## Pass Criteria
- [ ] Splash screen displays with white text (day theme)
- [ ] Splash animations complete successfully
- [ ] Automatic transition to oil panel occurs
- [ ] Oil gauges display with needles at minimum
- [ ] No errors in serial monitor
- [ ] Display renders correctly on square ILI9341

## Notes
- This test verifies the basic system initialization
- No sensor data should be present (min values)
- All triggers should be INACTIVE