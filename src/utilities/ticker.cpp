#include "utilities/ticker.h"
#include <functional>

// Static Methods

/// @brief Get the elapsed time since the last call to this function and update internal timestamp
/// @return elapsed time since last call in milliseconds
uint32_t Ticker::get_elapsed_millis() {
    static uint32_t lastTimeStamp = millis();

    auto currentTimeStamp = millis();
    auto elapsed = currentTimeStamp - lastTimeStamp;
    lastTimeStamp = currentTimeStamp;

    return elapsed;
}

/// @brief Adaptive Timing to generate a ~60fps refresh rate
/// @param start_time the start time of the loop method
void Ticker::handle_dynamic_delay(uint32_t startTime) {
    // Calculate how long processing took
    uint32_t elapsedTime = millis() - startTime;

    // Adjust delay to target ~60fps (16.7ms per frame)
    uint32_t targetFrameTime = 16;
    if (elapsedTime < targetFrameTime)
      delay(targetFrameTime - elapsedTime);

    else
      delay(1); // No delay needed, just yield to other tasks
}

/// @brief Handle lv tasks by calculating the time differences since start up
void Ticker::handle_lv_tasks() {
    // log_d("...");
    static uint32_t lastTickIncrement = 0;
    static uint32_t lastTaskRun = 0;
    
    uint32_t currentTime = millis();
    
    // Increment tick counter with the elapsed time
    uint32_t elapsed = currentTime - lastTickIncrement;
    if (elapsed > 0) {
        lv_tick_inc(elapsed);
        lastTickIncrement = currentTime;
    }
    
    // Process all pending tasks
    lv_timer_handler();
    lastTaskRun = currentTime;
}

