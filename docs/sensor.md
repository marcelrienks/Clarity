# Sensor Implementation Guidelines

## Success Criteria for Oil Pressure and Temperature Sensors

### 1. Panel and Component Responsibilities
- **Oil pressure and temperature panels** and their respective components are responsible for:
  - Controlling the unit of measure for display
  - Setting the scale min/max values based on visual requirements
  - Determining tick count for gauge displays
  - These settings should be based on visual needs, not the sensor's measurement extent

### 2. Unit of Measure Injection
- **Panel managers** will supply the sensor with the preferred unit of measure when the sensor is instantiated
- The unit of measure will be injected during sensor initialization
- This ensures sensors return readings in the format expected by the consuming panel

### 3. Sensor Core Responsibilities
- **Pressure and temperature sensors** are responsible for:
  - Reading the analog voltage signal from their respective GPIO pins
  - Mapping the analog value to the relevant unit of measure that was injected during initialization
  - This approach ensures sensors can be changed in future without requiring updates to mapping logic elsewhere

### 4. Scale Independence
- The min/max values of display scales are determined by the component and panel
- **Sensors only report the reading value**, even if it exceeds the min/max bounds of the display scale
- This separation ensures sensors remain independent of UI display concerns

### 5. Preference-Based Configuration
- The preferred unit of measure will be injected into the sensor by the panel
- This injection will be based on reading user preferences from the preference service
- This ensures the entire system respects user-configured units

## Implementation Plan

### Phase 1: Refactor Sensor Architecture

1. **Update ISensor Interface**
   - Add a method to set the target unit of measure
   - Consider adding a method to get supported units

2. **Modify Sensor Constructors**
   - Remove preference service dependency from sensor constructors
   - Add unit of measure parameter to sensor initialization

3. **Create Unit-Aware Sensor Base Class**
   - Implement common unit conversion logic
   - Handle unit storage and validation
   - Provide consistent interface for all sensors

### Phase 2: Update Oil Pressure Sensor

1. **Remove Internal Unit Logic**
   - Remove preference service dependency
   - Remove hardcoded unit conversions in GetReading()

2. **Implement Unit-Based Reading**
   - Store target unit during initialization
   - Convert ADC readings directly to requested unit
   - Return readings in the specified unit format

3. **Simplify Sensor Logic**
   - Focus only on ADC reading and conversion
   - Remove UI-related concerns (scales, display ranges)

### Phase 3: Update Oil Temperature Sensor

1. **Apply Same Refactoring as Pressure Sensor**
   - Remove preference service dependency
   - Remove internal unit decision logic

2. **Implement Temperature Unit Conversion**
   - Support Celsius and Fahrenheit based on injected unit
   - Maintain accuracy in conversions

### Phase 4: Update Panel Managers

1. **OemOilPanel Updates**
   - Read pressure and temperature units from preferences
   - Pass units to sensors during initialization
   - Remove any sensor-specific unit logic from panels

2. **Component Updates**
   - Ensure components work with sensor readings in their expected units
   - Update scale configurations to be independent of sensor ranges

### Phase 5: Testing and Validation

1. **Unit Tests**
   - Test sensors with different injected units
   - Verify correct conversions for edge cases
   - Test sensor independence from display scales

2. **Integration Tests**
   - Verify panel-sensor communication
   - Test preference changes and unit updates
   - Ensure smooth transitions between units

3. **Hardware Testing**
   - Validate ADC readings with actual hardware
   - Test full range of sensor inputs
   - Verify accuracy of unit conversions

### Benefits of This Architecture

1. **Modularity**: Sensors become truly modular and replaceable
2. **Flexibility**: Easy to add new sensor types or units
3. **Separation of Concerns**: Clear boundaries between sensing and display
4. **Maintainability**: Changes to display don't affect sensor logic
5. **Testability**: Sensors can be tested independently of UI components