#include "sensors/gpio_sensor.h"
#include "utilities/logging.h"
#include <Arduino.h>
#include <esp32-hal-log.h>
#ifdef CLARITY_DEBUG
#include "managers/error_manager.h"
#endif

GpioSensor::GpioSensor(const GpioSensorConfig& config, IGpioProvider* gpioProvider)
    : config_(config)
    , gpioProvider_(gpioProvider)
    , previousState_(false)
{
    // Constructor should be lightweight - initialization happens in Init()
}

GpioSensor::~GpioSensor()
{
    if (gpioProvider_ && initialized_) {
        // Clean up GPIO interrupt if attached
        // Note: Only key sensors used interrupts in original implementation
        if (config_.pin == gpio_pins::KEY_PRESENT || config_.pin == gpio_pins::KEY_NOT_PRESENT) {
            gpioProvider_->DetachInterrupt(config_.pin);
        }
    }
}

void GpioSensor::Init()
{
    if (!gpioProvider_) {
        log_e("%s: GPIO provider is null", config_.name);
        return;
    }

    // Check debug-only sensor availability
    #ifndef CLARITY_DEBUG
    if (config_.debugOnly) {
        log_w("%s: Debug-only sensor not available in release build", config_.name);
        initialized_ = false;
        return;
    }
    #endif

    // Configure GPIO pin based on configuration
    gpioProvider_->PinMode(config_.pin, config_.pinMode);

    // Initialize the sensor state to avoid false change detection
    previousState_ = readLogicalState();
    initialized_ = true;

    log_i("%s initialization completed on GPIO %d, mode: %d, initial state: %s",
          config_.name, config_.pin, config_.pinMode,
          previousState_ ? "ACTIVE" : "INACTIVE");
}

Reading GpioSensor::GetReading()
{
    bool currentState = readLogicalState();
    return Reading(currentState);
}

bool GpioSensor::GetState()
{
    return readLogicalState();
}

bool GpioSensor::HasStateChanged()
{
    if (!initialized_) {
        log_w("%s: HasStateChanged called before initialization", config_.name);
        return false;
    }

    // Check debug-only sensor availability at runtime
    #ifndef CLARITY_DEBUG
    if (config_.debugOnly) {
        return false; // Debug sensors don't change in release builds
    }
    #endif

    bool currentState = readLogicalState();
    bool oldState = previousState_; // Save BEFORE DetectChange modifies it

    // Add detailed GPIO state debugging for test automation
    log_t("%s GPIO %d poll: current=%s, previous=%s",
          config_.name, config_.pin,
          readRawPinState() ? "HIGH" : "LOW",
          oldState ? "ACTIVE" : "INACTIVE");

    bool changed = DetectChange(currentState, previousState_);

    if (changed) {
        log_t("%s state changed: %s -> %s",
              config_.name,
              oldState ? "ACTIVE" : "INACTIVE",
              currentState ? "ACTIVE" : "INACTIVE");

        // Special case logging for key sensors to maintain compatibility
        if (config_.pin == gpio_pins::KEY_PRESENT) {
            log_t("KeyPresentSensor state changed: %s -> %s",
                  oldState ? "PRESENT" : "NOT_PRESENT",
                  currentState ? "PRESENT" : "NOT_PRESENT");
        }
        else if (config_.pin == gpio_pins::KEY_NOT_PRESENT) {
            log_t("KeyNotPresentSensor state changed: %s -> %s",
                  oldState ? "NOT_PRESENT" : "PRESENT",
                  currentState ? "NOT_PRESENT" : "PRESENT");
        }
        else if (config_.pin == gpio_pins::LIGHTS) {
            log_t("Lights sensor state changed: %s -> %s",
                  oldState ? "ON" : "OFF",
                  currentState ? "ON" : "OFF");
        }
    }

    return changed;
}

void GpioSensor::OnInterruptTriggered()
{
    log_t("%s interrupt triggered - state changed", config_.name);

    // Sensor-specific interrupt handling based on pin
    if (config_.pin == gpio_pins::LIGHTS) {
        bool currentState = readLogicalState();
        if (currentState) {
            log_t("Lights turned on - system could switch to night theme");
        } else {
            log_t("Lights turned off - system could switch to day theme");
        }
    }
    else if (config_.pin == gpio_pins::DEBUG_ERROR) {
        #ifdef CLARITY_DEBUG
        log_i("Debug error button interrupt detected - error generation handled by trigger system");
        #endif
    }
    else {
        // Generic interrupt handling for other sensors
        bool currentState = readLogicalState();
        log_t("%s current state: %s", config_.name, currentState ? "ACTIVE" : "INACTIVE");
    }
}

bool GpioSensor::readRawPinState()
{
    if (!gpioProvider_) {
        log_w("%s: GPIO provider is null, returning false", config_.name);
        return false;
    }

    // Read raw GPIO pin state
    return gpioProvider_->DigitalRead(config_.pin);
}

bool GpioSensor::readLogicalState()
{
    bool rawState = readRawPinState();

    // Convert raw state to logical state based on activeHigh configuration
    return config_.activeHigh ? rawState : !rawState;
}