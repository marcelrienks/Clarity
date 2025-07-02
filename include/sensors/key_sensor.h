#pragma once // preventing duplicate definitions, alternative to the traditional include guards

#include "interfaces/i_sensor.h"
#include "utilities/types.h"

#include <lvgl.h>
#include <LovyanGFX.hpp>
#include <random>

class KeySensor : public ISensor
{
public:
    KeySensor();

    void init() override;
    Reading get_reading() override;
};