#pragma once // preventing duplicate definitions, alternative to the traditional include guards

// System/Library Includes
#include <Arduino.h>
#include <lvgl.h>
#include <functional>

/**
 * @class Ticker
 * @brief LVGL timing and dynamic delay management utility
 * 
 * @details This utility class provides timing services for LVGL integration
 * and dynamic delay management. It handles LVGL's task scheduling and
 * provides adaptive delays based on system performance.
 * 
 * @static_class All methods are static - no instantiation required
 * @lvgl_integration Manages LVGL timer and task execution
 * @performance_optimization Dynamic delays prevent CPU overload
 * 
 * @core_functions:
 * - getElapsedMillis(): High-resolution timing using Arduino millis()
 * - handleLvTasks(): Process LVGL timer callbacks and animations
 * - handleDynamicDelay(): Adaptive delay based on execution time
 * 
 * @timing_strategy:
 * - Fixed LVGL tick processing
 * - Dynamic delays adjust to system load
 * - Prevents blocking operations from affecting UI responsiveness
 * 
 * @usage_context:
 * - Main loop timing coordination
 * - LVGL task scheduling
 * - Performance-adaptive delays
 * - Animation timing synchronization
 * 
 * @context This utility manages timing for the main application loop.
 * It ensures LVGL gets proper time slices for animations and UI updates
 * while providing adaptive delays for optimal performance.
 */
class Ticker
{
public:
    // Static Methods
    static uint32_t getElapsedMillis();
    static void handleDynamicDelay(uint32_t startTime);
    static void handleLvTasks();
};