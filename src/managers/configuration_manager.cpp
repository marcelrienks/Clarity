#include "managers/configuration_manager.h"
#include "providers/storage_provider.h"
#include "managers/error_manager.h"
#include "utilities/logging.h"
#include <esp32-hal-log.h>

// Static instance for singleton pattern
static ConfigurationManager* configManagerInstancePtr_ = nullptr;

// ========== Constructors and Destructor ==========

/**
 * @brief Construct ConfigurationManager and set singleton pointer
 * @details Initializes the manager but does not create storage yet
 */
ConfigurationManager::ConfigurationManager() : initialized_(false)
{
    log_v("ConfigurationManager() constructor called");

    // Set singleton instance for static access
    configManagerInstancePtr_ = this;
}

/**
 * @brief Destructor - clean up singleton pointer and storage
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
std::vector<void(*)(IConfigurationManager*)>& ConfigurationManager::GetSchemaFunctions() {
    static std::vector<void(*)(IConfigurationManager*)> functions;
    return functions;
}

/**
 * @brief Add a configuration schema function for later execution
 * @param func Function pointer that will register a component's configuration schema
 *
 * Called during static initialization by components.
 * Functions are stored and executed later when RegisterAllSchemas() is called.
 */
void ConfigurationManager::AddSchema(void(*func)(IConfigurationManager*)) {
    if (!func) {
        log_e("ConfigurationManager: Cannot add null schema function");
        return;
    }

    GetSchemaFunctions().push_back(func);
    log_d("ConfigurationManager: Schema function added (total: %zu)", GetSchemaFunctions().size());
}

// ========== Public Interface Methods ==========

/**
 * @brief Initialize the configuration manager with storage
 * @return true if initialization successful
 *
 * Creates the internal StorageProvider backend for storage operations.
 */
bool ConfigurationManager::Initialize() {
    if (storageProvider_) {
        log_w("ConfigurationManager already initialized");
        return true;
    }

    storageProvider_ = std::make_unique<StorageProvider>();
    if (!storageProvider_) {
        log_e("Failed to create StorageProvider");
        ErrorManager::Instance().ReportCriticalError("ConfigurationManager",
                                                     "Failed to create storage backend");
        return false;
    }

    log_i("ConfigurationManager initialized with storage backend");
    return true;
}

/**
 * @brief Execute all registered schema functions
 *
 * Called once during application setup to register all component schemas.
 * Uses this instance as the service, delegating to internal storage.
 */
void ConfigurationManager::RegisterAllSchemas() {
    if (!EnsureStorageReady()) {
        log_e("Cannot register schemas - storage not initialized");
        return;
    }

    if (initialized_) {
        log_w("Schemas already registered - skipping duplicate registration");
        return;
    }

    auto& functions = GetSchemaFunctions();
    size_t totalFunctions = functions.size();

    log_i("Registering %zu configuration schemas", totalFunctions);

    size_t successCount = 0;
    for (auto func : functions) {
        if (func) {
            // Pass this ConfigurationManager as the IConfigurationManager
            func(this);
            successCount++;
        }
    }

    initialized_ = true;
    log_i("Configuration registration complete: %zu/%zu schemas registered successfully",
          successCount, totalFunctions);

    // Load all configuration sections from NVS after registration
    if (!LoadAllConfigSections()) {
        log_w("Some configuration sections failed to load from storage");
    }
}

// ========== IConfigurationManager Implementation ==========

bool ConfigurationManager::RegisterConfigSection(const Config::ConfigSection& section) {
    if (!EnsureStorageReady()) return false;
    return storageProvider_->RegisterConfigSection(section);
}

std::vector<std::string> ConfigurationManager::GetRegisteredSectionNames() const {
    if (!EnsureStorageReady()) return {};
    return storageProvider_->GetRegisteredSectionNames();
}

std::optional<Config::ConfigSection> ConfigurationManager::GetConfigSection(const std::string& sectionName) const {
    if (!EnsureStorageReady()) return std::nullopt;
    return storageProvider_->GetConfigSection(sectionName);
}

bool ConfigurationManager::SaveConfigSection(const std::string& sectionName) {
    if (!EnsureStorageReady()) return false;
    return storageProvider_->SaveConfigSection(sectionName);
}

bool ConfigurationManager::LoadConfigSection(const std::string& sectionName) {
    if (!EnsureStorageReady()) return false;
    return storageProvider_->LoadConfigSection(sectionName);
}

bool ConfigurationManager::SaveAllConfigSections() {
    if (!EnsureStorageReady()) return false;
    return storageProvider_->SaveAllConfigSections();
}

bool ConfigurationManager::LoadAllConfigSections() {
    if (!EnsureStorageReady()) return false;
    return storageProvider_->LoadAllConfigSections();
}

bool ConfigurationManager::ValidateConfigValue(const std::string& fullKey, const Config::ConfigValue& value) const {
    if (!EnsureStorageReady()) return false;
    return storageProvider_->ValidateConfigValue(fullKey, value);
}

uint32_t ConfigurationManager::RegisterChangeCallback(const std::string& fullKey, ConfigChangeCallback callback) {
    if (!EnsureStorageReady()) return 0;
    return storageProvider_->RegisterChangeCallback(fullKey, callback);
}

bool ConfigurationManager::IsSchemaRegistered(const std::string& sectionName) const {
    if (!EnsureStorageReady()) return false;
    return storageProvider_->IsSchemaRegistered(sectionName);
}

// Configuration Value Helper Methods
std::string ConfigurationManager::GetTypeName(const Config::ConfigValue& value) const {
    if (!EnsureStorageReady()) return "unknown";
    return storageProvider_->GetTypeName(value);
}

bool ConfigurationManager::TypesMatch(const Config::ConfigValue& a, const Config::ConfigValue& b) const {
    if (!EnsureStorageReady()) return false;
    return storageProvider_->TypesMatch(a, b);
}

std::string ConfigurationManager::ToString(const Config::ConfigValue& value) const {
    if (!EnsureStorageReady()) return "";
    return storageProvider_->ToString(value);
}

Config::ConfigValue ConfigurationManager::FromString(const std::string& str, const Config::ConfigValue& templateValue) const {
    if (!EnsureStorageReady()) return std::monostate{};
    return storageProvider_->FromString(str, templateValue);
}

bool ConfigurationManager::IsNumeric(const Config::ConfigValue& value) const {
    if (!EnsureStorageReady()) return false;
    return storageProvider_->IsNumeric(value);
}

// Protected Implementation Methods
std::optional<Config::ConfigValue> ConfigurationManager::QueryConfigImpl(const std::string& fullKey) const {
    if (!EnsureStorageReady()) return std::nullopt;
    return storageProvider_->QueryConfigValue(fullKey);
}

bool ConfigurationManager::UpdateConfigImpl(const std::string& fullKey, const Config::ConfigValue& value) {
    if (!EnsureStorageReady()) return false;
    return storageProvider_->UpdateConfigValue(fullKey, value);
}

// ========== Private Methods ==========

/**
 * @brief Ensure storage is initialized before operations
 * @return true if storage is ready
 */
bool ConfigurationManager::EnsureStorageReady() const {
    if (!storageProvider_) {
        log_e("Storage provider not initialized - call Initialize() first");
        return false;
    }
    return true;
}