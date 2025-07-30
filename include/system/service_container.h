#pragma once

// System/Library Includes
#include <memory>
#include <unordered_map>
#include <functional>
#include <string>

// Project Includes
#include "interfaces/i_service_container.h"

/**
 * @class ServiceContainer
 * @brief Concrete implementation of dependency injection container
 * 
 * @details This class provides a full-featured dependency injection container
 * supporting singleton and transient service lifetimes, automatic dependency 
 * resolution, and type-safe service retrieval. Uses std::type_index for 
 * efficient service lookup and std::unordered_map for O(1) registration/resolution.
 * 
 * @design_pattern Dependency Injection Container
 * @thread_safety Not thread-safe - external synchronization required for multi-threaded use
 * @memory_management Container manages singleton instance lifetimes, caller manages transient instances
 * 
 * @performance:
 * - Registration: O(1) average case
 * - Resolution: O(1) average case for singletons, O(1) + creation time for transients
 * - Memory: Proportional to number of registered services + singleton instances
 */
class ServiceContainer : public IServiceContainer
{
private:
    /**
     * @enum ServiceLifetime
     * @brief Enumeration of supported service lifetimes
     */
    enum class ServiceLifetime
    {
        Singleton,  ///< Single instance shared across all resolutions
        Transient   ///< New instance created on each resolution
    };
    
    /**
     * @struct ServiceRegistration
     * @brief Internal structure to store service registration information
     */
    struct ServiceRegistration
    {
        ServiceLifetime lifetime;
        std::function<void*()> singletonFactory;
        std::function<void*(IServiceContainer*)> transientFactory;
        std::function<void(void*)> deleter;
        void* singletonInstance; ///< Cached singleton instance (raw pointer)
        
        ServiceRegistration(ServiceLifetime lt, 
                          std::function<void*()> factory,
                          std::function<void(void*)> del)
            : lifetime(lt), singletonFactory(std::move(factory)), deleter(std::move(del)), singletonInstance(nullptr) {}
            
        ServiceRegistration(ServiceLifetime lt,
                          std::function<void*(IServiceContainer*)> factory,
                          std::function<void(void*)> del)
            : lifetime(lt), transientFactory(std::move(factory)), deleter(std::move(del)), singletonInstance(nullptr) {}
            
        ~ServiceRegistration() {
            if (singletonInstance && deleter) {
                deleter(singletonInstance);
            }
        }
        
        // Make non-copyable due to raw pointer management
        ServiceRegistration(const ServiceRegistration&) = delete;
        ServiceRegistration& operator=(const ServiceRegistration&) = delete;
        
        // Allow moving
        ServiceRegistration(ServiceRegistration&& other) noexcept
            : lifetime(other.lifetime)
            , singletonFactory(std::move(other.singletonFactory))
            , transientFactory(std::move(other.transientFactory))
            , deleter(std::move(other.deleter))
            , singletonInstance(other.singletonInstance)
        {
            other.singletonInstance = nullptr; // Transfer ownership
        }
        
        ServiceRegistration& operator=(ServiceRegistration&& other) noexcept
        {
            if (this != &other) {
                // Clean up existing instance
                if (singletonInstance && deleter) {
                    deleter(singletonInstance);
                }
                
                lifetime = other.lifetime;
                singletonFactory = std::move(other.singletonFactory);
                transientFactory = std::move(other.transientFactory);
                deleter = std::move(other.deleter);
                singletonInstance = other.singletonInstance;
                other.singletonInstance = nullptr;
            }
            return *this;
        }
    };
    
    /// Map of service type to registration information
    std::unordered_map<std::string, ServiceRegistration> services_;
    
public:
    ServiceContainer() = default;
    virtual ~ServiceContainer() = default;
    
    // Disable copy/move to maintain singleton instance integrity
    ServiceContainer(const ServiceContainer&) = delete;
    ServiceContainer& operator=(const ServiceContainer&) = delete;
    ServiceContainer(ServiceContainer&&) = delete;
    ServiceContainer& operator=(ServiceContainer&&) = delete;
    
    // IServiceContainer interface implementation
    void clear() override;

protected:
    // Protected implementation methods from IServiceContainer
    void registerSingletonImpl(const char* typeId, 
                              std::function<void*()> factory,
                              std::function<void(void*)> deleter) override;
                              
    void registerTransientImpl(const char* typeId,
                              std::function<void*(IServiceContainer*)> factory,
                              std::function<void(void*)> deleter) override;
                              
    void* resolveImpl(const char* typeId) override;
    
    void* createImpl(const char* typeId) override;
    
    bool isRegisteredImpl(const char* typeId) const override;

private:
    /**
     * @brief Get service registration by type, throwing if not found
     * @param typeId String identifier for the service
     * @return Reference to service registration
     * @throws std::runtime_error if service is not registered
     */
    ServiceRegistration& getRegistration(const char* typeId);
    
    /**
     * @brief Get service registration by type (const version)
     * @param typeId String identifier for the service
     * @return Const reference to service registration
     * @throws std::runtime_error if service is not registered
     */
    const ServiceRegistration& getRegistration(const char* typeId) const;
};