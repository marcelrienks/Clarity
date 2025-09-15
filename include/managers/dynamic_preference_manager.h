#pragma once

#include "interfaces/i_preference_service.h"
#include "interfaces/i_dynamic_config_service.h"
#include "config/config_types.h"
#include "utilities/types.h"

#include <ArduinoJson.h>
#include <Preferences.h>
#include <nvs_flash.h>
#include <map>
#include <mutex>

/**
 * @class DynamicPreferenceManager
 * @brief Enhanced configuration manager with dynamic registration support
 *
 * @details This manager extends the original PreferenceManager with support for:
 * - Component self-registration of configuration requirements
 * - Sectioned NVS storage for better organization
 * - Type-safe configuration access with templates
 * - Automatic migration from legacy configuration format
 * - Metadata-driven validation
 *
 * @backward_compatibility Maintains support for legacy Configs struct during migration
 * @storage_format Sectioned NVS with separate namespace per component
 * @thread_safety Thread-safe configuration access with mutex protection
 */
class DynamicPreferenceManager : public IPreferenceService, public IDynamicConfigService {
public:
    // ========== Constructors and Destructor ==========
    DynamicPreferenceManager() = default;
    DynamicPreferenceManager(const DynamicPreferenceManager&) = delete;
    DynamicPreferenceManager& operator=(const DynamicPreferenceManager&) = delete;
    ~DynamicPreferenceManager() = default;

    // ========== IPreferenceService Implementation ==========
    void Init() override;
    void SaveConfig() override;
    void LoadConfig() override;
    void CreateDefaultConfig() override;
    Configs& GetConfig() override;
    const Configs& GetConfig() const override;
    void SetConfig(const Configs& config) override;
    std::string GetPreference(const std::string& key) const override;
    void SetPreference(const std::string& key, const std::string& value) override;
    bool HasPreference(const std::string& key) const override;

    // ========== IDynamicConfigService Implementation ==========
    bool RegisterConfigSection(const Config::ConfigSection& section) override;
    bool UnregisterConfigSection(const std::string& sectionName) override;
    std::vector<std::string> GetRegisteredSectionNames() const override;
    std::optional<Config::ConfigSection> GetConfigSection(const std::string& sectionName) const override;
    std::vector<Config::ConfigSection> GetAllConfigSections() const override;
    bool SaveConfigSection(const std::string& sectionName) override;
    bool LoadConfigSection(const std::string& sectionName) override;
    bool SaveAllConfigSections() override;
    bool LoadAllConfigSections() override;
    bool ValidateConfigValue(const std::string& fullKey, const Config::ConfigValue& value) const override;
    bool ResetToDefault(const std::string& fullKey) override;
    bool ResetSectionToDefaults(const std::string& sectionName) override;

protected:
    std::optional<Config::ConfigValue> QueryConfigImpl(const std::string& fullKey) const override;
    bool UpdateConfigImpl(const std::string& fullKey, const Config::ConfigValue& value) override;

private:
    // ========== Private Data Members ==========
    static inline const char* CONFIG_KEY_ = "config";                 ///< Legacy config key
    static inline const char* META_NAMESPACE_ = "config_meta";        ///< Metadata namespace
    static inline const char* SECTION_PREFIX_ = "cfg_";              ///< Prefix for section namespaces
    static inline const char* MIGRATION_FLAG_ = "migration_v1";      ///< Migration completion flag
    static inline const int MAX_NAMESPACE_LEN_ = 15;                 ///< NVS namespace length limit

    mutable std::mutex configMutex_;                                 ///< Thread safety mutex
    std::map<std::string, Config::ConfigSection> registeredSections_; ///< Registered configuration sections
    static inline Configs legacyConfig_;                             ///< Legacy configuration for backward compatibility
    Preferences preferences_;                                        ///< NVS preferences instance
    bool migrationCompleted_ = false;                               ///< Migration status flag

    // ========== Private Helper Methods ==========

    /**
     * @brief Parse a full configuration key into section and item
     * @param fullKey Full key (e.g., "oil_temp_sensor.unit")
     * @return Pair of section name and item key
     */
    std::pair<std::string, std::string> ParseConfigKey(const std::string& fullKey) const;

    /**
     * @brief Get NVS namespace for a section
     * @param sectionName The section name
     * @return NVS namespace string (truncated if necessary)
     */
    std::string GetSectionNamespace(const std::string& sectionName) const;

    /**
     * @brief Migrate legacy configuration to new format
     * @return true if migration successful or not needed
     */
    bool MigrateLegacyConfig();

    /**
     * @brief Synchronize legacy config struct with dynamic sections
     * @details Updates the legacy Configs struct based on current dynamic values
     */
    void SyncLegacyConfig();

    /**
     * @brief Synchronize dynamic sections with legacy config struct
     * @details Updates dynamic configuration based on legacy struct values
     */
    void SyncFromLegacyConfig();

    /**
     * @brief Create default configuration sections
     * @details Registers default sections for system components
     */
    void CreateDefaultSections();

    /**
     * @brief Validate integer value against range constraints
     * @param value The value to validate
     * @param constraints Range string (e.g., "0-100")
     * @return true if value is within range
     */
    bool ValidateIntRange(int value, const std::string& constraints) const;

    /**
     * @brief Validate float value against range constraints
     * @param value The value to validate
     * @param constraints Range string (e.g., "0.0-1.0")
     * @return true if value is within range
     */
    bool ValidateFloatRange(float value, const std::string& constraints) const;

    /**
     * @brief Validate enum value against allowed options
     * @param value The value to validate
     * @param constraints Comma-separated options (e.g., "PSI,Bar,kPa")
     * @return true if value is in the allowed list
     */
    bool ValidateEnumValue(const std::string& value, const std::string& constraints) const;

    /**
     * @brief Parse comma-separated string into vector
     * @param str The string to parse
     * @return Vector of parsed options
     */
    std::vector<std::string> ParseOptions(const std::string& str) const;

    /**
     * @brief Save section list to metadata namespace
     * @return true if save successful
     */
    bool SaveSectionList();

    /**
     * @brief Load section list from metadata namespace
     * @return true if load successful
     */
    bool LoadSectionList();

    /**
     * @brief Convert ConfigValue to NVS-compatible type and store
     * @param prefs Preferences instance for the section
     * @param key The configuration key
     * @param value The value to store
     * @param type The value type
     * @return true if storage successful
     */
    bool StoreValueToNVS(Preferences& prefs, const std::string& key,
                        const Config::ConfigValue& value, Config::ConfigValueType type);

    /**
     * @brief Load value from NVS and convert to ConfigValue
     * @param prefs Preferences instance for the section
     * @param key The configuration key
     * @param type The expected value type
     * @return ConfigValue containing the loaded value
     */
    Config::ConfigValue LoadValueFromNVS(Preferences& prefs, const std::string& key,
                                         Config::ConfigValueType type);
};