#pragma once

/**
 * @file gpio_pins.h
 * @brief GPIO pin definitions for Clarity digital gauge system
 *
 * @details This header defines all GPIO pin assignments used throughout the Clarity system.
 * Pin assignments are based on the NodeMCU-32S development board and ESP32 hardware constraints.
 *
 * @hardware_notes:
 * - GPIO 36 & 39: ADC input only pins (no pull-up/pull-down resistors)
 * - GPIO 25-27: General purpose I/O pins with full functionality
 * - All analog pins use 12-bit ADC resolution (0-4095 range)
 *
 * @context This centralizes all hardware pin assignments to prevent conflicts
 * and make hardware changes easier to manage across the entire codebase.
 */

namespace gpio_pins
{
// Analog Input Pins (ADC)

/**
 * @brief Oil pressure sensor analog input pin
 */
constexpr int OIL_PRESSURE = 36;

/**
 * @brief Oil temperature sensor analog input pin
 */
constexpr int OIL_TEMPERATURE = 39;

// Digital Input Pins

/**
 * @brief Key presence detection digital input pin
 */
constexpr int KEY_PRESENT = 25;

/**
 * @brief Key not present detection digital input pin
 */
constexpr int KEY_NOT_PRESENT = 26;

/**
 * @brief Lock state digital input pin
 */
constexpr int LOCK = 27;

/**
 * @brief Lights detection digital input pin
 */
constexpr int LIGHTS = 33;

/**
 * @brief Debug error trigger pin (for development only)
 */
constexpr int DEBUG_ERROR = 34;

/**
 * @brief Input button pin for single button navigation
 */
constexpr int INPUT_BUTTON = 32;
} // namespace gpio_pins