#pragma once

#include "interfaces/i_preference_service.h"
#include "config/config_types.h"
#include "utilities/constants.h"

#include <ArduinoJson.h>
#include <Preferences.h>
#include <nvs_flash.h>
#include <map>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

/**
 * @class PreferenceManager
 * @brief Modern dynamic configuration manager
 *
 * @details This manager provides:
 * - Component self-registration of configuration requirements
 * - Sectioned NVS storage for better organization
 * - Type-safe configuration access with templates
 * - Live configuration updates with callbacks
 * - Metadata-driven validation and UI generation
 *
 * Based on docs/plans/dynamic-config-implementation.md design.
 * No backwards compatibility - clean modern design.
 *
 * @storage_format Sectioned NVS with separate namespace per component
 * @thread_safety Thread-safe configuration access with mutex protection
 */
class PreferenceManager : public IPreferenceService {
public:
    // ========== Constructors and Destructor ==========
    PreferenceManager();
    PreferenceManager(const PreferenceManager&) = delete;
    PreferenceManager& operator=(const PreferenceManager&) = delete;
    ~PreferenceManager() = default;

    // ========== IPreferenceService Implementation ==========

    // ========== Dynamic Configuration Methods ==========

    /**
     * @brief Register a configuration section for component self-registration
     * @param section ConfigSection containing items, metadata, and validation rules
     * @return true if registration successful, false if section already exists
     * @details Implements component self-registration pattern from dynamic-config-implementation.md
     * Automatically loads any existing persisted values after registration
     */
    bool RegisterConfigSection(const Config::ConfigSection& section) override;

    /**
     * @brief Get list of all registered configuration section names
     * @return Vector of section names for UI generation and iteration
     * @details Used by ConfigPanel for automatic menu generation
     */
    std::vector<std::string> GetRegisteredSectionNames() const override;

    /**
     * @brief Retrieve a specific configuration section by name
     * @param sectionName Name of the section to retrieve
     * @return Optional containing section if found, nullopt if not registered
     * @details Returns section with current values and metadata for UI display
     */
    std::optional<Config::ConfigSection> GetConfigSection(const std::string& sectionName) const override;

    /**
     * @brief Save a specific configuration section to NVS storage
     * @param sectionName Name of the section to persist
     * @return true if save operation successful
     * @details Uses sectioned NVS storage with separate namespace per component
     */
    bool SaveConfigSection(const std::string& sectionName) override;

    /**
     * @brief Load a specific configuration section from NVS storage
     * @param sectionName Name of the section to load
     * @return true if load operation successful
     * @details Loads persisted values into in-memory section definition
     */
    bool LoadConfigSection(const std::string& sectionName) override;

    /**
     * @brief Save all registered configuration sections to storage
     * @return true if all sections saved successfully
     * @details Batch operation for complete configuration persistence
     */
    bool SaveAllConfigSections() override;

    /**
     * @brief Load all registered configuration sections from storage
     * @return true if all sections loaded successfully
     * @details Batch operation for complete configuration restoration
     */
    bool LoadAllConfigSections() override;

    /**
     * @brief Validate a configuration value against its metadata constraints
     * @param fullKey Full dot-separated key (e.g., "oil_temp_sensor.unit")
     * @param value Value to validate
     * @return true if value passes validation (type, range, enum options)
     * @details Performs type checking and constraint validation from metadata
     */
    bool ValidateConfigValue(const std::string& fullKey, const Config::ConfigValue& value) const override;

    /**
     * @brief Register callback for live configuration change notifications
     * @param fullKey Configuration key to watch (empty string for all changes)
     * @param callback Function to call when configuration changes
     * @return Callback ID for later unregistration
     * @details Enables real-time response to configuration updates
     */
    uint32_t RegisterChangeCallback(const std::string& fullKey, ConfigChangeCallback callback) override;

protected:
    /**
     * @brief Internal implementation for querying configuration values
     * @param fullKey Full dot-separated configuration key
     * @return Optional containing ConfigValue if found
     * @details Protected to allow template methods in interface to access
     */
    std::optional<Config::ConfigValue> QueryConfigImpl(const std::string& fullKey) const override;

    /**
     * @brief Internal implementation for updating configuration values
     * @param fullKey Full dot-separated configuration key
     * @param value New value to set
     * @return true if update successful (validation passed and NVS write succeeded)
     * @details Protected to allow template methods in interface to access
     */
    bool UpdateConfigImpl(const std::string& fullKey, const Config::ConfigValue& value) override;

private:
    // ========== Private Data Members ==========
    static inline const char* CONFIG_KEY_ = StorageConstants::NVS::CONFIG_KEY;                 ///< Legacy config key
    static inline const char* META_NAMESPACE_ = StorageConstants::NVS::META_NAMESPACE;        ///< Metadata namespace
    static inline const char* SECTION_PREFIX_ = StorageConstants::NVS::SECTION_PREFIX;              ///< Prefix for section namespaces
    static inline const char* MIGRATION_FLAG_ = StorageConstants::NVS::MIGRATION_FLAG;      ///< Migration completion flag
    static inline const int MAX_NAMESPACE_LEN_ = StorageConstants::NVS::MAX_NAMESPACE_LEN;                 ///< NVS namespace length limit

    mutable SemaphoreHandle_t configMutex_;                         ///< Thread safety semaphore
    std::map<std::string, Config::ConfigSection> registeredSections_; ///< Registered configuration sections
    Preferences preferences_;                                        ///< NVS preferences instance

    // Live update system
    uint32_t nextCallbackId_ = 1;                                  ///< Counter for generating callback IDs
    std::map<uint32_t, std::pair<std::string, ConfigChangeCallback>> changeCallbacks_; ///< Configuration change callbacks

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
     * @brief Convert ConfigValue to NVS-compatible type and store
     * @param prefs Preferences instance for the section
     * @param key The configuration key
     * @param value The value to store
     * @param type The value type
     * @return true if storage successful
     */
    bool StoreValueToNVS(Preferences& prefs, const std::string& key,
                        const Config::ConfigValue& value);

    /**
     * @brief Load value from NVS and convert to ConfigValue
     * @param prefs Preferences instance for the section
     * @param key The configuration key
     * @param type The expected value type
     * @return ConfigValue containing the loaded value
     */
    Config::ConfigValue LoadValueFromNVS(Preferences& prefs, const std::string& key,
                                         const Config::ConfigValue& templateValue);
};