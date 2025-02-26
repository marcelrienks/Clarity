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
    lv_tick_inc(Ticker::get_elapsed_millis());
    lv_task_handler();
    SerialLogger().log_point("Ticker::handle_lv_tasks()", "Completed");
}