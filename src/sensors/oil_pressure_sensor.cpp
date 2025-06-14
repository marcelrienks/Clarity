#include "sensors/oil_pressure_sensor.h"

OilPressureSensor::OilPressureSensor()
    : _engine(std::mt19937(std::random_device{}())),
      _distribution(std::uniform_int_distribution<int>(0, 60)) {}

void OilPressureSensor::init()
{
    // Not needed but required to satisfy interface
}

// TODO: TEMP code for testing
Reading OilPressureSensor::get_reading()
{
    this->current_reading = _distribution(_engine);

    log_i("current_reading is %s", std::to_string(this->current_reading).c_str());

    return this->current_reading;
}