#include "utilities/common_types.h"

template<typename T>
T* get_value_from_reading(Reading *reading) {
     return std::get_if<T>(reading);
}