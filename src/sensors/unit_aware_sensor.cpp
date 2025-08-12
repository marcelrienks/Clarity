#include "sensors/unit_aware_sensor.h"
#include "managers/error_manager.h"
#include <Arduino.h>
#include <esp32-hal-log.h>
#include <algorithm>

// Constructor
UnitAwareSensor::UnitAwareSensor(IGpioProvider* gpioProvider, int updateRateMs) 
    : gpioProvider_(gpioProvider), updateIntervalMs_(updateRateMs)
{
    // Initialize with zero reading
    currentReading_ = 0;
    previousReading_ = -1; // Ensure first update is detected as changed
}

// ISensor interface implementation

void UnitAwareSensor::SetTargetUnit(const std::string& unit)
{
    // Validate unit is supported
    if (!IsUnitSupported(unit)) {
        ErrorManager::Instance().ReportError(ErrorLevel::WARNING, "UnitAwareSensor",
            "Unsupported unit requested: " + unit + ". Using default.");
        
        // Use first supported unit as default
        auto supportedUnits = GetSupportedUnits();
        if (!supportedUnits.empty()) {
            targetUnit_ = supportedUnits[0];
        }
    } else {
        targetUnit_ = unit;
        log_d("Target unit set to: %s", targetUnit_.c_str());
    }
}

Reading UnitAwareSensor::GetReading()
{
    unsigned long currentTime = millis();
    
    // Only read new value every updateIntervalMs_ milliseconds
    if (currentTime - lastUpdateTime_ >= updateIntervalMs_) {
        lastUpdateTime_ = currentTime;
        previousReading_ = currentReading_; // Store current before reading new
        
        // Read raw value from derived class implementation
        int32_t rawValue = ReadRawValue();
        
        // Check for invalid readings
        if (rawValue < 0 || rawValue > ADC_MAX_VALUE) {
            ErrorManager::Instance().ReportError(ErrorLevel::ERROR, "UnitAwareSensor",
                "Raw reading out of range: " + std::to_string(rawValue));
            return std::monostate{}; // Return invalid reading
        }
        
        // Convert to target unit using derived class implementation
        int32_t newValue = ConvertReading(rawValue);
        
        // Only update if value actually changed (avoid redundant updates)
        if (newValue != currentReading_) {
            currentReading_ = newValue;
            log_i("Sensor reading changed to %d %s (raw: %d)", 
                currentReading_, targetUnit_.c_str(), rawValue);
        }
    }
    
    return currentReading_;
}

// Protected methods

bool UnitAwareSensor::IsUnitSupported(const std::string& unit) const
{
    auto supportedUnits = GetSupportedUnits();
    return std::find(supportedUnits.begin(), supportedUnits.end(), unit) != supportedUnits.end();
}