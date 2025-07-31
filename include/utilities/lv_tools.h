#pragma once // preventing duplicate definitions, alternative to the traditional include guards

// System/Library Includes
#include <Arduino.h>
#include <lvgl.h>

// Forward declaration
class IStyleService;

/**
 * @class LvTools
 * @brief LVGL screen management and utility functions
 * 
 * @details This utility class provides common LVGL screen management functions
 * used throughout the application. It standardizes screen creation and
 * cleanup operations with consistent theming and styling.
 * 
 * @dependency_injection Methods now accept IStyleService parameter for proper DI
 * @screen_management Standardized screen creation and styling
 * @theme_integration Direct IStyleService integration (no globals)
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
    // Static Methods with dependency injection
    static lv_obj_t* create_blank_screen(IStyleService* styleService);
    static void reset_screen(lv_obj_t *screen, IStyleService* styleService);
};