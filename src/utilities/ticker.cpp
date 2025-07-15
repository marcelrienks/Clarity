#include "utilities/ticker.h"
#include <functional>

// Static Methods

/// @brief Get the elapsed time since the last call to this function and update internal timestamp
/// @return elapsed time since last call in milliseconds
uint32_t Ticker::get_elapsed_millis() {
    static uint32_t last_time_stamp = millis();

    auto current_time_stamp = millis();
    auto elapsed = current_time_stamp - last_time_stamp;
    last_time_stamp = current_time_stamp;

    return elapsed;
}

/// @brief Adaptive Timing to generate a ~60fps refresh rate
/// @param start_time the start time of the loop method
void Ticker::handle_dynamic_delay(uint32_t start_time) {
    // Calculate how long processing took
    uint32_t elapsed_time = millis() - start_time;

    // Adjust delay to target ~60fps (16.7ms per frame)
    uint32_t target_frame_time = 16;
    if (elapsed_time < target_frame_time)
      delay(target_frame_time - elapsed_time);

    else
      delay(1); // No delay needed, just yield to other tasks
}

/// @brief Handle lv tasks by calculating the time differences since start up
void Ticker::handle_lv_tasks() {
    // log_d("...");
    static uint32_t last_tick_increment = 0;
    static uint32_t last_task_run = 0;
    
    uint32_t current_time = millis();
    
    // Increment tick counter with the elapsed time
    uint32_t elapsed = current_time - last_tick_increment;
    if (elapsed > 0) {
        lv_tick_inc(elapsed);
        last_tick_increment = current_time;
    }
    
    // Process all pending tasks
    lv_timer_handler();
    last_task_run = current_time;
}

/// @brief Execute function if throttling interval has elapsed
/// @param interval_ms Minimum interval in milliseconds between executions
/// @param func Function to execute if interval has elapsed
void Ticker::execute_throttled(uint32_t interval_ms, std::function<void()> func) {
    static uint32_t last_execution_time = 0;
    uint32_t current_time = millis();
    
    // Execute on first call or if interval has elapsed
    if (last_execution_time == 0 || current_time - last_execution_time >= interval_ms) {
        last_execution_time = current_time;
        func();
    }
}