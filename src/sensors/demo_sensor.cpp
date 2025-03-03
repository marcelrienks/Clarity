#include "sensors/demo_sensor.h"

DemoSensor::DemoSensor()
{
    _engine = std::mt19937(std::random_device{}());
    _distribution = std::uniform_int_distribution<int>(0, 100);
}

void DemoSensor::init()
{
    //TODO: once compiling, confirm that this is needed, due to interface
    // Not needed but required to satisfy interface
}

/// @brief Fakes getting a temperature reading from a sensor
/// @return temperature reading
Reading DemoSensor::get_reading()
{
    int elapsed_time = millis() - this->last_read_time;

    // This is where the sensor would be read, and potentially the data interpreted to some degree
    if (this->last_read_time == 0 || elapsed_time > 1000)
    {
        this->last_read_time = millis();

        // Generate a random number in the range [0, 100]
        this->current_reading = _distribution(_engine);

        SerialLogger().log_value("DemoSensor::get_reading()", "currentReading", std::to_string(this->current_reading));//TODO: the currentReading value is not being shown in the logs
    }
    
    return this->current_reading;
}

DemoSensor::~DemoSensor() {
    //TODO: once compiling, confirm that this is needed, due to interface
    // Not needed but required to satisfy interface
}