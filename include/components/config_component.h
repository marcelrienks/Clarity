#pragma once

#include "interfaces/i_component.h"
#include "interfaces/i_panel_service.h"
#include "interfaces/i_style_service.h"
#include "definitions/types.h"
#include <functional>
#include <string>
#include <vector>

/**
 * @class ConfigComponent
 * @brief View component for configuration menu display
 *
 * @details This component handles all LVGL rendering for the configuration
 * menu system. It provides a scrolling menu interface optimized for round
 * displays with centered selection and fading effects.
 *
 * @design_pattern MVP View - Pure presentation logic, no business logic
 * @presenter ConfigPanel manages this component and handles user input
 *
 * @visual_features:
 * - Centered menu item selection
 * - Progressive fading for items above/below center
 * - Smooth scrolling through menu items
 * - Round display optimized layout
 *
 * @menu_layout:
 * - 5 visible items at a time (2 above, current, 2 below)
 * - Current item highlighted with larger font and background
 * - Items fade based on distance from center
 *
 * @context Part of MVP pattern where ConfigPanel (Presenter) manages
 * this component (View) and PreferenceManager acts as the Model.
 */
class ConfigComponent : public IComponent
{
  public:
    // Menu item structure
    struct MenuItem
    {
        std::string label;
        std::string actionType;  // Type of action: "theme_switch", "panel_load", etc.
        std::string actionParam; // Parameter for the action
    };

    // ========== Constructors and Destructor ==========
    ConfigComponent(IPanelService* panelService = nullptr, 
                   IStyleService* styleService = nullptr);
    ~ConfigComponent() = default;
    
    // ========== Public Interface Methods ==========
    void ExecuteAction(const std::string& actionType, const std::string& actionParam);

    // IComponent interface implementation
    void Render(lv_obj_t *screen, const ComponentLocation &location, IDisplayProvider *display) override;
    void SetValue(int32_t value) override;

    // ConfigComponent specific initialization
    void Init(lv_obj_t *screen);
    
    // Set style service for theme-aware colors
    void SetStyleService(IStyleService* styleService);
    
    // Update colors based on current theme
    void UpdateThemeColors();

    // Configuration menu specific methods
    void SetTitle(const std::string &title);
    void SetMenuItems(const std::vector<MenuItem> &items);
    void SetCurrentIndex(size_t index);
    void SetHintText(const std::string &hint);


  private:
    // ========== Private Methods ==========
    void CreateUI();
    void UpdateMenuDisplay();
    
    // Helper methods for UpdateMenuDisplay
    void ApplyCenterItemStyle(lv_obj_t* label);
    void ApplyCenterItemBackground(lv_obj_t* label);
    void ApplyDefaultCenterBackground(lv_obj_t* label);
    
    // Theme-aware color helpers
    lv_color_t GetThemeGradientColor(int distanceFromCenter, bool isSelected = false) const;

    // ========== Private Data Members ==========
    lv_obj_t *screen_ = nullptr;
    lv_obj_t *container_ = nullptr;
    lv_obj_t *titleLabel_ = nullptr;
    lv_obj_t *menuContainer_ = nullptr;
    std::vector<lv_obj_t *> menuLabels_;
    lv_obj_t *hintLabel_ = nullptr;

    // Menu state
    std::vector<MenuItem> menuItems_;
    size_t currentIndex_ = 0;
    std::string currentTitle_ = "Configuration";
    
    // Dependency-injected interfaces for direct calls
    IPanelService* panelService_ = nullptr;
    IStyleService* styleService_ = nullptr;

    // Constants
    static constexpr int VISIBLE_ITEMS = 5;
    static constexpr int CENTER_INDEX = 2;
    static constexpr int ITEM_HEIGHT = 24; // Reduced from 30 to 24 for tighter spacing
    static constexpr int MENU_WIDTH = 200;
};