#include "sensors/demo_sensor.h"

DemoSensor::DemoSensor()
    : _engine(std::mt19937(std::random_device{}())), _distribution(std::uniform_int_distribution<int>(0, 100)) {}

/// @brief Initializes the sensor. This method is required by the Sensor interface but not used in this demo.
void DemoSensor::init()
{
    // Not needed but required to satisfy interface
}

/// @brief Fakes getting a temperature reading from a sensor
/// @return temperature reading
Reading DemoSensor::get_reading()
{
    int millies = millis();
    int elapsed_time = millies - this->last_read_time;

    // This is where the sensor would be read, and potentially the data interpreted to some degree
    if (this->last_read_time == 0 || elapsed_time > 1000)
    {
        this->last_read_time = millies;

        // Generate a random number in the range [0, 100]
        this->current_reading = _distribution(_engine);

        log_i("currentReading is %s", std::to_string(this->current_reading).c_str());
    }
    else
        log_d("Reading bypassed because elapsed_time is %i", elapsed_time);

    return this->current_reading;
}