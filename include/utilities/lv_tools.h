#ifndef LV_TOOLS_H
#define LV_TOOLS_H

#include <lvgl.h>

class LvTools
{
public:
    static void init_blank_screen(lv_obj_t *screen);
    static lv_obj_t* create_blank_screen();
};

#endif // LV_TOOLS_H