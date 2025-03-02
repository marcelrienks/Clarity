#include "utilities/tools.h"

/// @brief Convert a string to int, with validation
/// @return an integer of the string
int Tools::string_to_int(const std::string& string)
{
    // Check for empty string
    if (string.empty())
        return 0;

    // Skip whitespace at beginning (optional)
    size_t firstNonSpace = string.find_first_not_of(" \t\n\r\f\v");
    if (firstNonSpace == std::string::npos)
        // String contains only whitespace
        return 0;

    try
    {
        size_t pos = 0;
        int result = std::stoi(string, &pos);

        // Check if the entire string was converted
        // (pos will be the position of the first character after the number)
        if (pos != string.length())
        {
            // Find the first non-whitespace after the number
            size_t nextNonSpace = string.find_first_not_of(" \t\n\r\f\v", pos);
            if (nextNonSpace != std::string::npos)
                // Found non-whitespace after the number - not fully converted
                return 0;
        }

        return result;
    }
    catch (const std::invalid_argument &)
    {
        // String cannot be converted to an integer
        return 0;
    }
    catch (const std::out_of_range &)
    {
        // Value would be out of range for an int
        return 0;
    }
}