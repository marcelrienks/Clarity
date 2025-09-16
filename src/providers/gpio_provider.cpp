#include "providers/gpio_provider.h"
#include <esp32-hal-log.h>

/**
 * @brief Reads the digital state of a GPIO pin
 * @param pin The GPIO pin number to read
 * @return true if pin is HIGH, false if LOW
 *
 * Provides hardware abstraction for digital GPIO reads. Wraps the Arduino
 * digitalRead function to provide a consistent interface for GPIO operations
 * across the system.
 */
bool GpioProvider::DigitalRead(int pin)
{
    return ::digitalRead(pin);
}

/**
 * @brief Reads the analog value from a GPIO pin
 * @param pin The GPIO pin number to read
 * @return 16-bit ADC value representing the analog voltage
 *
 * Provides hardware abstraction for analog GPIO reads. Wraps the Arduino
 * analogRead function to provide ADC readings for sensor inputs.
 */
uint16_t GpioProvider::AnalogRead(int pin)
{
    return ::analogRead(pin);
}

/**
 * @brief Configures a GPIO pin as input or output
 * @param pin The GPIO pin number to configure
 * @param mode The pin mode (INPUT, OUTPUT, INPUT_PULLUP, etc.)
 *
 * Provides hardware abstraction for GPIO pin configuration. Wraps the Arduino
 * pinMode function to configure GPIO pins for different operation modes.
 */
void GpioProvider::PinMode(int pin, int mode)
{
    ::pinMode(pin, mode);
}

/**
 * @brief Attaches an interrupt handler to a GPIO pin
 * @param pin The GPIO pin number for interrupt attachment
 * @param callback Function pointer to the interrupt handler
 * @param mode Interrupt trigger mode (RISING, FALLING, CHANGE, etc.)
 *
 * Configures hardware interrupts for GPIO pins. Validates pin number,
 * attaches the interrupt handler, and tracks the attachment state.
 * Used for real-time response to hardware events.
 */
void GpioProvider::AttachInterrupt(int pin, void (*callback)(), int mode)
{
    if (pin >= 0 && pin < MAX_GPIO_PIN) {
        log_i("GPIO %d interrupt attached", pin);
        ::attachInterrupt(digitalPinToInterrupt(pin), callback, mode);
        attachedInterrupts_[pin] = true;
    }
}

/**
 * @brief Detaches an interrupt handler from a GPIO pin
 * @param pin The GPIO pin number to detach interrupt from
 *
 * Removes hardware interrupt configuration from a GPIO pin. Validates
 * pin number, detaches the interrupt, and updates tracking state.
 * Used for cleanup and reconfiguration of interrupt-driven sensors.
 */
void GpioProvider::DetachInterrupt(int pin)
{
    if (pin >= 0 && pin < MAX_GPIO_PIN) {
        log_i("GPIO %d interrupt detached", pin);
        ::detachInterrupt(digitalPinToInterrupt(pin));
        attachedInterrupts_[pin] = false;
    }
}

/**
 * @brief Checks if a GPIO pin has an attached interrupt
 * @param pin The GPIO pin number to check
 * @return true if pin has an attached interrupt, false otherwise
 *
 * Queries the interrupt attachment state for a GPIO pin. Validates
 * pin number and returns the tracked attachment state. Used for
 * interrupt management and debugging.
 */
bool GpioProvider::HasInterrupt(int pin)
{
    return (pin >= 0 && pin < MAX_GPIO_PIN) ? attachedInterrupts_[pin] : false;
}