#include "utilities/lv_tools.h"
#include "interfaces/i_style_service.h"

// Static Methods

/// @brief Create a blank screen with the current theme's background style
/// @param styleService Style service for theme application
/// @return an instance of a newly created screen
lv_obj_t *LvTools::createBlankScreen(IStyleService *styleService)
{
    log_d("Creating blank screen with theme background");
    
    lv_obj_t *screen = lv_obj_create(NULL);
    LvTools::resetScreen(screen, styleService);

    return screen;
}

/// @brief Resets the screen to use the current theme's background style
/// @param screen the screen on which to render a blank display
/// @param styleService Style service for theme application
void LvTools::resetScreen(lv_obj_t *screen, IStyleService *styleService)
{
    log_d("Applying theme background style to screen");
    
    // Apply the current theme's background style using dependency injection
    if (styleService) {
        styleService->applyThemeToScreen(screen);
    }
}