#include "utilities/lv_tools.h"

// Static Methods

/// @brief Create a blank screen with the current theme's background style
/// @return an instance of a newly created screen
lv_obj_t* LvTools::create_blank_screen()
{
    log_d("Creating blank screen with theme background");
    
    lv_obj_t *screen = lv_obj_create(NULL);
    LvTools::reset_screen(screen);

    return screen;
}

/// @brief Resets the screen to use the current theme's background style
/// @param screen the screen on which to render a blank display
void LvTools::reset_screen(lv_obj_t *screen)
{
    log_d("Applying theme background style to screen");
    
    // Apply the current theme's background style from the StyleManager
    // Step 4.5: Use global service pointer for backward compatibility
    extern IStyleService* g_styleService;
    if (g_styleService) {
        g_styleService->applyThemeToScreen(screen);
    }
}