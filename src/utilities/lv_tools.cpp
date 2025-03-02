#include "utilities/lv_tools.h"

void LvTools::init_blank_screen(lv_obj_t *screen)
{
    lv_obj_set_style_bg_color(screen, lv_color_black(), LV_PART_MAIN); // Set background color
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, LV_PART_MAIN);       // Make it fully opaque
}

lv_obj_t* LvTools::create_blank_screen()
{
    lv_obj_t *screen = lv_obj_create(NULL);
    LvTools::init_blank_screen(screen);

    return screen;
}