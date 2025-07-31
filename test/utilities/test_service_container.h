#pragma once

// System/Library Includes
#include <memory>
#include <map>
#include <string>
#include <typeinfo>
#include <stdexcept>

// Project Includes
#include "interfaces/i_service_container.h"
#include "system/service_container.h"

/**
 * @class TestServiceContainer
 * @brief Specialized service container for unit testing with mock support
 * 
 * @details Extends the base service container with testing-specific functionality
 * including easy mock registration, test isolation, and verification capabilities.
 * Designed to make unit testing with dependency injection simple and reliable.
 * 
 * @testing_features
 * - Easy mock service registration
 * - Test isolation with reset functionality
 * - Type-safe service resolution
 * - Integration with existing mock implementations
 */
class TestServiceContainer : public ServiceContainer
{
public:
    TestServiceContainer();
    virtual ~TestServiceContainer() = default;

    /**
     * @brief Register a mock implementation for an interface
     * @tparam Interface The interface type to mock
     * @tparam Mock The mock implementation type
     * @param mock Unique pointer to the mock instance
     */
    template<typename Interface, typename Mock>
    void registerMock(std::unique_ptr<Mock> mock)
    {
        static_assert(std::is_base_of_v<Interface, Mock>, 
                     "Mock must implement the Interface");
        
        // Store the mock as a singleton in the base container
        Interface* mockPtr = mock.release();
        registerSingleton<Interface>([mockPtr]() {
            return std::unique_ptr<Interface>(mockPtr);
        });
    }

    /**
     * @brief Register a mock factory function for an interface
     * @tparam Interface The interface type to mock
     * @param factory Function that creates mock instances
     */
    template<typename Interface>
    void registerMockFactory(std::function<std::unique_ptr<Interface>()> factory)
    {
        registerSingleton<Interface>(factory);
    }

    /**
     * @brief Reset all registered services for test isolation
     */
    void reset();

    /**
     * @brief Check if a service type is registered
     * @tparam T The service type to check
     * @return True if service is registered, false otherwise
     */
    template<typename T>
    bool isRegistered() const
    {
        return hasService<T>();
    }

    /**
     * @brief Get the number of registered services
     * @return Count of registered services
     */
    size_t getServiceCount() const;

private:
    // Track registered service types for reset functionality
    std::vector<std::string> registeredTypes_;
    
    void trackRegistration(const std::string& typeName);
};