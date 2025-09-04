#pragma once

#include "interfaces/i_provider_factory.h"
#include <memory>

/**
 * @class ProviderFactory
 * @brief Concrete factory implementation for creating hardware providers
 * 
 * @details This factory creates all hardware abstraction providers used in
 * the Clarity system. It implements the IProviderFactory interface to enable
 * testability through dependency injection.
 * 
 * @design_pattern Concrete Factory (implements Abstract Factory)
 * @responsibility Hardware provider instantiation and configuration
 * @ownership All created providers are returned as unique_ptr for clear ownership
 * @thread_safety Factory methods are thread-safe for creation
 * 
 * @architecture_notes
 * - Single responsibility: Only creates providers
 * - Implements IProviderFactory for test injection
 * - Used by ManagerFactory to obtain providers
 * - Enables mock provider injection in tests
 * 
 * @example
 * @code
 * // Production usage
 * auto providerFactory = std::make_unique<ProviderFactory>();
 * auto gpioProvider = providerFactory->CreateGpioProvider();
 * auto displayProvider = providerFactory->CreateDisplayProvider();
 * auto deviceProvider = providerFactory->CreateDeviceProvider();
 * 
 * // Pass to ManagerFactory
 * auto managerFactory = std::make_unique<ManagerFactory>(providerFactory.get());
 * @endcode
 */
class ProviderFactory : public IProviderFactory
{
public:
    /// @brief Default constructor
    ProviderFactory() = default;
    
    /// @brief Default destructor
    ~ProviderFactory() override = default;
    
    // IProviderFactory implementation
    
    /// @brief Create GPIO provider for digital/analog I/O operations
    /// @return Unique pointer to configured GpioProvider instance
    /// @note Creates concrete GpioProvider with ESP32 pin configurations
    std::unique_ptr<IGpioProvider> CreateGpioProvider() override;
    
    /// @brief Create display provider for LVGL screen operations
    /// @param deviceProvider Device provider instance to use for display initialization
    /// @return Unique pointer to configured DisplayProvider instance
    /// @note Creates concrete DisplayProvider with 240x240 round display setup
    std::unique_ptr<IDisplayProvider> CreateDisplayProvider(IDeviceProvider* deviceProvider) override;
    
    /// @brief Create device provider for low-level hardware driver operations
    /// @return Unique pointer to configured DeviceProvider instance
    /// @note Creates concrete DeviceProvider for GC9A01 display driver
    std::unique_ptr<IDeviceProvider> CreateDeviceProvider() override;
};