#pragma once

#include <ArduinoJson.h>
#include <functional>
#include <memory>
#include <string>
#include <vector>

/**
 * @enum ConfigMenuItemType
 * @brief Types of configuration menu items
 */
enum class ConfigMenuItemType
{
    Submenu, ///< Opens a submenu
    Choice,  ///< Multiple choice selection
    Toggle,  ///< Boolean on/off switch
    Number,  ///< Numeric value with min/max
    Action,  ///< Executes a specific action
    Back     ///< Returns to parent menu
};

/**
 * @struct VisibilityCondition
 * @brief Defines conditional visibility for menu items
 */
struct VisibilityCondition
{
    std::string preference;    ///< Preference key to check
    std::string expectedValue; ///< Expected value for visibility

    bool evaluate(class IPreferenceService *prefService) const;
};

/**
 * @class ConfigMenuItem
 * @brief Represents a single item in the configuration menu
 *
 * @details This class represents menu items loaded from JSON configuration
 * files. It supports various types of settings including submenus, choices,
 * toggles, numbers, and actions. Items can be conditionally visible based
 * on other preference values.
 */
class ConfigMenuItem
{
  public:
    // Constructors
    ConfigMenuItem() = default;
    ConfigMenuItem(const JsonObject &json);

    // Properties
    std::string id;            ///< Unique identifier
    std::string label;         ///< Display label
    ConfigMenuItemType type;   ///< Item type
    std::string preferenceKey; ///< Associated preference key

    // Type-specific data
    std::vector<std::shared_ptr<ConfigMenuItem>> children; ///< Submenu items
    std::vector<std::string> choices;                      ///< Available choices
    std::vector<std::string> choiceLabels;                 ///< Display labels for choices
    float minValue = 0;                                    ///< Minimum value (for Number type)
    float maxValue = 100;                                  ///< Maximum value (for Number type)
    float step = 1;                                        ///< Step value (for Number type)
    std::string defaultValue;                              ///< Default value
    std::string actionId;                                  ///< Action identifier

    // Visibility
    std::unique_ptr<VisibilityCondition> visibilityCondition;

    // Methods
    bool isVisible(IPreferenceService *prefService) const;
    std::string getCurrentValue(IPreferenceService *prefService) const;
    void setValue(IPreferenceService *prefService, const std::string &value) const;
    std::string getDisplayValue(IPreferenceService *prefService) const;

    // Factory method
    static std::vector<std::shared_ptr<ConfigMenuItem>> loadFromJson(const JsonArray &jsonArray);

  private:
    ConfigMenuItemType parseType(const std::string &typeStr) const;
    void parseVisibility(const JsonObject &json);
};