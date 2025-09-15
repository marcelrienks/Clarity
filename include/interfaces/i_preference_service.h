#pragma once

#include "config/config_types.h"
#include <optional>
#include <vector>
#include <string>
#include <functional>

/**
 * @interface IPreferenceService
 * @brief Modern dynamic configuration interface for component self-registration
 *
 * @details This interface provides a complete dynamic configuration system
 * that enables components to self-register their configuration requirements,
 * automatic UI generation, and type-safe configuration access.
 *
 * Based on docs/plans/dynamic-config-implementation.md design:
 * - Component self-registration of configuration sections
 * - Type-safe configuration access with templates
 * - Metadata-driven UI generation
 * - Sectioned NVS storage organization
 * - Live configuration updates with callbacks
 *
 * @design_principles:
 * - Component self-registration for configuration needs
 * - Type-safe configuration access with templates
 * - Metadata-driven UI generation
 * - Sectioned storage organization
 * - No backwards compatibility - clean modern design
 */
class IPreferenceService
{
public:
    virtual ~IPreferenceService() = default;

    // ========== Live Update Callback Types ==========

    /**
     * @brief Callback function type for configuration changes
     * @param fullKey The full configuration key that changed
     * @param oldValue The previous value (may be empty if key was new)
     * @param newValue The new value
     */
    using ConfigChangeCallback = std::function<void(const std::string& fullKey,
                                                   const std::optional<Config::ConfigValue>& oldValue,
                                                   const Config::ConfigValue& newValue)>;

    // ========== Core Functionality Methods ==========

    /**
     * @brief Save current configuration to persistent storage
     */
    virtual void SaveConfig() = 0;

    /**
     * @brief Load configuration from persistent storage
     */
    virtual void LoadConfig() = 0;

    /**
     * @brief Create default configuration if none exists
     */
    virtual void CreateDefaultConfig() = 0;

    // ========== Legacy String-based Access (for simple cases) ==========

    /**
     * @brief Get a preference value by key
     * @param key The preference key
     * @return String representation of the value
     */
    virtual std::string GetPreference(const std::string& key) const = 0;

    /**
     * @brief Set a preference value by key
     * @param key The preference key
     * @param value String representation of the value
     */
    virtual void SetPreference(const std::string& key, const std::string& value) = 0;

    /**
     * @brief Check if a preference exists
     * @param key The preference key
     * @return true if preference exists
     */
    virtual bool HasPreference(const std::string& key) const = 0;

    // ========== Dynamic Configuration Registration ==========

    /**
     * @brief Register a configuration section for a component
     * @param section The configuration section to register
     * @return true if registration successful, false if section already exists
     */
    virtual bool RegisterConfigSection(const Config::ConfigSection& section) = 0;

    // ========== Type-Safe Configuration Access ==========

    /**
     * @brief Query a configuration value with type safety
     * @tparam T The expected type of the configuration value
     * @param fullKey Full configuration key (e.g., "oil_temp_sensor.unit")
     * @return Optional containing the value if found and type matches
     */
    template<typename T>
    std::optional<T> QueryConfig(const std::string& fullKey) const {
        // Delegate to implementation method for actual config lookup
        auto valueOpt = QueryConfigImpl(fullKey);
        if (!valueOpt) return std::nullopt;

        // Use ConfigValueHelper for type-safe value extraction
        return Config::ConfigValueHelper::GetValue<T>(*valueOpt);
    }

    /**
     * @brief Update a configuration value with type safety
     * @tparam T The type of the configuration value
     * @param fullKey Full configuration key (e.g., "oil_temp_sensor.unit")
     * @param value The new value to set
     * @return true if update successful
     */
    template<typename T>
    bool UpdateConfig(const std::string& fullKey, const T& value) {
        // Convert typed value to ConfigValue variant and delegate to implementation
        return UpdateConfigImpl(fullKey, Config::ConfigValue(value));
    }

    // ========== Section Access Methods ==========

    /**
     * @brief Get all registered section names
     * @return Vector of registered section names
     */
    virtual std::vector<std::string> GetRegisteredSectionNames() const = 0;

    /**
     * @brief Get a configuration section by name
     * @param sectionName The name of the section to retrieve
     * @return Optional containing the section if found
     */
    virtual std::optional<Config::ConfigSection> GetConfigSection(const std::string& sectionName) const = 0;

    // ========== Persistence Methods ==========

    /**
     * @brief Save a specific configuration section to storage
     * @param sectionName The name of the section to save
     * @return true if save successful
     */
    virtual bool SaveConfigSection(const std::string& sectionName) = 0;

    /**
     * @brief Load a specific configuration section from storage
     * @param sectionName The name of the section to load
     * @return true if load successful
     */
    virtual bool LoadConfigSection(const std::string& sectionName) = 0;

    /**
     * @brief Save all configuration sections to storage
     * @return true if save successful
     */
    virtual bool SaveAllConfigSections() = 0;

    /**
     * @brief Load all configuration sections from storage
     * @return true if load successful
     */
    virtual bool LoadAllConfigSections() = 0;

    // ========== Validation Methods ==========

    /**
     * @brief Validate a configuration value against its metadata
     * @param fullKey Full configuration key
     * @param value The value to validate
     * @return true if value is valid
     */
    virtual bool ValidateConfigValue(const std::string& fullKey, const Config::ConfigValue& value) const = 0;

    // ========== Live Update Methods ==========

    /**
     * @brief Register a callback for configuration changes
     * @param fullKey The full configuration key to watch (empty string for all keys)
     * @param callback The callback function to call when the configuration changes
     * @return Callback ID for later unregistration
     */
    virtual uint32_t RegisterChangeCallback(const std::string& fullKey, ConfigChangeCallback callback) = 0;

protected:
    // ========== Implementation Methods ==========
    // These are protected to allow template methods to work

    /**
     * @brief Internal implementation for querying configuration values
     * @param fullKey Full configuration key
     * @return Optional containing the ConfigValue if found
     */
    virtual std::optional<Config::ConfigValue> QueryConfigImpl(const std::string& fullKey) const = 0;

    /**
     * @brief Internal implementation for updating configuration values
     * @param fullKey Full configuration key
     * @param value The new value to set
     * @return true if update successful
     */
    virtual bool UpdateConfigImpl(const std::string& fullKey, const Config::ConfigValue& value) = 0;
};