#include "config/config_registry.h"
#include "utilities/logging.h"
#include <algorithm>

/**
 * @brief Get the registrar collection using Construct-On-First-Use idiom
 * @return Reference to the static vector of registration functions
 *
 * This pattern ensures the vector exists before any static initialization
 * tries to register schemas, avoiding the static initialization order fiasco.
 */
std::vector<ConfigRegistry::RegistrationFunc>& ConfigRegistry::GetRegistrars() {
    static std::vector<RegistrationFunc> registrars;
    return registrars;
}

/**
 * @brief Register a configuration schema function
 * @param func Function that registers a component's configuration
 * @return true for static initialization chaining
 *
 * Called during static initialization by components using REGISTER_CONFIG_SCHEMA macro.
 * The function is stored and will be executed later by RegisterAllSchemas().
 */
bool ConfigRegistry::RegisterSchema(RegistrationFunc func) {
    GetRegistrars().push_back(func);
    return true;  // Return value used for static initialization
}

/**
 * @brief Execute all registered schema functions
 * @param service Preference service to register schemas with
 *
 * Called once during application setup to register all component schemas.
 * This is the only place where all schemas are processed, and it doesn't
 * need to know what components exist.
 */
void ConfigRegistry::RegisterAllSchemas(IPreferenceService* service) {
    if (!service) {
        log_e("Cannot register schemas - preference service is null");
        return;
    }

    auto& registrars = GetRegistrars();
    log_i("Registering %zu configuration schemas", registrars.size());

    for (auto& registrar : registrars) {
        try {
            registrar(service);
        } catch (const std::exception& e) {
            log_e("Exception during schema registration: %s", e.what());
        }
    }

    log_i("All configuration schemas registered successfully");
}

/**
 * @brief Get count of registered schemas
 * @return Number of schemas registered
 *
 * Useful for debugging and verifying that components are registering.
 */
size_t ConfigRegistry::GetRegisteredCount() {
    return GetRegistrars().size();
}

/**
 * @brief Reset the registry (for testing only)
 *
 * Clears all registered schemas. Should only be used in test environments
 * to ensure test isolation.
 */
void ConfigRegistry::Reset() {
    GetRegistrars().clear();
    log_d("ConfigRegistry reset - all schemas cleared");
}