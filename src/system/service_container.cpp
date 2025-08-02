#include "system/service_container.h"
#include <stdexcept>
#include <sstream>

void ServiceContainer::clear()
{
    services_.clear();
}

void ServiceContainer::registerSingletonImpl(const char *typeId, 
                                            std::function<void *()> factory,
                                            std::function<void(void *)> deleter)
{
    std::string key(typeId);
    services_.emplace(std::piecewise_construct,
                     std::forward_as_tuple(key),
                     std::forward_as_tuple(ServiceLifetime::Singleton, std::move(factory), std::move(deleter)));
}

void ServiceContainer::registerTransientImpl(const char *typeId,
                                            std::function<void *(ServiceContainer *)> factory,
                                            std::function<void(void *)> deleter)
{
    std::string key(typeId);
    services_.emplace(std::piecewise_construct,
                     std::forward_as_tuple(key),
                     std::forward_as_tuple(ServiceLifetime::Transient, std::move(factory), std::move(deleter)));
}

void *ServiceContainer::resolveImpl(const char *typeId)
{
    auto &registration = getRegistration(typeId);
    
    if (registration.lifetime == ServiceLifetime::Singleton) {
        // For singletons, create instance if it doesn't exist and cache it
        if (!registration.singletonInstance) {
            registration.singletonInstance = registration.singletonFactory();
        }
        return registration.singletonInstance;
    } else {
        // For transients, we cannot return a managed pointer from resolve()
        // This violates the ownership model - resolve() should only work with singletons
        throw std::runtime_error("Cannot resolve transient service via resolve() - use create() instead for proper ownership management");
    }
}

void *ServiceContainer::createImpl(const char *typeId)
{
    auto &registration = getRegistration(typeId);
    
    if (registration.lifetime == ServiceLifetime::Singleton) {
        // For singletons via create(), always create a new instance (don't use cached)
        return registration.singletonFactory();
    } else {
        // For transients, create new instance with container access
        return registration.transientFactory(this);
    }
}

bool ServiceContainer::isRegisteredImpl(const char *typeId) const
{
    std::string key(typeId);
    return services_.find(key) != services_.end();
}

ServiceContainer::ServiceRegistration &ServiceContainer::getRegistration(const char *typeId)
{
    std::string key(typeId);
    auto it = services_.find(key);
    
    if (it == services_.end()) {
        std::ostringstream oss;
        oss << "Service not registered: " << typeId;
        throw std::runtime_error(oss.str());
    }
    
    return it->second;
}

const ServiceContainer::ServiceRegistration &ServiceContainer::getRegistration(const char *typeId) const
{
    std::string key(typeId);
    auto it = services_.find(key);
    
    if (it == services_.end()) {
        std::ostringstream oss;
        oss << "Service not registered: " << typeId;
        throw std::runtime_error(oss.str());
    }
    
    return it->second;
}

ServiceContainer& ServiceContainer::getInstance() {
    static ServiceContainer instance;
    return instance;
}

bool ServiceContainer::hasServiceImpl(const std::string& name) const {
    return testServices_.find(name) != testServices_.end();
}

void ServiceContainer::unregisterServiceImpl(const std::string& name) {
    testServices_.erase(name);
}