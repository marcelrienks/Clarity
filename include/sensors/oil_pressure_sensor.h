#pragma once // preventing duplicate definitions, alternative to the traditional include guards

#include "interfaces/i_sensor.h"
#include "utilities/types.h"

#include <lvgl.h>
#include <LovyanGFX.hpp>
#include <esp_random.h>

class OilPressureSensor : public ISensor
{
public:
    OilPressureSensor();

    void init() override;
    Reading get_reading() override;
    
    // Delta-based update support
    bool has_value_changed() override;
    Reading get_cached_reading() override;

    // TODO: TEMP for testing
private:
    int32_t _current_reading = 0;
    int32_t _previous_reading = -1;
    unsigned long _last_update_time = 0;
    static constexpr unsigned long UPDATE_INTERVAL_MS = 1000; // Update every 1000ms (1Hz)
};