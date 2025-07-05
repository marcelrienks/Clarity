#pragma once // preventing duplicate definitions, alternative to the traditional include guards

#include "managers/style_manager.h"

#include <Arduino.h>
#include <lvgl.h>

/**
 * @class LvTools
 * @brief LVGL screen management and utility functions
 * 
 * @details This utility class provides common LVGL screen management functions
 * used throughout the application. It standardizes screen creation and
 * cleanup operations with consistent theming and styling.
 * 
 * @static_class All methods are static - no instantiation required
 * @screen_management Standardized screen creation and styling
 * @theme_integration Automatic StyleManager integration
 * 
 * @core_functions:
 * - reset_screen(): Clean screen reset with theme application
 * - create_blank_screen(): Create new screen with default theming
 * 
 * @standardization_benefits:
 * - Consistent screen styling across all panels
 * - Automatic theme application
 * - Reduced code duplication
 * - Centralized screen management logic
 * 
 * @usage_context:
 * - Panel screen creation (SplashPanel, OemOilPanel)
 * - Screen transitions and cleanup
 * - Theme application to new screens
 * 
 * @context This utility creates and manages LVGL screens with
 * consistent theming. All panels use these functions to create their
 * screens, ensuring uniform styling and proper cleanup.
 */
class LvTools
{
public:
    static void reset_screen(lv_obj_t *screen);
    static lv_obj_t* create_blank_screen();
};