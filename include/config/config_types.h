#pragma once

#include <string>
#include <variant>
#include <vector>
#include <optional>
#include <map>

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
 * @enum ConfigValueType
 * @brief Supported configuration value types
 */
enum class ConfigValueType {
    Integer,    ///< Integer values with optional range constraints
    Float,      ///< Floating-point values with optional range constraints
    String,     ///< Free-form text values
    Boolean,    ///< True/false toggle values
    Enum        ///< Enumerated values from a predefined list
};

/**
 * @typedef ConfigValue
 * @brief Variant type for storing configuration values
 *
 * @details Supports all configuration value types in a type-safe manner.
 * Uses std::monostate for uninitialized values.
 */
using ConfigValue = std::variant<std::monostate, int, float, std::string, bool>;

/**
 * @struct ConfigMetadata
 * @brief Metadata for configuration items
 *
 * @details Provides validation constraints and UI hints for configuration items.
 * The metadata string format depends on the value type:
 * - Integer/Float ranges: "min-max" (e.g., "0-100")
 * - Enum options: Comma-separated list (e.g., "PSI,Bar,kPa")
 * - Integer/Float options: Comma-separated list (e.g., "250,500,1000,2000")
 */
struct ConfigMetadata {
    std::string constraints;     ///< Validation constraints or enum options
    std::string unit;            ///< Unit of measurement (optional)
    std::string description;     ///< Detailed description for UI tooltips
    bool readOnly = false;       ///< Whether this config is read-only
    bool advanced = false;       ///< Whether to show in advanced settings only

    ConfigMetadata() = default;
    ConfigMetadata(const std::string& constraints) : constraints(constraints) {}
    ConfigMetadata(const std::string& constraints, const std::string& unit)
        : constraints(constraints), unit(unit) {}
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
    ConfigValueType type;         ///< Data type of the configuration value
    ConfigValue value;            ///< Current value
    ConfigValue defaultValue;     ///< Default value for reset functionality
    ConfigMetadata metadata;      ///< Validation and UI metadata

    ConfigItem() = default;

    ConfigItem(const std::string& key,
               const std::string& displayName,
               ConfigValueType type,
               const ConfigValue& defaultValue)
        : key(key), displayName(displayName), type(type),
          value(defaultValue), defaultValue(defaultValue) {}

    ConfigItem(const std::string& key,
               const std::string& displayName,
               ConfigValueType type,
               const ConfigValue& defaultValue,
               const ConfigMetadata& metadata)
        : key(key), displayName(displayName), type(type),
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
    int displayOrder = 0;         ///< Order for UI display (lower = earlier)

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

/**
 * @class ConfigValueHelper
 * @brief Helper class for type-safe configuration value operations
 *
 * @details Provides template methods for extracting values from ConfigValue
 * variants and converting between types safely.
 */
class ConfigValueHelper {
public:
    /**
     * @brief Extract a typed value from a ConfigValue variant
     * @tparam T The expected type
     * @param value The ConfigValue to extract from
     * @return Optional containing the value if type matches
     */
    template<typename T>
    static std::optional<T> GetValue(const ConfigValue& value) {
        if (std::holds_alternative<T>(value)) {
            return std::get<T>(value);
        }
        return std::nullopt;
    }

    /**
     * @brief Convert a ConfigValue to string representation
     * @param value The ConfigValue to convert
     * @return String representation of the value
     */
    static std::string ToString(const ConfigValue& value) {
        return std::visit([](const auto& v) -> std::string {
            using T = std::decay_t<decltype(v)>;
            if constexpr (std::is_same_v<T, std::monostate>) {
                return "";
            } else if constexpr (std::is_same_v<T, bool>) {
                return v ? "true" : "false";
            } else if constexpr (std::is_same_v<T, std::string>) {
                return v;
            } else {
                return std::to_string(v);
            }
        }, value);
    }

    /**
     * @brief Parse a string into a ConfigValue based on type
     * @param str The string to parse
     * @param type The expected type
     * @return ConfigValue containing the parsed value
     */
    static ConfigValue FromString(const std::string& str, ConfigValueType type) {
        switch (type) {
            case ConfigValueType::Integer:
                try {
                    return std::stoi(str);
                } catch (...) {
                    return std::monostate{};
                }

            case ConfigValueType::Float:
                try {
                    return std::stof(str);
                } catch (...) {
                    return std::monostate{};
                }

            case ConfigValueType::Boolean:
                return (str == "true" || str == "1");

            case ConfigValueType::String:
            case ConfigValueType::Enum:
                return str;

            default:
                return std::monostate{};
        }
    }
};

} // namespace Config