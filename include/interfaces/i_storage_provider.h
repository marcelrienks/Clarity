#pragma once

#include "definitions/configs.h"
#include <vector>
#include <string>
#include <optional>
#include <functional>

// Type alias for configuration change callbacks
using ConfigChangeCallback = std::function<void(const std::string&, const std::optional<Config::ConfigValue>&, const Config::ConfigValue&)>;

/**
 * @interface IStorageProvider
 * @brief Hardware abstraction interface for configuration storage operations
 *
 * @details This interface provides hardware-agnostic storage operations for
 * configuration persistence. It abstracts the underlying storage mechanism
 * (NVS, EEPROM, filesystem, etc.) from the configuration management system.
 *
 * @design_pattern Provider Pattern - Hardware abstraction layer
 * @storage_abstraction Separates storage hardware from configuration logic
 * @dependency_injection Injectable interface for testing and hardware variants
 *
 * @implementations:
 * - StorageProvider: ESP32 NVS (Non-Volatile Storage) implementation
 * - MockStorageProvider: In-memory implementation for testing
 * - FileStorageProvider: Filesystem-based implementation (future)
 *
 * @context This interface allows ConfigurationManager to be hardware-agnostic
 * while providing persistent configuration storage across system reboots.
 */
class IStorageProvider {
public:
    virtual ~IStorageProvider() = default;

    // ========== Storage Operations ==========

    /**
     * @brief Register a configuration section for storage
     * @param section ConfigSection containing items, metadata, and validation rules
     * @return true if registration successful, false if section already exists
     */
    virtual bool RegisterConfigSection(const Config::ConfigSection& section) = 0;

    /**
     * @brief Get list of all registered configuration section names
     * @return Vector of section names for UI generation and iteration
     */
    virtual std::vector<std::string> GetRegisteredSectionNames() const = 0;

    /**
     * @brief Retrieve a specific configuration section by name
     * @param sectionName Name of the section to retrieve
     * @return Optional containing section if found, nullopt if not registered
     */
    virtual std::optional<Config::ConfigSection> GetConfigSection(const std::string& sectionName) const = 0;

    /**
     * @brief Save a specific configuration section to storage
     * @param sectionName Name of the section to persist
     * @return true if save operation successful
     */
    virtual bool SaveConfigSection(const std::string& sectionName) = 0;

    /**
     * @brief Load a specific configuration section from storage
     * @param sectionName Name of the section to load
     * @return true if load operation successful
     */
    virtual bool LoadConfigSection(const std::string& sectionName) = 0;

    /**
     * @brief Save all registered configuration sections to storage
     * @return true if all sections saved successfully
     */
    virtual bool SaveAllConfigSections() = 0;

    /**
     * @brief Load all registered configuration sections from storage
     * @return true if all sections loaded successfully
     */
    virtual bool LoadAllConfigSections() = 0;

    /**
     * @brief Validate a configuration value against its metadata constraints
     * @param fullKey Full dot-separated key (e.g., "oil_temp_sensor.unit")
     * @param value Value to validate
     * @return true if value passes validation (type, range, enum options)
     */
    virtual bool ValidateConfigValue(const std::string& fullKey, const Config::ConfigValue& value) const = 0;

    /**
     * @brief Register callback for live configuration change notifications
     * @param fullKey Configuration key to watch (empty string for all changes)
     * @param callback Function to call when configuration changes
     * @return Callback ID for later unregistration
     */
    virtual uint32_t RegisterChangeCallback(const std::string& fullKey, ConfigChangeCallback callback) = 0;

    /**
     * @brief Check if a configuration schema is registered
     * @param sectionName Name of the section to check
     * @return true if schema is registered, false otherwise
     */
    virtual bool IsSchemaRegistered(const std::string& sectionName) const = 0;

    // ========== Configuration Value Access ==========

    /**
     * @brief Query configuration value by key
     * @param fullKey Full dot-separated configuration key
     * @return Optional containing ConfigValue if found
     */
    virtual std::optional<Config::ConfigValue> QueryConfigValue(const std::string& fullKey) const = 0;

    /**
     * @brief Update configuration value by key
     * @param fullKey Full dot-separated configuration key
     * @param value New value to set
     * @return true if update successful (validation passed and storage write succeeded)
     */
    virtual bool UpdateConfigValue(const std::string& fullKey, const Config::ConfigValue& value) = 0;

    // ========== Configuration Value Helper Methods ==========

    /**
     * @brief Get type name of a configuration value
     * @param value The value to get type name for
     * @return Type name as string
     */
    virtual std::string GetTypeName(const Config::ConfigValue& value) const = 0;

    /**
     * @brief Check if two configuration values have matching types
     * @param a First value
     * @param b Second value
     * @return true if types match
     */
    virtual bool TypesMatch(const Config::ConfigValue& a, const Config::ConfigValue& b) const = 0;

    /**
     * @brief Convert configuration value to string representation
     * @param value The value to convert
     * @return String representation of the value
     */
    virtual std::string ToString(const Config::ConfigValue& value) const = 0;

    /**
     * @brief Convert string to configuration value using template value for type
     * @param str String to convert
     * @param templateValue Template value providing the target type
     * @return ConfigValue with converted value
     */
    virtual Config::ConfigValue FromString(const std::string& str, const Config::ConfigValue& templateValue) const = 0;

    /**
     * @brief Check if configuration value is numeric (int or float)
     * @param value The value to check
     * @return true if value is numeric
     */
    virtual bool IsNumeric(const Config::ConfigValue& value) const = 0;
};