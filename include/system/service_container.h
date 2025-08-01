#pragma once

// System/Library Includes
#include <memory>
#include <unordered_map>
#include <functional>
#include <string>

// Project Includes

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
class ServiceContainer
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
        std::function<void*(ServiceContainer*)> transientFactory;
        std::function<void(void*)> deleter;
        void* singletonInstance; ///< Cached singleton instance (raw pointer)
        
        ServiceRegistration(ServiceLifetime lt, 
                          std::function<void*()> factory,
                          std::function<void(void*)> del)
            : lifetime(lt), singletonFactory(std::move(factory)), deleter(std::move(del)), singletonInstance(nullptr) {}
            
        ServiceRegistration(ServiceLifetime lt,
                          std::function<void*(ServiceContainer*)> factory,
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
    
    // Public interface implementation
    void clear();
    
    // Template methods for type-safe registration and resolution
    template<typename T>
    void registerSingleton(std::function<std::unique_ptr<T>()> factory) {
        // Use explicit string ID instead of RTTI/template magic
        registerSingleton(getTypeId<T>(), factory);
    }
    
    template<typename T>
    T* resolve() {
        return static_cast<T*>(resolveImpl(getTypeId<T>()));
    }
    
    // Explicit registration with string ID
    template<typename T>
    void registerSingleton(const char* typeId, std::function<std::unique_ptr<T>()> factory) {
        registerSingletonImpl(typeId, 
                             [factory]() -> void* { return factory().release(); },
                             [](void* ptr) { delete static_cast<T*>(ptr); });
    }

private:
    // Type ID generation - use simple class name strings
    template<typename T>
    constexpr const char* getTypeId() {
        // This will be specialized for each type
        return "UnknownType";
    }

protected:
    // Protected implementation methods
    void registerSingletonImpl(const char* typeId, 
                              std::function<void*()> factory,
                              std::function<void(void*)> deleter);
                              
    void registerTransientImpl(const char* typeId,
                              std::function<void*(ServiceContainer*)> factory,
                              std::function<void(void*)> deleter);
                              
    void* resolveImpl(const char* typeId);
    
    void* createImpl(const char* typeId);
    
    bool isRegisteredImpl(const char* typeId) const;

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

// Forward declarations for specializations
class Device;
class IGpioProvider;
class IDisplayProvider;
class IStyleService;
class IPreferenceService;
class IPanelService;
class ITriggerService;

// Template specializations for type IDs
template<> constexpr const char* ServiceContainer::getTypeId<Device>() { return "Device"; }
template<> constexpr const char* ServiceContainer::getTypeId<IGpioProvider>() { return "IGpioProvider"; }
template<> constexpr const char* ServiceContainer::getTypeId<IDisplayProvider>() { return "IDisplayProvider"; }
template<> constexpr const char* ServiceContainer::getTypeId<IStyleService>() { return "IStyleService"; }
template<> constexpr const char* ServiceContainer::getTypeId<IPreferenceService>() { return "IPreferenceService"; }
template<> constexpr const char* ServiceContainer::getTypeId<IPanelService>() { return "IPanelService"; }
template<> constexpr const char* ServiceContainer::getTypeId<ITriggerService>() { return "ITriggerService"; }