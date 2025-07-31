#pragma once

#include <memory>
#include "system/service_container.h"
#include "clarity_application.h"

// Forward declarations
class IComponentFactory;

/**
 * @brief Bootstrap class to manage application lifecycle and eliminate global state
 * 
 * This class encapsulates the service container and application instance,
 * providing proper object lifetime management and eliminating the need
 * for global variables in main.cpp.
 */
class ClarityBootstrap {
private:
    std::unique_ptr<ServiceContainer> serviceContainer_;
    std::unique_ptr<ClarityApplication> application_;

public:
    /**
     * @brief Initialize the service container and register all dependencies
     */
    void initialize();
    
    /**
     * @brief Run the main application loop
     */
    void run();
    
    /**
     * @brief Clean shutdown of application and services
     */
    void shutdown();

private:
    /**
     * @brief Register production components with the component factory
     */
    void registerProductionComponents(IComponentFactory* componentFactory);
    
    /**
     * @brief Register all services in the dependency injection container
     */
    void registerServices();
};