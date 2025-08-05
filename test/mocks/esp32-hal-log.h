#pragma once

/**
 * @file esp32-hal-log.h
 * @brief Mock ESP32 HAL logging header for native testing
 */

#ifdef UNIT_TESTING

#include "Arduino.h"

// All logging functionality is provided by Arduino.h mock

#endif // UNIT_TESTING