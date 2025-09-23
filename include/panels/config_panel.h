#pragma once

#include "components/config_component.h"
#include "interfaces/i_action_handler.h"
#include "interfaces/i_display_provider.h"
#include "interfaces/i_gpio_provider.h"
#include "interfaces/i_panel.h"
#include "interfaces/i_panel_manager.h"
#include "interfaces/i_configuration_manager.h"
#include "interfaces/i_style_manager.h"
#include "definitions/configs.h"
#include "definitions/types.h"

#include <memory>
#include <string>
#include <vector>

/**
 * @class ConfigPanel
 * @brief Configuration settings panel for system preferences
 *
 * @details This panel provides a single-button navigable interface for
 * configuring system settings. It displays a menu structure similar to
 * the error panel but with grey colors for a settings-appropriate theme.
 *
 * @presenter_role Manages configuration UI and setting persistence
 * @navigation Single button: short press cycles options, long press selects
 * @implementation Initial version with basic configuration options
 *
 * @ui_layout:
 * - Header: "Configuration" title
 * - Menu: List of options with highlight indicator
 * - Footer: Current selection hint
 *
 * @menu_structure:
 * - Option 1 (placeholder)
 * - Option 2 (placeholder)
 * - Exit (returns to previous panel)
 *
 * @visual_style:
 * - Grey color scheme matching system settings theme
 * - Highlighted option with border/background
 * - Clean, minimalist appearance
 *
 * @context This is an initial implementation with basic configuration options.
 * Additional settings and functionality can be added as needed.
 */
class ConfigPanel : public IPanel
{
  public:
    // ========== Constructors and Destructor ==========
    ConfigPanel(IGpioProvider *gpio, IDisplayProvider *display, IStyleManager *styleManager,
                IPanelManager *panelManager, IConfigurationManager *configurationManager);
    ~ConfigPanel();

    // ========== Public Interface Methods ==========
    static constexpr const char *NAME = PanelNames::CONFIG;
    void Init() override;
    void Load() override;
    void Update() override;


    // IActionService Interface Implementation (inherited through IPanel)
    void HandleShortPress() override;
    void HandleLongPress() override;

  private:
    // ========== Private Types ==========

    enum class MenuActionType {
        ENTER_SECTION,
        TOGGLE_BOOLEAN,
        SHOW_OPTIONS,
        SET_CONFIG_VALUE,
        BACK,
        NONE,
        PANEL_EXIT,
        UNKNOWN
    };

    // ========== Private Methods ==========

    // Dynamic configuration methods
    void BuildDynamicMenus();
    void BuildSectionMenu(const std::string& sectionName);
    std::string FormatItemLabel(const Config::ConfigItem& item) const;
    void ShowOptionsMenu(const std::string& fullKey, const Config::ConfigItem& item);
    void ShowBooleanToggle(const std::string& fullKey, const Config::ConfigItem& item);

    // Extracted ShowOptionsMenu helpers
    void ShowEnumOptionsMenu(const std::string& fullKey, const Config::ConfigItem& item, const std::vector<std::string>& options);
    void ShowNumericOptionsMenu(const std::string& fullKey, const Config::ConfigItem& item);
    void ShowStringOptionsMenu(const std::string& fullKey, const Config::ConfigItem& item);
    ConfigComponent::MenuItem CreateMenuItemWithSelection(const std::string& label, const std::string& fullKey, const std::string& value, bool isSelected) const;

    // Extracted ShowNumericOptionsMenu helpers
    struct NumericRange {
        float minValue;
        float maxValue;
        float currentValue;
    };
    NumericRange ParseNumericRange(const Config::ConfigItem& item) const;
    std::vector<float> GenerateNumericValues(const NumericRange& range, const Config::ConfigItem& item) const;
    std::string FormatNumericValue(float value, const Config::ConfigItem& item) const;

    std::vector<std::string> ParseOptions(const std::string& constraints) const;
    std::pair<std::string, std::string> ParseConfigKey(const std::string& fullKey) const;

    // Helper methods for HandleLongPress
    bool ValidateMenuState() const;
    void ExecuteMenuAction(const ConfigComponent::MenuItem& item);
    MenuActionType ParseActionType(const std::string& actionTypeStr) const;
    void HandleEnterSection(const std::string& sectionName);
    void HandleToggleBoolean(const std::string& fullKey);
    void HandleShowOptions(const std::string& fullKey);
    void HandleSetConfigValue(const std::string& actionParam);
    void HandleBackAction(const std::string& actionParam);
    void HandlePanelExit();

    // ========== Static Methods ==========
    static void ShowPanelCompletionCallback(lv_event_t *event);

    // ========== Private Data Members ==========

    // Instance Data Members
    IGpioProvider *gpioProvider_;
    IDisplayProvider *displayProvider_;
    IStyleManager *styleManager_;
    IPanelManager *panelManager_;
    IConfigurationManager *configurationManager_ = nullptr;
    lv_obj_t* screen_ = nullptr;

    // Component (View) - static allocation
    ConfigComponent configComponent_;
    bool componentInitialized_ = false;

    // Dynamic menu state
    std::vector<ConfigComponent::MenuItem> menuItems_;
    size_t currentMenuIndex_ = 0;
    std::string currentSectionName_;

    // Callback function removed - using interface-based notifications
};