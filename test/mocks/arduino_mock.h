#pragma once

/**
 * @file arduino_mock.h
 * @brief Mock Arduino constants for native testing
 * 
 * @details This file provides mock implementations of Arduino constants
 * that are needed by the test code but not available in native testing.
 */

// Arduino pin mode constants
#ifndef INPUT
#define INPUT 0
#define OUTPUT 1
#define INPUT_PULLUP 2
#define INPUT_PULLDOWN 3
#endif

// Arduino digital values
#ifndef HIGH
#define HIGH 1
#define LOW 0
#endif

// Arduino interrupt modes
#ifndef RISING
#define RISING 1
#define FALLING 2
#define CHANGE 3
#endif

// Arduino ADC constants
#ifndef ADC_11db
#define ADC_11db 3
#endif

// Arduino function declarations for native testing
extern "C" {
    unsigned long millis();
    void delay(unsigned long ms);
    void analogReadResolution(int bits);
    void analogSetAttenuation(int attenuation);
}