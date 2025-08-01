#pragma once

#include <variant>
#include "utilities/types.h"

/**
 * @class ReadingHelper
 * @brief Utility class for working with Reading variant types
 * 
 * @details This utility provides type-safe operations for Reading variants
 * used throughout the sensor system. It simplifies common operations like
 * validity checking and type-safe value extraction.
 * 
 * @static_utility All methods are static - no instantiation required
 * @template_support Generic getValue<T>() for type-safe extraction
 * @error_handling Uses std::variant exception handling for type safety
 * 
 * @core_operations:
 * - isValid(): Check if Reading contains actual data (not std::monostate)
 * - getValue<T>(): Type-safe extraction of specific data types
 * 
 * @usage_examples:
 * ```cpp
 * Reading reading = sensor.getReading();
 * if (ReadingHelper::isValid(reading)) {
 *     int32_t value = ReadingHelper::getValue<int32_t>(reading);
 * }
 * ```
 * 
 * @exception_safety getValue<T>() throws std::bad_variant_access if wrong type
 * @performance Inline static methods with minimal overhead
 * 
 * @context This utility simplifies working with sensor readings throughout
 * the system. It's used by panels and components to safely extract typed
 * values from the generic Reading variant.
 */
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
