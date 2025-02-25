#ifndef TICKER_H
#define TICKER_H

#include <Arduino.h>
#include <lvgl.h>

class Ticker
{
public:
    static uint32_t get_elapsed_millis();
    static void handle_lv_tasks();
};

#endif // TICKER_H