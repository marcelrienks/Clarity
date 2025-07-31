#include "test_device.h"

// Constructors
TestDevice::TestDevice(std::unique_ptr<IGpioProvider> gpioProvider, 
                       std::unique_ptr<IDisplayProvider> displayProvider)
    : gpioProvider_(std::move(gpioProvider)), displayProvider_(std::move(displayProvider))
{
    // Minimal initialization for testing - no hardware setup required
}

// Core Interface Methods
void TestDevice::prepare()
{
    // Lightweight preparation for testing
    // No LVGL initialization, display buffers, or hardware configuration needed
    // The mock providers handle all display/GPIO operations
}

