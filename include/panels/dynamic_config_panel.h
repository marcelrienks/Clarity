#pragma once

#include "config/config_menu_loader.h"
#include "interfaces/i_action_service.h"
#include "interfaces/i_display_provider.h"
#include "interfaces/i_gpio_provider.h"
#include "interfaces/i_panel.h"
#include "interfaces/i_panel_service.h"
#include "interfaces/i_preference_service.h"
#include "interfaces/i_style_service.h"
#include "utilities/types.h"

#include <memory>
#include <stack>
#include <vector>

/**
 * @class DynamicConfigPanel
 * @brief Configuration panel with JSON-driven dynamic menus
 *
 * @details This panel provides a flexible configuration interface where
 * menu structure is defined by JSON files. It supports nested menus,
 * various input types, conditional visibility, and dynamic content.
 *
 * @presenter_role Manages dynamic configuration UI
 * @data_source JSON menu definitions and preference service
 * @navigation Single button: short press cycles, long press selects
 *
 * @menu_features:
 * - Dynamic menu loading from JSON
 * - Nested submenus with back navigation
 * - Multiple input types (choice, toggle, number, action)
 * - Conditional item visibility
 * - Preference binding
 * - Action handlers
 */
class DynamicConfigPanel : public IPanel, public IActionService
{
  public:
    // Constructors and Destructors
    DynamicConfigPanel(IGpioProvider *gpio, IDisplayProvider *display, IStyleService *styleService);
    ~DynamicConfigPanel();

    // Core Functionality Methods
    static constexpr const char *NAME = PanelNames::CONFIG;
    void Init() override;
    void Load(std::function<void()> callbackFunction) override;
    void Update(std::function<void()> callbackFunction) override;

    // Manager injection method
    void SetManagers(IPanelService *panelService, IStyleService *styleService) override;
    void SetPreferenceService(IPreferenceService *preferenceService);

    // IActionService Interface Implementation
    Action GetShortPressAction() override;
    Action GetLongPressAction() override;
    bool CanProcessInput() const override;

    // IPanel override to provide input service
    IActionService *GetInputService() override
    {
        return this;
    }

  private:
    // Menu navigation state
    struct MenuLevel
    {
        std::vector<std::shared_ptr<ConfigMenuItem>> items;
        size_t selectedIndex = 0;
        std::string title;
    };

    // Private methods
    void CreateMenuUI();
    void UpdateMenuDisplay();
    void ExecuteCurrentOption();
    void LoadMenuDefinition();
    void RegisterActions();
    void RegisterConfigurablePanels();

    // Navigation methods
    void EnterSubmenu(std::shared_ptr<ConfigMenuItem> item);
    void GoBack();
    void HandleChoice(std::shared_ptr<ConfigMenuItem> item);
    void HandleNumber(std::shared_ptr<ConfigMenuItem> item);
    void HandleToggle(std::shared_ptr<ConfigMenuItem> item);
    void HandleAction(std::shared_ptr<ConfigMenuItem> item);

    // UI update methods
    void RefreshMenuItems();
    std::string GetItemDisplayText(std::shared_ptr<ConfigMenuItem> item);

    // Static callback
    static void ShowPanelCompletionCallback(lv_event_t *event);

    // Instance Data Members
    IGpioProvider *gpioProvider_;
    IDisplayProvider *displayProvider_;
    IStyleService *styleService_;
    IPanelService *panelService_;
    IPreferenceService *preferenceService_ = nullptr;

    // Menu management
    std::unique_ptr<ConfigMenuLoader> menuLoader_;
    std::stack<MenuLevel> menuStack_;
    MenuLevel currentLevel_;

    // UI elements
    lv_obj_t *titleLabel_ = nullptr;
    lv_obj_t *menuContainer_ = nullptr;
    std::vector<lv_obj_t *> menuLabels_;
    lv_obj_t *hintLabel_ = nullptr;
    lv_obj_t *valueLabel_ = nullptr; // For showing current values

    // State tracking
    bool inValueEditMode_ = false;
    float editingNumberValue_ = 0;

    // Callback function
    std::function<void()> callbackFunction_;
};