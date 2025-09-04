#pragma once

#include <memory>

// Forward declarations
class IGpioProvider;
class IDisplayProvider;
class IDeviceProvider;

/**
 * @interface IProviderFactory
 * @brief Factory interface for creating hardware abstraction providers
 * 
 * @details This interface enables testability by allowing mock provider injection
 * in test scenarios. The concrete ProviderFactory implementation creates real
 * hardware providers, while test code can provide mock implementations.
 * 
 * @design_pattern Abstract Factory Pattern
 * @testability Enables dependency injection of mock providers
 * @memory All methods return unique_ptr for clear ownership transfer
 * 
 * @usage
 * Production:
 * @code
 * auto providerFactory = std::make_unique<ProviderFactory>();
 * auto managerFactory = std::make_unique<ManagerFactory>(providerFactory.get());
 * @endcode
 * 
 * Testing:
 * @code
 * auto mockProviderFactory = std::make_unique<MockProviderFactory>();
 * auto managerFactory = std::make_unique<ManagerFactory>(mockProviderFactory.get());
 * @endcode
 */
class IProviderFactory
{
public:
    /// @brief Virtual destructor for interface
    virtual ~IProviderFactory() = default;
    
    /// @brief Create GPIO provider for digital/analog I/O
    /// @return Unique pointer to IGpioProvider instance
    virtual std::unique_ptr<IGpioProvider> CreateGpioProvider() = 0;
    
    /// @brief Create display provider for LVGL operations
    /// @param deviceProvider Device provider instance to use for display initialization
    /// @return Unique pointer to IDisplayProvider instance
    virtual std::unique_ptr<IDisplayProvider> CreateDisplayProvider(IDeviceProvider* deviceProvider) = 0;
    
    /// @brief Create device provider for hardware driver operations
    /// @return Unique pointer to IDeviceProvider instance
    virtual std::unique_ptr<IDeviceProvider> CreateDeviceProvider() = 0;
};