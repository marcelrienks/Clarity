#pragma once

// System/Library Includes
#include <memory>
#include <functional>
#include <string>
#include <type_traits>

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
        registerSingletonImpl(getTypeId<TInterface>(), 
                             [factory]() -> void* {
                                 return factory().release();
                             },
                             [](void* ptr) {
                                 delete static_cast<TInterface*>(ptr);
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
        registerTransientImpl(getTypeId<TInterface>(),
                             [factory](IServiceContainer* container) -> void* {
                                 return factory(container).release();
                             },
                             [](void* ptr) {
                                 delete static_cast<TInterface*>(ptr);
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
        return static_cast<TInterface*>(resolveImpl(getTypeId<TInterface>()));
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
        auto ptr = createImpl(getTypeId<TInterface>());
        return std::unique_ptr<TInterface>(static_cast<TInterface*>(ptr));
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
        return isRegisteredImpl(getTypeId<TInterface>());
    }

protected:
    // Type ID system without RTTI
    template<typename T>
    static const char* getTypeId()
    {
        return __PRETTY_FUNCTION__; // GCC/Clang specific - includes template parameter
    }
    
    // Protected implementation methods (to be implemented by concrete classes)
    
    /**
     * @brief Implementation method for singleton registration
     * @param typeId String identifier for the service interface
     * @param factory Type-erased factory function returning raw pointer
     * @param deleter Type-erased deleter function
     */
    virtual void registerSingletonImpl(const char* typeId, 
                                      std::function<void*()> factory,
                                      std::function<void(void*)> deleter) = 0;

    /**
     * @brief Implementation method for transient registration
     * @param typeId String identifier for the service interface
     * @param factory Type-erased factory function with container access
     * @param deleter Type-erased deleter function
     */
    virtual void registerTransientImpl(const char* typeId,
                                      std::function<void*(IServiceContainer*)> factory,
                                      std::function<void(void*)> deleter) = 0;

    /**
     * @brief Implementation method for service resolution
     * @param typeId String identifier for the service interface
     * @return Pointer to service instance (type-erased)
     */
    virtual void* resolveImpl(const char* typeId) = 0;

    /**
     * @brief Implementation method for service creation
     * @param typeId String identifier for the service interface
     * @return Raw pointer to new instance (type-erased, caller must manage)
     */
    virtual void* createImpl(const char* typeId) = 0;

    /**
     * @brief Implementation method for registration checking
     * @param typeId String identifier for the service interface
     * @return True if service is registered, false otherwise
     */
    virtual bool isRegisteredImpl(const char* typeId) const = 0;
};