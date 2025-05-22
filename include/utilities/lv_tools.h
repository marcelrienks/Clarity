#pragma once // preventing duplicate definitions, alternative to the traditional include guards

#include "managers/style_manager.h"

#include <lvgl.h>

class LvTools
{
public:
    static void reset_screen(lv_obj_t *screen);
    static lv_obj_t* create_blank_screen();
};