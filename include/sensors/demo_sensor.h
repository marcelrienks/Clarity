#ifndef DEMO_SENSOR_H
#define DEMO_SENSOR_H

#include "interfaces/i_sensor.h"
#include "utilities/serial_logger.h"
#include "utilities/tools.h"

#include <lvgl.h>
#include <LovyanGFX.hpp>
#include <random>

class DemoSensor : public ISensor
{
private:
    std::mt19937 _engine;                          // Mersenne Twister engine
    std::uniform_int_distribution<> _distribution; // Uniform distribution

    std::string last_read_time;
    std::string current_reading;
public:
    DemoSensor();
    ~DemoSensor();

    void init();
    std::string get_reading();
};

#endif // DEMO_SENSOR_H