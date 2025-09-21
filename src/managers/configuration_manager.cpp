#include "managers/configuration_manager.h"
#include "managers/error_manager.h"
#include "utilities/logging.h"
#include <esp32-hal-log.h>

// Static instance for singleton pattern
static ConfigurationManager* configManagerInstancePtr_ = nullptr;

// ========== Constructors and Destructor ==========

/**
 * @brief Construct ConfigurationManager and set singleton pointer
 * @details Initializes the manager but does not execute schemas yet
 */
ConfigurationManager::ConfigurationManager() : initialized_(false)
{
    log_v("ConfigurationManager() constructor called");

    // Set singleton instance for static access
    configManagerInstancePtr_ = this;
}

/**
 * @brief Destructor - clean up singleton pointer
 */
ConfigurationManager::~ConfigurationManager()
{
    log_v("~ConfigurationManager() destructor called");

    // Clear singleton instance if we are the current instance
    if (configManagerInstancePtr_ == this) {
        configManagerInstancePtr_ = nullptr;
    }
}

// ========== Static Methods ==========

/**
 * @brief Singleton access for manager architecture
 * @details Must be initialized by ManagerFactory before use
 */
ConfigurationManager& ConfigurationManager::Instance() {
    if (!configManagerInstancePtr_) {
        log_e("ConfigurationManager::Instance() called before initialization");
        ErrorManager::Instance().ReportCriticalError("ConfigurationManager",
                                                     "Instance() called before initialization - configuration will not function");
    }
    return *configManagerInstancePtr_;
}

/**
 * @brief Get the function collection using Construct-On-First-Use idiom
 * @return Reference to the static vector of registration functions
 *
 * This pattern ensures the vector exists before any static initialization
 * tries to register schemas, avoiding the static initialization order fiasco.
 */
std::vector<void(*)(IPreferenceService*)>& ConfigurationManager::GetSchemaFunctions() {
    static std::vector<void(*)(IPreferenceService*)> functions;
    return functions;
}

/**
 * @brief Add a configuration schema function for later execution
 * @param func Function pointer that will register a component's configuration schema
 *
 * Called during static initialization by components.
 * Functions are stored and executed later when RegisterAllSchemas() is called.
 */
void ConfigurationManager::AddSchema(void(*func)(IPreferenceService*)) {
    if (!func) {
        log_e("ConfigurationManager: Cannot add null schema function");
        return;
    }

    GetSchemaFunctions().push_back(func);
    log_d("ConfigurationManager: Schema function added (total: %zu)", GetSchemaFunctions().size());
}

// ========== Public Interface Methods ==========

/**
 * @brief Execute all registered schema functions
 * @param service Preference service to register schemas with
 *
 * Called once during application setup to register all component schemas.
 * This is the main coordination point for configuration registration.
 */
void ConfigurationManager::RegisterAllSchemas(IPreferenceService* service) {
    if (!service) {
        log_e("ConfigurationManager: Cannot register schemas - preference service is null");
        ErrorManager::Instance().ReportCriticalError("ConfigurationManager",
                                                     "Cannot register schemas - preference service is null");
        return;
    }

    if (initialized_) {
        log_w("ConfigurationManager: Schemas already registered - skipping duplicate registration");
        return;
    }

    auto& functions = GetSchemaFunctions();
    log_i("ConfigurationManager: Registering %zu configuration schemas", functions.size());

    for (auto func : functions) {
        try {
            func(service);
        } catch (...) {
            log_e("ConfigurationManager: Exception occurred during schema registration");
            ErrorManager::Instance().ReportError(ErrorLevel::ERROR, "ConfigurationManager",
                                                "Exception during schema registration");
        }
    }

    initialized_ = true;
    log_i("ConfigurationManager: All configuration schemas registered successfully");
}