#pragma once

#include "interfaces/i_preference_service.h"
#include "interfaces/i_dynamic_config_service.h"
#include "config/config_types.h"
#include "utilities/types.h"
#include <functional>
#include <vector>
#include <string>

/**
 * @class ConfigMigrationService
 * @brief Handles migration from legacy configuration to dynamic configuration system
 *
 * @details This service provides a controlled migration path from the hardcoded
 * Configs struct to the dynamic configuration system. It supports:
 * - Gradual component migration
 * - Live configuration updates
 * - Rollback capabilities
 * - Data integrity validation
 *
 * @design_pattern Service with Strategy Pattern for migration steps
 * @thread_safety Thread-safe migration operations
 * @data_integrity Ensures no configuration loss during migration
 */
class ConfigMigrationService
{
public:
    /**
     * @enum MigrationPhase
     * @brief Current phase of the migration process
     */
    enum class MigrationPhase
    {
        Legacy,           ///< Using legacy configuration only
        Transitional,     ///< Both systems active, dynamic taking precedence
        Dynamic,          ///< Full dynamic configuration active
        Complete          ///< Migration complete, legacy system removed
    };

    /**
     * @struct MigrationStep
     * @brief Individual migration step with validation
     */
    struct MigrationStep
    {
        std::string name;                           ///< Step name for logging
        std::function<bool()> execute;              ///< Execution function
        std::function<bool()> validate;             ///< Validation function
        std::function<void()> rollback;             ///< Rollback function
        bool completed = false;                     ///< Step completion status
    };

    /**
     * @typedef ConfigChangeCallback
     * @brief Callback for configuration change notifications
     */
    using ConfigChangeCallback = std::function<void(const std::string& key, const Config::ConfigValue& oldValue, const Config::ConfigValue& newValue)>;

    // Constructor and Destructor
    ConfigMigrationService(IPreferenceService* preferenceService, IDynamicConfigService* dynamicConfigService);
    ~ConfigMigrationService() = default;

    // Migration Control
    /**
     * @brief Initialize migration service and detect current phase
     * @return Current migration phase
     */
    MigrationPhase Initialize();

    /**
     * @brief Execute next migration step
     * @return true if step completed successfully
     */
    bool ExecuteNextStep();

    /**
     * @brief Execute all remaining migration steps
     * @return true if all steps completed successfully
     */
    bool ExecuteFullMigration();

    /**
     * @brief Rollback the last completed migration step
     * @return true if rollback successful
     */
    bool RollbackLastStep();

    /**
     * @brief Get current migration phase
     * @return Current phase
     */
    MigrationPhase GetCurrentPhase() const { return currentPhase_; }

    /**
     * @brief Get migration progress as percentage
     * @return Progress (0-100)
     */
    float GetMigrationProgress() const;

    // Configuration Access (Hybrid Mode)
    /**
     * @brief Get configuration value using hybrid lookup
     * @tparam T Expected value type
     * @param key Configuration key
     * @return Optional value if found
     */
    template<typename T>
    std::optional<T> GetConfigValue(const std::string& key) const;

    /**
     * @brief Set configuration value using hybrid system
     * @tparam T Value type
     * @param key Configuration key
     * @param value New value
     * @return true if set successfully
     */
    template<typename T>
    bool SetConfigValue(const std::string& key, const T& value);

    // Change Notification
    /**
     * @brief Register callback for configuration changes
     * @param callback Callback function
     */
    void RegisterChangeCallback(const ConfigChangeCallback& callback);

    /**
     * @brief Unregister change callback
     * @param callback Callback function to remove
     */
    void UnregisterChangeCallback(const ConfigChangeCallback& callback);

    // Validation and Integrity
    /**
     * @brief Validate configuration integrity across both systems
     * @return true if configurations are consistent
     */
    bool ValidateConfigurationIntegrity() const;

    /**
     * @brief Create backup of current configuration state
     * @return Backup identifier for rollback
     */
    std::string CreateConfigurationBackup();

    /**
     * @brief Restore configuration from backup
     * @param backupId Backup identifier
     * @return true if restore successful
     */
    bool RestoreConfigurationBackup(const std::string& backupId);

private:
    IPreferenceService* preferenceService_;
    IDynamicConfigService* dynamicConfigService_;
    MigrationPhase currentPhase_;
    std::vector<MigrationStep> migrationSteps_;
    size_t currentStepIndex_;
    std::vector<ConfigChangeCallback> changeCallbacks_;

    // Migration Steps
    void InitializeMigrationSteps();
    bool MigrateBasicSettings();
    bool MigrateSensorSettings();
    bool MigrateAdvancedSettings();
    bool EnableDynamicUI();
    bool ValidateMigration();
    bool CleanupLegacyData();

    // Helper Methods
    void NotifyConfigurationChange(const std::string& key, const Config::ConfigValue& oldValue, const Config::ConfigValue& newValue);
    bool SyncLegacyToDynamic();
    bool SyncDynamicToLegacy();
    std::string GenerateBackupId() const;

    // Legacy-Dynamic Mapping
    struct ConfigMapping
    {
        std::string legacyKey;
        std::string dynamicKey;
        Config::ConfigValueType type;
    };

    std::vector<ConfigMapping> GetConfigMappings() const;
};