#pragma once

#include <string>
#include <variant>
#include <vector>
#include <optional>
#include <map>
#include "definitions/constants.h"

/**
 * @file config_types.h
 * @brief Core data structures for the dynamic configuration system
 *
 * @details Provides the fundamental types needed for component self-registration
 * of configuration requirements, metadata-driven UI generation, and type-safe
 * configuration access.
 */

namespace Config {

/**
 * @typedef ConfigValue
 * @brief Variant type for storing configuration values
 *
 * @details Supports all configuration value types in a type-safe manner.
 * Uses std::monostate for uninitialized values.
 */
using ConfigValue = std::variant<std::monostate, int, float, std::string, bool>;

/**
 * @enum ConfigItemType
 * @brief UI hint for how to display the configuration item
 */
enum class ConfigItemType {
    Value,      ///< Single value (int, float, string, bool)
    Selection,  ///< Selection from predefined options (enum-like)
    Range       ///< Value with min/max constraints
};

/**
 * @struct ConfigMetadata
 * @brief Metadata for configuration items
 *
 * @details Provides validation constraints and UI hints for configuration items.
 * The metadata string format depends on the value type:
 * - Integer/Float ranges: "min-max" (e.g., "0-100")
 * - Enum/Selection options: Comma-separated list (e.g., "PSI,Bar,kPa")
 * - Integer/Float options: Comma-separated list (e.g., "250,500,1000,2000")
 */
struct ConfigMetadata {
    std::string constraints;     ///< Validation constraints or enum options
    std::string unit;            ///< Unit of measurement (optional)
    std::string description;     ///< Detailed description for UI tooltips
    ConfigItemType itemType = ConfigItemType::Value; ///< UI display hint
    bool readOnly = false;       ///< Whether this config is read-only
    bool advanced = false;       ///< Whether to show in advanced settings only

    ConfigMetadata() = default;
    ConfigMetadata(const std::string& constraints, ConfigItemType type = ConfigItemType::Selection)
        : constraints(constraints), itemType(type) {}
    ConfigMetadata(const std::string& constraints, const std::string& unit, ConfigItemType type = ConfigItemType::Value)
        : constraints(constraints), unit(unit), itemType(type) {}
};

/**
 * @struct ConfigItem
 * @brief Individual configuration item with value and metadata
 *
 * @details Represents a single configuration setting with its current value,
 * display information, and validation metadata.
 */
struct ConfigItem {
    std::string key;              ///< Unique key within the section
    std::string displayName;      ///< Human-readable name for UI
    ConfigValue value;            ///< Current value
    ConfigValue defaultValue;     ///< Default value for reset functionality
    ConfigMetadata metadata;      ///< Validation and UI metadata

    ConfigItem() = default;

    ConfigItem(const std::string& key,
               const std::string& displayName,
               const ConfigValue& defaultValue)
        : key(key), displayName(displayName),
          value(defaultValue), defaultValue(defaultValue) {}

    ConfigItem(const std::string& key,
               const std::string& displayName,
               const ConfigValue& defaultValue,
               const ConfigMetadata& metadata)
        : key(key), displayName(displayName),
          value(defaultValue), defaultValue(defaultValue), metadata(metadata) {}
};

/**
 * @struct ConfigSection
 * @brief Grouped configuration items for a component
 *
 * @details Represents all configuration items for a single component,
 * organized as a logical section in the configuration system.
 */
struct ConfigSection {
    std::string componentName;    ///< Name of the owning component
    std::string sectionName;      ///< Unique section identifier (e.g., "oil_temp_sensor")
    std::string displayName;      ///< Human-readable section name for UI
    std::vector<ConfigItem> items; ///< Configuration items in this section

    ConfigSection() = default;

    ConfigSection(const std::string& componentName,
                  const std::string& sectionName,
                  const std::string& displayName)
        : componentName(componentName), sectionName(sectionName), displayName(displayName) {}

    /**
     * @brief Add a configuration item to this section
     */
    void AddItem(const ConfigItem& item) {
        items.push_back(item);
    }

    /**
     * @brief Find a configuration item by key
     * @return Optional containing the item if found
     */
    ConfigItem* FindItem(const std::string& key) {
        for (auto& item : items) {
            if (item.key == key) {
                return &item;
            }
        }
        return nullptr;
    }

    /**
     * @brief Find a configuration item by key (const version)
     * @return Optional containing the item if found
     */
    const ConfigItem* FindItem(const std::string& key) const {
        for (const auto& item : items) {
            if (item.key == key) {
                return &item;
            }
        }
        return nullptr;
    }
};


} // namespace Config