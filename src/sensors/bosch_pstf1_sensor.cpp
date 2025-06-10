#include "sensors/bosch_pstf1_sensor.h"

BoschPstf1Sensor::BoschPstf1Sensor()
    : _engine(std::mt19937(std::random_device{}())),
      _pressure_distribution(std::uniform_int_distribution<int>(0, 60)),
      _temperature_distribution(std::uniform_int_distribution<int>(0, -120)) {}

void BoschPstf1Sensor::init()
{
    // Not needed but required to satisfy interface
}

// TODO: TEMP code for testing
Reading BoschPstf1Sensor::get_pressure_reading()
{
    int millies = millis();
    int elapsed_time = millies - this->last_read_time;

    this->last_read_time = millies;
    this->current_pressure_reading = _pressure_distribution(_engine);

    log_i("currentPressureReading is %s", std::to_string(this->current_pressure_reading).c_str());

    return this->current_pressure_reading;
}

Reading BoschPstf1Sensor::get_temperature_reading()
{
    int millies = millis();
    int elapsed_time = millies - this->last_read_time;

    this->last_read_time = millies;
    this->current_temperature_reading = _temperature_distribution(_engine);

    log_i("currentReading is %s", std::to_string(this->current_temperature_reading).c_str());

    return this->current_temperature_reading;
}