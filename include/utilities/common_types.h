#pragma once // preventing duplicate definitions, alternative to the traditional include guards

#include <variant>
#include <string>

// Define the possible return types
using Reading = std::variant<std::monostate, int32_t, double, std::string, bool>;