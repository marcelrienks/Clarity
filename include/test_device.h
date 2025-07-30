#pragma once

#include "interfaces/i_device.h"
#include "interfaces/i_gpio_provider.h"
#include "interfaces/i_display_provider.h"
#include <memory>

/**
 * @class TestDevice
 * @brief Test implementation of IDevice for integration testing
 * 
 * @details This class provides a test implementation of the IDevice interface
 * that accepts mock providers for GPIO and display operations. It enables
 * integration testing of the full system without requiring actual ESP32 hardware.
 * 
 * @usage Used in integration tests to inject mock providers and test real
 * component interactions with controlled hardware behavior.
 * 
 * @design_pattern Dependency Injection - accepts providers through constructor
 * @thread_safety Same as injected providers
 * @memory_usage Minimal - no display buffers or hardware initialization
 */
class TestDevice : public IDevice
{
public:
    /// @brief Constructor with provider injection
    /// @param gpioProvider Mock GPIO provider for testing
    /// @param displayProvider Mock display provider for testing
    TestDevice(std::unique_ptr<IGpioProvider> gpioProvider, 
               std::unique_ptr<IDisplayProvider> displayProvider);

    // Core Interface Methods
    /// @brief Prepare device for testing - lightweight initialization
    void prepare() override;
    
    // Provider Access Methods
    /// @brief Get injected GPIO provider
    /// @return Pointer to GPIO provider instance
    IGpioProvider* getGpioProvider() override;
    
    /// @brief Get injected display provider
    /// @return Pointer to display provider instance
    IDisplayProvider* getDisplayProvider() override;

private:
    // Provider instances
    std::unique_ptr<IGpioProvider> gpioProvider_;
    std::unique_ptr<IDisplayProvider> displayProvider_;
};