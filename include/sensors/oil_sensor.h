#pragma once // preventing duplicate definitions, alternative to the traditional include guards

#include "interfaces/i_sensor.h"
#include "utilities/serial_logger.h"
#include "utilities/types.h"

#include <lvgl.h>
#include <LovyanGFX.hpp>
#include <random>

class OilSensor : public ISensor
{
public:
    OilSensor();

    void init() override;
    Reading get_reading() override;
};