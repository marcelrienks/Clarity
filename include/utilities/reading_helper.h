#pragma once

#include <variant>
#include "utilities/types.h"

class ReadingHelper {
public:
    static bool isValid(const Reading& reading) {
        return !std::holds_alternative<std::monostate>(reading);
    }
    
    template<typename T>
    static T getValue(const Reading& reading) {
        return std::get<T>(reading);
    }
};
