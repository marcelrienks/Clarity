#include "utilities/ticker.h"
#include "utilities/serial_logger.h"

/// @brief get the elapsed time since the last call to this function
/// @return elapsed time since last get
uint32_t Ticker::get_elapsed_millis() {
    static uint32_t last_time_stamp = millis();

    auto current_time_stamp = millis();
    auto elapsed = current_time_stamp - last_time_stamp;
    last_time_stamp = current_time_stamp;

    return elapsed;
}

/// @brief Handle lv tasks by calculating the time differences since start up
void Ticker::handle_lv_tasks() {
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
    lv_task_handler();
    last_task_run = current_time;
}