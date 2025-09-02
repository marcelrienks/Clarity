# Sensor Architecture

## Oil Pressure and Temperature Sensors

### 1. Panel and Component Responsibilities
- **Oil pressure and temperature panels** and their respective components are responsible for:
  - Controlling the unit of measure for display
  - Setting the scale min/max values based on visual requirements
  - Determining tick count for gauge displays
  - These settings should be based on visual needs, not the sensor's measurement extent

### 2. Unit of Measure Injection
- **Panels** will supply the sensor with the preferred unit of measure when they create their sensors
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

## Sensor Design Requirements

### ISensor Interface
- Method to set the target unit of measure
- Method to get supported units
- Consistent interface for all sensors

### Unit-Aware Sensor Base Class
- Common unit conversion logic
- Unit storage and validation
- Consistent interface implementation

## Oil Pressure Sensor

### Core Responsibilities
- ADC reading and conversion
- Unit-based reading output
- No UI-related concerns (scales, display ranges)

### Unit Support
- Store target unit during initialization
- Convert ADC readings directly to requested unit
- Return readings in the specified unit format
- Support Bar, PSI, kPa units

## Oil Temperature Sensor

### Core Responsibilities
- ADC reading and conversion
- Temperature unit conversion
- Maintain accuracy in conversions

### Unit Support
- Support Celsius and Fahrenheit based on injected unit
- Accurate temperature conversions
- No preference service dependency

## Panel Integration

### OemOilPanel Requirements
- Read pressure and temperature units from preferences
- Pass units to sensors during panel initialization
- No sensor-specific unit logic in panels

### Component Requirements
- Work with sensor readings in their expected units
- Scale configurations independent of sensor ranges
- Visual display separate from sensor logic

## Architecture Benefits

1. **Modularity**: Sensors are truly modular and replaceable
2. **Flexibility**: Easy to add new sensor types or units
3. **Separation of Concerns**: Clear boundaries between sensing and display
4. **Maintainability**: Changes to display don't affect sensor logic
5. **Testability**: Sensors can be tested independently of UI components