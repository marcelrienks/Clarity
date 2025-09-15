#pragma once

#include "components/config_component.h"
#include "interfaces/i_action_service.h"
#include "interfaces/i_display_provider.h"
#include "interfaces/i_gpio_provider.h"
#include "interfaces/i_panel.h"
#include "interfaces/i_panel_service.h"
#include "interfaces/i_preference_service.h"
#include "interfaces/i_style_service.h"
#include "config/config_types.h"
#include "utilities/types.h"

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
    // Constructors and Destructors
    ConfigPanel(IGpioProvider *gpio, IDisplayProvider *display, IStyleService *styleService);
    ~ConfigPanel();

    // Core Functionality Methods
    static constexpr const char *NAME = PanelNames::CONFIG;
    void Init() override;
    void Load() override;
    void Update() override;

    // Manager injection method
    void SetManagers(IPanelService *panelService, IStyleService *styleService) override;
    void SetPreferenceService(IPreferenceService *preferenceService);

    // IActionService Interface Implementation (inherited through IPanel)
    void (*GetShortPressFunction())(void* panelContext) override;
    void (*GetLongPressFunction())(void* panelContext) override;
    void* GetPanelContext() override;
    
    // Public action handlers
    void HandleShortPress();
    void HandleLongPress();

  private:
    // Menu state enum
    enum class MenuState
    {
        MainMenu,
        PanelSubmenu,
        ThemeSubmenu,
        UpdateRateSubmenu,
        SplashSubmenu,
        SplashDurationSubmenu,
        PressureUnitSubmenu,
        TempUnitSubmenu,
        CalibrationSubmenu,
        PressureOffsetSubmenu,
        PressureScaleSubmenu,
        TempOffsetSubmenu,
        TempScaleSubmenu
    };

    // Private methods
    void InitializeMenuItems();
    void UpdateMenuItemsWithCurrentValues();
    void ExecuteCurrentOption();
    void EnterSubmenu(MenuState submenu);
    void ExitSubmenu();
    void UpdateSubmenuItems();
    void UpdateCalibration(const std::string& key, float value);

    // Dynamic configuration methods
    void BuildDynamicMenus();
    void BuildSectionMenu(const std::string& sectionName);
    std::string FormatItemLabel(const Config::ConfigItem& item) const;
    void ShowEnumSelector(const std::string& fullKey, const Config::ConfigItem& item);
    void ShowNumericEditor(const std::string& fullKey, const Config::ConfigItem& item);
    void ShowBooleanToggle(const std::string& fullKey, const Config::ConfigItem& item);
    std::vector<std::string> ParseOptions(const std::string& constraints) const;
    std::pair<std::string, std::string> ParseConfigKey(const std::string& fullKey) const;

    // Static callback
    static void ShowPanelCompletionCallback(lv_event_t *event);

    // Instance Data Members
    IGpioProvider *gpioProvider_;
    IDisplayProvider *displayProvider_;
    IStyleService *styleService_;
    IPanelService *panelService_;
    IPreferenceService *preferenceService_ = nullptr;
    bool useDynamicConfig_ = false;
    // screen_ is inherited from IPanel base class

    // Component (View) - static allocation
    ConfigComponent configComponent_;
    bool componentInitialized_ = false;

    // Menu state (Presenter logic)
    std::vector<ConfigComponent::MenuItem> menuItems_;
    size_t currentMenuIndex_ = 0;
    MenuState currentMenuState_ = MenuState::MainMenu;

    // Callback function removed - using interface-based notifications
};