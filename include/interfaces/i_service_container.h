#pragma once

// System/Library Includes
#include <memory>
#include <functional>
#include <typeinfo>

/**
 * @interface IServiceContainer
 * @brief Interface for dependency injection container
 * 
 * @details This interface abstracts dependency injection container operations,
 * providing service registration, resolution, and lifecycle management.
 * Implementations should support both singleton and transient service lifetimes,
 * automatic dependency resolution, and type-safe service retrieval.
 * 
 * @design_pattern Interface Segregation - Focused on DI container operations only
 * @design_pattern Service Locator - Centralized service resolution
 * @testability Enables mocking for unit tests with test service configurations
 * @dependency_injection Core abstraction for the entire DI system
 * 
 * @lifecycle_support:
 * - Singleton: One instance per container lifetime
 * - Transient: New instance on every resolution
 * - Future: Scoped (one instance per scope)
 */
class IServiceContainer
{
public:
    virtual ~IServiceContainer() = default;

    // Service Registration Methods
    
    /**
     * @brief Register a singleton service with a factory function
     * @tparam TInterface Interface type to register
     * @param factory Function that creates the service instance
     * @details Singleton services are created once and reused for all subsequent requests
     */
    template<typename TInterface>
    void registerSingleton(std::function<std::unique_ptr<TInterface>()> factory)
    {
        registerSingletonImpl(typeid(TInterface), 
                             [factory]() -> std::unique_ptr<void> {
                                 return std::unique_ptr<void>(factory().release());
                             });
    }

    /**
     * @brief Register a transient service with a factory function
     * @tparam TInterface Interface type to register
     * @param factory Function that creates service instances with container access
     * @details Transient services are created new on every resolution request
     */
    template<typename TInterface>
    void registerTransient(std::function<std::unique_ptr<TInterface>(IServiceContainer*)> factory)
    {
        registerTransientImpl(typeid(TInterface),
                             [factory](IServiceContainer* container) -> std::unique_ptr<void> {
                                 return std::unique_ptr<void>(factory(container).release());
                             });
    }

    // Service Resolution Methods
    
    /**
     * @brief Resolve a service instance by interface type
     * @tparam TInterface Interface type to resolve
     * @return Pointer to service instance (managed by container)
     * @details Returns the registered service instance. For singletons, returns
     * the same instance. For transients, returns a new instance.
     */
    template<typename TInterface>
    TInterface* resolve()
    {
        return static_cast<TInterface*>(resolveImpl(typeid(TInterface)));
    }

    /**
     * @brief Create a new service instance (always creates new instance)
     * @tparam TInterface Interface type to create
     * @return Unique pointer to new service instance (caller owns)
     * @details Always creates a new instance regardless of registration type.
     * Useful for creating objects that need to be owned by the caller.
     */
    template<typename TInterface>
    std::unique_ptr<TInterface> create()
    {
        auto ptr = createImpl(typeid(TInterface));
        return std::unique_ptr<TInterface>(static_cast<TInterface*>(ptr.release()));
    }

    // Container Management Methods
    
    /**
     * @brief Clear all registrations and cached instances
     * @details Useful for testing scenarios where clean container state is needed
     */
    virtual void clear() = 0;

    /**
     * @brief Check if a service type is registered
     * @tparam TInterface Interface type to check
     * @return True if service is registered, false otherwise
     */
    template<typename TInterface>
    bool isRegistered() const
    {
        return isRegisteredImpl(typeid(TInterface));
    }

protected:
    // Protected implementation methods (to be implemented by concrete classes)
    
    /**
     * @brief Implementation method for singleton registration
     * @param type Type information for the service interface
     * @param factory Type-erased factory function
     */
    virtual void registerSingletonImpl(const std::type_info& type, 
                                      std::function<std::unique_ptr<void>()> factory) = 0;

    /**
     * @brief Implementation method for transient registration
     * @param type Type information for the service interface
     * @param factory Type-erased factory function with container access
     */
    virtual void registerTransientImpl(const std::type_info& type,
                                      std::function<std::unique_ptr<void>(IServiceContainer*)> factory) = 0;

    /**
     * @brief Implementation method for service resolution
     * @param type Type information for the service interface
     * @return Pointer to service instance (type-erased)
     */
    virtual void* resolveImpl(const std::type_info& type) = 0;

    /**
     * @brief Implementation method for service creation
     * @param type Type information for the service interface
     * @return Unique pointer to new instance (type-erased)
     */
    virtual std::unique_ptr<void> createImpl(const std::type_info& type) = 0;

    /**
     * @brief Implementation method for registration checking
     * @param type Type information for the service interface
     * @return True if service is registered, false otherwise
     */
    virtual bool isRegisteredImpl(const std::type_info& type) const = 0;
};