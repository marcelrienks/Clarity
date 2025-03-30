#include "utilities/lv_tools.h"

/// @brief Resets the screen to use a default black background
/// @param screen the screen on which to render a blank display
void LvTools::reset_screen(lv_obj_t *screen)
{
    lv_obj_set_style_bg_color(screen, lv_color_black(), LV_PART_MAIN); // Set background color, this is regardless of inversion build flag, black = black etc.
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, LV_PART_MAIN);       // Make it fully opaque
}

/// @brief Create a blank screen
/// @return an instance of a newly created screen
lv_obj_t* LvTools::create_blank_screen()
{
    lv_obj_t *screen = lv_obj_create(NULL);
    LvTools::reset_screen(screen);

    return screen;
}