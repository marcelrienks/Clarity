#pragma once

#include "config/config_types.h"
#include <optional>
#include <vector>
#include <string>
#include <functional>

/**
 * @interface IDynamicConfigService
 * @brief Interface for dynamic configuration registration and management
 *
 * @details This interface extends the basic preference service with dynamic
 * configuration capabilities, allowing components to self-register their
 * configuration requirements and enabling automatic UI generation.
 *
 * @design_principles:
 * - Component self-registration for configuration needs
 * - Type-safe configuration access with templates
 * - Metadata-driven UI generation
 * - Sectioned storage organization
 */
class IDynamicConfigService {
public:
    virtual ~IDynamicConfigService() = default;

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

    /**
     * @brief Callback function type for section changes
     * @param sectionName The name of the section that changed
     * @param changeType Type of change (added, removed, modified)
     */
    using SectionChangeCallback = std::function<void(const std::string& sectionName,
                                                    const std::string& changeType)>;

    // ========== Registration Methods ==========

    /**
     * @brief Register a configuration section for a component
     * @param section The configuration section to register
     * @return true if registration successful, false if section already exists
     */
    virtual bool RegisterConfigSection(const Config::ConfigSection& section) = 0;

    /**
     * @brief Unregister a configuration section
     * @param sectionName The name of the section to unregister
     * @return true if unregistration successful
     */
    virtual bool UnregisterConfigSection(const std::string& sectionName) = 0;

    // ========== Query Methods ==========

    /**
     * @brief Query a configuration value with type safety
     * @tparam T The expected type of the configuration value
     * @param fullKey Full configuration key (e.g., "oil_temp_sensor.unit")
     * @return Optional containing the value if found and type matches
     */
    template<typename T>
    std::optional<T> QueryConfig(const std::string& fullKey) const {
        auto valueOpt = QueryConfigImpl(fullKey);
        if (!valueOpt) return std::nullopt;
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

    /**
     * @brief Get all configuration sections
     * @return Vector of all registered configuration sections
     */
    virtual std::vector<Config::ConfigSection> GetAllConfigSections() const = 0;

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

    /**
     * @brief Reset a configuration item to its default value
     * @param fullKey Full configuration key
     * @return true if reset successful
     */
    virtual bool ResetToDefault(const std::string& fullKey) = 0;

    /**
     * @brief Reset an entire section to default values
     * @param sectionName The name of the section to reset
     * @return true if reset successful
     */
    virtual bool ResetSectionToDefaults(const std::string& sectionName) = 0;

    // ========== Live Update Methods ==========

    /**
     * @brief Register a callback for configuration changes
     * @param fullKey The full configuration key to watch (empty string for all keys)
     * @param callback The callback function to call when the configuration changes
     * @return Callback ID for later unregistration
     */
    virtual uint32_t RegisterChangeCallback(const std::string& fullKey, ConfigChangeCallback callback) = 0;

    /**
     * @brief Register a callback for section changes
     * @param sectionName The section name to watch (empty string for all sections)
     * @param callback The callback function to call when sections change
     * @return Callback ID for later unregistration
     */
    virtual uint32_t RegisterSectionCallback(const std::string& sectionName, SectionChangeCallback callback) = 0;

    /**
     * @brief Unregister a configuration change callback
     * @param callbackId The callback ID returned from RegisterChangeCallback
     * @return true if callback was found and removed
     */
    virtual bool UnregisterChangeCallback(uint32_t callbackId) = 0;

    /**
     * @brief Unregister a section change callback
     * @param callbackId The callback ID returned from RegisterSectionCallback
     * @return true if callback was found and removed
     */
    virtual bool UnregisterSectionCallback(uint32_t callbackId) = 0;

    /**
     * @brief Force notification of all registered callbacks for a key
     * @param fullKey The configuration key to notify about
     * @return true if notifications were sent
     */
    virtual bool NotifyConfigChange(const std::string& fullKey) = 0;

    /**
     * @brief Enable or disable live updates
     * @param enabled Whether live updates should be active
     */
    virtual void SetLiveUpdatesEnabled(bool enabled) = 0;

    /**
     * @brief Check if live updates are currently enabled
     * @return true if live updates are enabled
     */
    virtual bool AreLiveUpdatesEnabled() const = 0;

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