#pragma once // preventing duplicate definitions, alternative to the traditional include guards

#include "interfaces/i_sensor.h"
#include "utilities/types.h"

#include <lvgl.h>
#include <LovyanGFX.hpp>
#include <random>

class BoschPstf1Sensor : public ISensor
{
public:
    BoschPstf1Sensor();

    void init() override;
    Reading get_reading() override;

//TODO: TEMP for testing
private:
    std::mt19937 _engine;                          // Mersenne Twister engine
    std::uniform_int_distribution<> _distribution; // Uniform distribution

    int32_t last_read_time = 0;
    int32_t current_reading;
};