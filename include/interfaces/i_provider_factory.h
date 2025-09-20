#pragma once

#include <memory>

// Forward declarations
class IGpioProvider;
class IDisplayProvider;
class DeviceProvider;

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
    virtual ~IProviderFactory() = default;
    
    virtual std::unique_ptr<IGpioProvider> CreateGpioProvider() = 0;
    
    virtual std::unique_ptr<IDisplayProvider> CreateDisplayProvider(DeviceProvider* deviceProvider) = 0;
    
    virtual std::unique_ptr<DeviceProvider> CreateDeviceProvider() = 0;
};