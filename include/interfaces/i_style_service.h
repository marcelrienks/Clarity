#pragma once

// System/Library Includes
#include <lvgl.h>

// Project Includes
#include "utilities/types.h"

/**
 * @interface IStyleService
 * @brief Interface for style management and theme operations
 * 
 * @details This interface abstracts theme management functionality,
 * providing access to LVGL styles and theme operations. Implementations
 * should handle initialization, theme switching, and style application
 * to screen objects.
 * 
 * @design_pattern Interface Segregation - Focused on style operations only
 * @testability Enables mocking for unit tests
 * @dependency_injection Replaces direct StyleManager singleton access
 */
class IStyleService
{
public:
    virtual ~IStyleService() = default;

    // Core Functionality Methods
    
    /**
     * @brief Initialize all styles with default values
     */
    virtual void initializeStyles() = 0;

    /**
     * @brief Check if the style service has been initialized
     * @return true if initialized, false otherwise
     */
    virtual bool isInitialized() const = 0;

    /**
     * @brief Initialize the style service with a specific theme
     * @param theme Theme identifier (e.g., "DAY", "NIGHT")
     */
    virtual void init(const char* theme) = 0;

    /**
     * @brief Apply the current theme to a screen object
     * @param screen LVGL screen object to apply theme to
     */
    virtual void applyThemeToScreen(lv_obj_t* screen) = 0;

    /**
     * @brief Switch to a different theme
     * @param theme Theme identifier to switch to
     */
    virtual void setTheme(const char* theme) = 0;

    // Style Accessor Methods
    
    /**
     * @brief Get the background style for the current theme
     * @return Reference to background LVGL style object
     */
    virtual lv_style_t& getBackgroundStyle() = 0;

    /**
     * @brief Get the text style for the current theme
     * @return Reference to text LVGL style object
     */
    virtual lv_style_t& getTextStyle() = 0;

    /**
     * @brief Get the gauge normal style for the current theme
     * @return Reference to gauge normal LVGL style object
     */
    virtual lv_style_t& getGaugeNormalStyle() = 0;

    /**
     * @brief Get the gauge warning style for the current theme
     * @return Reference to gauge warning LVGL style object
     */
    virtual lv_style_t& getGaugeWarningStyle() = 0;

    /**
     * @brief Get the gauge danger style for the current theme
     * @return Reference to gauge danger LVGL style object
     */
    virtual lv_style_t& getGaugeDangerStyle() = 0;

    /**
     * @brief Get the gauge indicator style (major ticks)
     * @return Reference to gauge indicator LVGL style object
     */
    virtual lv_style_t& getGaugeIndicatorStyle() = 0;

    /**
     * @brief Get the gauge items style (minor ticks)
     * @return Reference to gauge items LVGL style object
     */
    virtual lv_style_t& getGaugeItemsStyle() = 0;

    /**
     * @brief Get the gauge main style (arc)
     * @return Reference to gauge main LVGL style object
     */
    virtual lv_style_t& getGaugeMainStyle() = 0;

    /**
     * @brief Get the gauge danger section style
     * @return Reference to gauge danger section LVGL style object
     */
    virtual lv_style_t& getGaugeDangerSectionStyle() = 0;

    // Theme Information Methods
    
    /**
     * @brief Get the current theme identifier
     * @return Current theme string identifier
     */
    virtual const char* getCurrentTheme() const = 0;

    // Color Access Methods
    
    /**
     * @brief Get the theme colors for the current theme
     * @return Reference to ThemeColors structure with all color definitions
     */
    virtual const ThemeColors& getThemeColors() const = 0;
};