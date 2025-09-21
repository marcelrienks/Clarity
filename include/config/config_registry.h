#pragma once

#include "interfaces/i_preference_service.h"
#include <functional>
#include <vector>
#include <memory>

/**
 * @class ConfigRegistry
 * @brief Self-registering configuration schema registry
 *
 * @details Enables components to register their configuration schemas
 * automatically at program startup without main.cpp knowing about them.
 * Uses static initialization to collect all schemas before main() runs.
 *
 * @pattern Self-Registration Pattern
 * @thread_safety Uses Construct-On-First-Use idiom for initialization order safety
 */
class ConfigRegistry {
public:
    using RegistrationFunc = std::function<void(IPreferenceService*)>;

    /**
     * @brief Register a configuration schema function
     * @param func Function that registers a component's configuration
     * @return true for static initialization chaining
     *
     * Called automatically during static initialization by components.
     * The registered functions are executed later by RegisterAllSchemas().
     */
    static bool RegisterSchema(RegistrationFunc func);

    /**
     * @brief Execute all registered schema functions
     * @param service Preference service to register schemas with
     *
     * Called once in main.cpp to register all collected schemas.
     * main.cpp doesn't need to know what schemas exist.
     */
    static void RegisterAllSchemas(IPreferenceService* service);

    /**
     * @brief Get count of registered schemas
     * @return Number of schemas registered
     *
     * Useful for debugging and testing.
     */
    static size_t GetRegisteredCount();

    /**
     * @brief Reset the registry (for testing only)
     *
     * Clears all registered schemas. Only use in test environments.
     */
    static void Reset();

private:
    /**
     * @brief Get the registrar collection (Construct-On-First-Use)
     * @return Reference to the static vector of registration functions
     *
     * This pattern avoids the static initialization order fiasco by
     * ensuring the vector exists before any component tries to register.
     */
    static std::vector<RegistrationFunc>& GetRegistrars();
};

/**
 * @brief Macro for component self-registration
 *
 * Use this in component .cpp files to automatically register schemas:
 * REGISTER_CONFIG_SCHEMA(MyComponent)
 *
 * The component must have a static method:
 * static void MyComponent::RegisterConfigSchema(IPreferenceService* service)
 */
#define REGISTER_CONFIG_SCHEMA(ComponentClass) \
    namespace { \
        static bool ComponentClass##_registered = \
            ConfigRegistry::RegisterSchema( \
                [](IPreferenceService* svc) { \
                    ComponentClass::RegisterConfigSchema(svc); \
                } \
            ); \
    }
