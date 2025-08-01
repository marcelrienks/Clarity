#pragma once

#include <variant>
#include "utilities/types.h"
#include "interfaces/i_gpio_provider.h"
#include "hardware/gpio_pins.h"

class ReadingHelper {
public:
    static bool isValid(const Reading& reading) {
        return !std::holds_alternative<std::monostate>(reading);
    }
    
    template<typename T>
    static T getValue(const Reading& reading) {
        return std::get<T>(reading);
    }
    
    /// @brief Read GPIO pins and determine key state
    /// @param gpio GPIO provider for hardware abstraction
    /// @return KeyState based on GPIO pin readings
    static KeyState readKeyState(IGpioProvider* gpio) {
        bool pin25High = gpio->digitalRead(gpio_pins::KEY_PRESENT);
        bool pin26High = gpio->digitalRead(gpio_pins::KEY_NOT_PRESENT);
        
        if (pin25High && pin26High) {
            return KeyState::Inactive; // Both pins HIGH - invalid state
        }
        
        if (pin25High) {
            return KeyState::Present;
        }
        
        if (pin26High) {
            return KeyState::NotPresent;
        }
        
        return KeyState::Inactive; // Both pins LOW
    }
};
