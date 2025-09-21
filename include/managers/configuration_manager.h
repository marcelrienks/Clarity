#pragma once

#include "interfaces/i_preference_service.h"
#include <vector>
#include <memory>

/**
 * @class ConfigurationManager
 * @brief Manager for component configuration schema registration and coordination
 *
 * @details This manager enables components to register their configuration schemas
 * automatically at program startup without main.cpp knowing about them.
 * Uses static initialization to collect all schemas before main() runs.
 *
 * @design_pattern Manager Pattern with Self-Registration
 * @memory_management Singleton with static storage for registration functions
 * @thread_safety Uses Construct-On-First-Use idiom for initialization order safety
 *
 * @lifecycle:
 * 1. Static Initialization: Components register schemas via AddSchema()
 * 2. Main Initialization: Instance created and RegisterAllSchemas() called
 * 3. Runtime: Manager maintains registration state
 *
 * @context This manager coordinates configuration schema registration across
 * all components. It ensures that configuration sections are available in the
 * Config Panel regardless of whether components are currently active.
 */
class ConfigurationManager {
public:
    // ========== Constructors and Destructor ==========
    ConfigurationManager();
    ConfigurationManager(const ConfigurationManager&) = delete;
    ConfigurationManager& operator=(const ConfigurationManager&) = delete;
    ~ConfigurationManager();

    // ========== Static Methods ==========
    static ConfigurationManager& Instance();

    /**
     * @brief Add a configuration schema function (static interface)
     * @param func Function pointer that registers a component's configuration
     *
     * Called automatically during static initialization by components.
     * The registered functions are executed later by RegisterAllSchemas().
     */
    static void AddSchema(void(*func)(IPreferenceService*));

    // ========== Public Interface Methods ==========

    /**
     * @brief Execute all registered schema functions
     * @param service Preference service to register schemas with
     *
     * Called once during application setup to register all component schemas.
     * This is the main coordination point for configuration registration.
     */
    void RegisterAllSchemas(IPreferenceService* service);

private:
    // ========== Private Methods ==========

    /**
     * @brief Get the function collection (Construct-On-First-Use)
     * @return Reference to the static vector of registration functions
     *
     * This pattern avoids the static initialization order fiasco by
     * ensuring the vector exists before any component tries to register.
     */
    static std::vector<void(*)(IPreferenceService*)>& GetSchemaFunctions();

    // ========== Private Data Members ==========
    bool initialized_ = false;
};