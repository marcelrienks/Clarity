#pragma once

#include <string>

/**
 * @class UnitConverter
 * @brief Static utility class for converting between different units
 * 
 * @details Provides conversion methods for pressure and temperature units
 * used in the Clarity system. All conversion methods are static and stateless.
 */
class UnitConverter {
public:
    // Pressure conversions (base unit: Bar)
    static float BarToPsi(float bar);
    static float BarToKPa(float bar);
    static float PsiToBar(float psi);
    static float KPaToBar(float kpa);
    
    // Temperature conversions (base unit: Celsius)
    static float CelsiusToFahrenheit(float celsius);
    static float FahrenheitToCelsius(float fahrenheit);
    
    // Unit formatting helpers
    static std::string FormatPressure(float value, const std::string& unit);
    static std::string FormatTemperature(float value, const std::string& unit);
    
    // Get unit symbols
    static const char* GetPressureSymbol(const std::string& unit);
    static const char* GetTemperatureSymbol(const std::string& unit);

private:
    // Conversion constants
    static constexpr float BAR_TO_PSI = 14.5038f;
    static constexpr float BAR_TO_KPA = 100.0f;
    static constexpr float CELSIUS_TO_FAHRENHEIT_MULT = 9.0f / 5.0f;
    static constexpr float CELSIUS_TO_FAHRENHEIT_OFFSET = 32.0f;
};