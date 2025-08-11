#include "utilities/unit_converter.h"
#include <sstream>
#include <iomanip>

// Pressure conversions
float UnitConverter::BarToPsi(float bar) {
    return bar * BAR_TO_PSI;
}

float UnitConverter::BarToKPa(float bar) {
    return bar * BAR_TO_KPA;
}

float UnitConverter::PsiToBar(float psi) {
    return psi / BAR_TO_PSI;
}

float UnitConverter::KPaToBar(float kpa) {
    return kpa / BAR_TO_KPA;
}

// Temperature conversions
float UnitConverter::CelsiusToFahrenheit(float celsius) {
    return celsius * CELSIUS_TO_FAHRENHEIT_MULT + CELSIUS_TO_FAHRENHEIT_OFFSET;
}

float UnitConverter::FahrenheitToCelsius(float fahrenheit) {
    return (fahrenheit - CELSIUS_TO_FAHRENHEIT_OFFSET) / CELSIUS_TO_FAHRENHEIT_MULT;
}

// Unit formatting helpers
std::string UnitConverter::FormatPressure(float value, const std::string& unit) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(1) << value << " " << GetPressureSymbol(unit);
    return oss.str();
}

std::string UnitConverter::FormatTemperature(float value, const std::string& unit) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(0) << value << "°" << GetTemperatureSymbol(unit);
    return oss.str();
}

// Get unit symbols
const char* UnitConverter::GetPressureSymbol(const std::string& unit) {
    if (unit == "PSI") return "PSI";
    if (unit == "kPa") return "kPa";
    return "Bar"; // Default to Bar
}

const char* UnitConverter::GetTemperatureSymbol(const std::string& unit) {
    if (unit == "F") return "F";
    return "C"; // Default to Celsius
}