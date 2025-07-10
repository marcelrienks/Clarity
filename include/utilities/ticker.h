#pragma once // preventing duplicate definitions, alternative to the traditional include guards

// System/Library Includes
#include <Arduino.h>
#include <lvgl.h>

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
 * - get_elapsed_millis(): High-resolution timing using Arduino millis()
 * - handle_lv_tasks(): Process LVGL timer callbacks and animations
 * - handle_dynamic_delay(): Adaptive delay based on execution time
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
    static uint32_t get_elapsed_millis();
    static void handle_dynamic_delay(uint32_t start_time);
    static void handle_lv_tasks();
};