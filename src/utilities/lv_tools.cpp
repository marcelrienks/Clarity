#include "utilities/lv_tools.h"

/// @brief Resets the screen to use the current theme's background style
/// @param screen the screen on which to render a blank display
void LvTools::reset_screen(lv_obj_t *screen)
{
    log_d("...");
    
    // Apply the current theme's background style from the StyleManager
    StyleManager::get_instance().apply_theme_to_screen(screen);
}

/// @brief Create a blank screen with the current theme's background style
/// @return an instance of a newly created screen
lv_obj_t* LvTools::create_blank_screen()
{
    log_d("...");
    
    lv_obj_t *screen = lv_obj_create(NULL);
    LvTools::reset_screen(screen);

    return screen;
}