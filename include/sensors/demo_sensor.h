#ifndef DEMO_SENSOR_H
#define DEMO_SENSOR_H

#include "interfaces/i_sensor.h"
#include "utilities/serial_logger.h"

#include <lvgl.h>
#include <LovyanGFX.hpp>
#include <random>

class DemoSensor : public ISensor
{
private:
    std::mt19937 _engine;                          // Mersenne Twister engine
    std::uniform_int_distribution<> _distribution; // Uniform distribution

    uint32_t last_read_time = 0;
    int current_reading = 0;
public:
    DemoSensor();
    ~DemoSensor();

    void init();
    int get_reading();
};

#endif // DEMO_SENSOR_H