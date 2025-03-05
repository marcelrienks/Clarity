#include "utilities/common_types.h"

template<typename T>
T get_value_from_reading(const Reading& reading) {
    if (std::holds_alternative<T>(reading)) {
        return std::get<T>(reading);
    }
    // Return a default value if the reading doesn't hold the requested type
    return T{};
}