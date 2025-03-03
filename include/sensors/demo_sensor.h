#pragma once // preventing duplicate definitions, alternative to the traditional include guards

#include "interfaces/i_sensor.h"
#include "utilities/serial_logger.h"
#include "utilities/common_types.h"

#include <lvgl.h>
#include <LovyanGFX.hpp>
#include <random>

class DemoSensor : public ISensor
{
private:
    std::mt19937 _engine;                          // Mersenne Twister engine
    std::uniform_int_distribution<> _distribution; // Uniform distribution

    int last_read_time;
    int current_reading;

public:
    DemoSensor();
    ~DemoSensor();

    void init() override;
    Reading get_reading() const override;
};