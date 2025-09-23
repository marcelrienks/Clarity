#pragma once

#include "interfaces/i_configuration_manager.h"
#include "interfaces/i_storage_provider.h"
#include <vector>
#include <memory>

/**
 * @class ConfigurationManager
 * @brief Unified configuration manager providing single interface for all components
 *
 * @details This manager serves as the single point of contact for all configuration
 * operations. It combines schema registration coordination with configuration access,
 * providing a consistent API for components while using IStorageProvider for
 * actual storage persistence via IStorageProvider.
 *
 * @design_pattern Manager Pattern with Internal Storage Implementation
 * @memory_management Singleton with ownership of storage backend
 * @thread_safety Delegates thread safety to PreferenceStorage
 *
 * @lifecycle:
 * 1. Static Initialization: Components register schemas via AddSchema()
 * 2. Main Initialization: Instance created with storage backend
 * 3. Schema Registration: RegisterAllSchemas() executes collected functions
 * 4. Runtime: Components use ConfigurationManager for all config operations
 *
 * @context This manager is the single interface for configuration. Components
 * should use ConfigurationManager exclusively, never PreferenceStorage directly.
 */
class ConfigurationManager : public IConfigurationManager {
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
    static void AddSchema(void(*func)(IConfigurationManager*));

    // ========== Public Interface Methods ==========

    /**
     * @brief Initialize the configuration manager with storage
     * @return true if initialization successful
     *
     * Creates the internal storage backend and prepares for configuration operations.
     * Must be called before any configuration operations.
     */
    bool Initialize();

    /**
     * @brief Execute all registered schema functions
     *
     * Called once during application setup to register all component schemas.
     * Uses this instance as the service for schema registration.
     */
    void RegisterAllSchemas();

    // ========== IPreferenceService Implementation ==========

    // Dynamic Configuration Methods
    bool RegisterConfigSection(const Config::ConfigSection& section) override;
    std::vector<std::string> GetRegisteredSectionNames() const override;
    std::optional<Config::ConfigSection> GetConfigSection(const std::string& sectionName) const override;
    bool SaveConfigSection(const std::string& sectionName) override;
    bool LoadConfigSection(const std::string& sectionName) override;
    bool SaveAllConfigSections() override;
    bool LoadAllConfigSections() override;
    bool ValidateConfigValue(const std::string& fullKey, const Config::ConfigValue& value) const override;
    uint32_t RegisterChangeCallback(const std::string& fullKey, ConfigChangeCallback callback) override;
    bool IsSchemaRegistered(const std::string& sectionName) const override;

    // Configuration Value Helper Methods
    std::string GetTypeName(const Config::ConfigValue& value) const override;
    bool TypesMatch(const Config::ConfigValue& a, const Config::ConfigValue& b) const override;
    std::string ToString(const Config::ConfigValue& value) const override;
    Config::ConfigValue FromString(const std::string& str, const Config::ConfigValue& templateValue) const override;
    bool IsNumeric(const Config::ConfigValue& value) const override;

protected:
    // Implementation Methods for Template Access
    std::optional<Config::ConfigValue> QueryConfigImpl(const std::string& fullKey) const override;
    bool UpdateConfigImpl(const std::string& fullKey, const Config::ConfigValue& value) override;

private:
    // ========== Private Methods ==========

    /**
     * @brief Get the function collection (Construct-On-First-Use)
     * @return Reference to the static vector of registration functions
     *
     * This pattern avoids the static initialization order fiasco by
     * ensuring the vector exists before any component tries to register.
     */
    static std::vector<void(*)(IConfigurationManager*)>& GetSchemaFunctions();

    /**
     * @brief Ensure storage is initialized before operations
     * @return true if storage is ready
     */
    bool EnsureStorageReady() const;

    // ========== Private Data Members ==========
    std::unique_ptr<IStorageProvider> storageProvider_;  ///< Internal storage backend
    bool initialized_ = false;                    ///< Registration state tracker
};