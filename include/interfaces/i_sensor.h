#pragma once // preventing duplicate definitions, alternative to the traditional include guards

// Project Includes
#include "utilities/types.h"

/**
 * @interface ISensor
 * @brief Base interface for all sensor implementations in the Clarity system
 * 
 * @details This interface defines the contract for sensors that acquire data
 * from hardware inputs or simulated sources. Sensors represent the Model layer
 * in the MVP architecture, providing data to panels and components.
 * 
 * @design_pattern Model in MVP - handles data acquisition and processing
 * @data_flow:
 * 1. init(): Initialize sensor hardware/configuration
 * 2. getReading(): Acquire current sensor value as Reading variant
 * 
 * @reading_types:
 * - int32_t: Numeric values (pressure, temperature)
 * - bool: Boolean states (key presence, lock status)
 * - double: Precise measurements
 * - std::string: Status messages or text data
 * - std::monostate: Invalid/uninitialized readings
 * 
 * @hardware_abstraction:
 * - Sensors abstract GPIO/ADC access via IGpioProvider
 * - Support both real hardware and simulation modes
 * - Consistent Reading interface regardless of data source
 * 
 * @implementations:
 * - KeySensor: Ignition key presence detection
 * - LockSensor: Vehicle lock status monitoring
 * - LightSensor: Ambient light level detection
 * - OilPressureSensor: Engine oil pressure monitoring
 * - OilTemperatureSensor: Engine oil temperature monitoring
 * 
 * @context This is the base interface for all sensors.
 * Sensors provide data to panels which then update their components
 * for display to the user.
 */
class ISensor
{
public:
    // Core Interface Methods
    virtual void Init() = 0;
    virtual Reading GetReading() = 0;
};