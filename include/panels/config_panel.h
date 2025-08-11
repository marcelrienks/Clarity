#pragma once

#include "interfaces/i_panel.h"
#include "interfaces/i_action_service.h"
#include "interfaces/i_panel_service.h"
#include "interfaces/i_gpio_provider.h"
#include "interfaces/i_display_provider.h"
#include "interfaces/i_style_service.h"
#include "interfaces/i_preference_service.h"
#include "utilities/types.h"

#include <utilities/lv_tools.h>
#include <vector>
#include <string>

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
 * @placeholder_phase Initial implementation with dummy options
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
 * @context This is a placeholder implementation for Phase 3.
 * Full functionality will be added in Phase 4 with actual settings.
 */
class ConfigPanel : public IPanel, public IActionService
{
public:
    // Constructors and Destructors
    ConfigPanel(IGpioProvider *gpio, IDisplayProvider *display, IStyleService *styleService);
    ~ConfigPanel();

    // Core Functionality Methods
    static constexpr const char* NAME = PanelNames::CONFIG;
    void Init() override;
    void Load(std::function<void()> callbackFunction) override;
    void Update(std::function<void()> callbackFunction) override;
    
    // Manager injection method
    void SetManagers(IPanelService* panelService, IStyleService* styleService) override;
    void SetPreferenceService(IPreferenceService* preferenceService);
    
    // IActionService Interface Implementation
    Action GetShortPressAction() override;
    Action GetLongPressAction() override;
    bool CanProcessInput() const override;
    
    // IPanel override to provide input service
    IActionService* GetInputService() override { return this; }

private:
    // Menu state enum
    enum class MenuState {
        MainMenu,
        PanelSubmenu,
        ThemeSubmenu,
        UpdateRateSubmenu
    };

    // Menu item structure
    struct MenuItem {
        std::string label;
        std::function<void()> action;
    };

    // Private methods
    void CreateMenuUI();
    void UpdateMenuDisplay();
    void ExecuteCurrentOption();
    void InitializeMenuItems();
    void UpdateMenuItemsWithCurrentValues();
    void CycleDefaultPanel();
    void CycleTheme();
    void CycleUpdateRate();
    void EnterSubmenu(MenuState submenu);
    void ExitSubmenu();
    void UpdateSubmenuItems();
    
    // Static callback
    static void ShowPanelCompletionCallback(lv_event_t *event);

    // Instance Data Members
    IGpioProvider *gpioProvider_;
    IDisplayProvider *displayProvider_;
    IStyleService *styleService_;
    IPanelService *panelService_;
    IPreferenceService *preferenceService_ = nullptr;
    // screen_ is inherited from IPanel base class
    
    // Menu state
    std::vector<MenuItem> menuItems_;
    size_t currentMenuIndex_ = 0;
    MenuState currentMenuState_ = MenuState::MainMenu;
    std::string selectedSettingValue_;
    
    // UI elements
    lv_obj_t *titleLabel_ = nullptr;
    lv_obj_t *menuContainer_ = nullptr;
    std::vector<lv_obj_t*> menuLabels_;
    lv_obj_t *hintLabel_ = nullptr;
    
    // Callback function
    std::function<void()> callbackFunction_;
};