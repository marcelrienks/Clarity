#include "components/config_component.h"
#include <Arduino.h>
#include <cmath>
#include <cstring>

// Constructor
ConfigComponent::ConfigComponent(IPanelService* panelService, IStyleService* styleService)
    : panelService_(panelService), styleService_(styleService)
{
    log_v("ConfigComponent constructor called");
}

// IComponent interface implementation
void ConfigComponent::Render(lv_obj_t *screen, const ComponentLocation &location, IDisplayProvider *display)
{
    log_v("Render() called");
    Init(screen);
}


void ConfigComponent::SetValue(int32_t value)
{
    log_v("SetValue() called");
    // Not used for config menu - updates handled via specific methods
}

void ConfigComponent::ExecuteAction(const std::string& actionType, const std::string& actionParam)
{
    log_v("ExecuteAction() called with type: %s, param: %s", actionType.c_str(), actionParam.c_str());
    
    if (actionType == "submenu")
    {
        // Handle submenu navigation - delegate back to config panel
        // This would require a callback to the ConfigPanel
    }
    else if (actionType == "panel_exit")
    {
        // Exit config panel and return to restoration panel
        if (panelService_)
        {
            const char *restorationPanel = panelService_->GetRestorationPanel();
            panelService_->CreateAndLoadPanel(restorationPanel, true);
        }
    }
    else if (actionType == "panel_load")
    {
        // Load specific panel
        if (panelService_)
        {
            panelService_->CreateAndLoadPanel(actionParam.c_str(), true);
        }
    }
    else
    {
        log_w("Unknown action type: %s", actionType.c_str());
    }
}

// ConfigComponent specific initialization
void ConfigComponent::Init(lv_obj_t *screen)
{
    log_v("Init() called");
    
    if (!screen)
    {
        log_e("ConfigComponent requires screen object");
        return;
    }


    screen_ = screen;
    
    // Create main container using full 240x240 screen size (like ErrorComponent)
    container_ = lv_obj_create(screen);
    lv_obj_set_size(container_, 240, 240); // Full screen size
    lv_obj_align(container_, LV_ALIGN_CENTER, 0, 0);
    
    // Apply circular styling to container, not screen
    lv_obj_set_style_radius(container_, 120, LV_PART_MAIN); // Make it circular (half of 240)
    
    // Default transparent background (will be updated by UpdateThemeColors if styleService is set)
    lv_obj_set_style_bg_opa(container_, LV_OPA_TRANSP, LV_PART_MAIN);

    CreateUI();
}

// Configuration menu specific methods

void ConfigComponent::SetTitle(const std::string &title)
{
    log_v("SetTitle() called");
    currentTitle_ = title;
    if (titleLabel_)
    {
        lv_label_set_text(titleLabel_, title.c_str());
    }
}

void ConfigComponent::SetMenuItems(const std::vector<MenuItem> &items)
{
    log_v("SetMenuItems() called");
    menuItems_ = items;
    UpdateMenuDisplay();
}

void ConfigComponent::SetCurrentIndex(size_t index)
{
    log_v("SetCurrentIndex() called");
    if (index < menuItems_.size())
    {
        currentIndex_ = index;
        UpdateMenuDisplay();
    }
}

void ConfigComponent::SetHintText(const std::string &hint)
{
    log_v("SetHintText() called");
    if (hintLabel_)
    {
        lv_label_set_text(hintLabel_, hint.c_str());
    }
}

void ConfigComponent::SetStyleService(IStyleService* styleService)
{
    log_v("SetStyleService() called");
    styleService_ = styleService;
    UpdateThemeColors();
}

void ConfigComponent::UpdateThemeColors()
{
    log_v("UpdateThemeColors() called");
    
    if (!styleService_)
    {
        log_w("No style service available for theme update");
        return;
    }

    
    const std::string& theme = styleService_->GetCurrentTheme();
    bool isNightTheme = (theme == "Night");
    
    // Update container background
    if (container_) {
        if (isNightTheme) {
            // Use very dark red background for night theme
            lv_obj_set_style_bg_color(container_, lv_color_hex(0x1A0000), LV_PART_MAIN);
            lv_obj_set_style_bg_opa(container_, LV_OPA_COVER, LV_PART_MAIN);
        } else {
            // Transparent to use panel's theme background
            lv_obj_set_style_bg_opa(container_, LV_OPA_TRANSP, LV_PART_MAIN);
        }
    }
    
    // Update title color
    if (titleLabel_) {
        if (isNightTheme) {
            lv_obj_set_style_text_color(titleLabel_, lv_color_hex(0xFF6666), LV_PART_MAIN); // Light red for night theme
        } else {
            lv_obj_set_style_text_color(titleLabel_, lv_color_hex(0xCCCCCC), LV_PART_MAIN); // Light gray for day theme
        }
    }
    
    // Update hint text color
    if (hintLabel_) {
        if (isNightTheme) {
            lv_obj_set_style_text_color(hintLabel_, lv_color_hex(0x993333), LV_PART_MAIN); // Darker red for night theme
        } else {
            lv_obj_set_style_text_color(hintLabel_, lv_color_hex(0x888888), LV_PART_MAIN); // Gray for day theme
        }
    }
    
    // Update menu display to refresh all menu item colors
    UpdateMenuDisplay();
}

// Private methods

lv_color_t ConfigComponent::GetThemeGradientColor(int distanceFromCenter, bool isSelected) const
{
    if (!styleService_) 
    {
        // Fallback to default colors if no style service
        return isSelected ? lv_color_hex(0xFFFFFF) : lv_color_hex(0x888888);
    }

    const ThemeColors& colors = styleService_->GetThemeColors();
    const std::string& theme = styleService_->GetCurrentTheme();
    
    if (isSelected)
    {
        // Selected item color based on theme
        if (theme == "Night")
        {
            return lv_color_hex(0xFF0000); // Bright red for selected item in night theme
        }
        else
        {
            return lv_color_hex(0xFFFFFF); // White for selected item in day theme
        }
    }
    
    // Create gradient based on theme
    if (theme == "Night")
    {
        // Night theme - shades of red based on distance
        uint32_t baseColor = 0xB00020; // Deep red from night theme
        uint8_t red = (baseColor >> 16) & 0xFF;
        uint8_t green = (baseColor >> 8) & 0xFF;
        uint8_t blue = baseColor & 0xFF;
        
        // Reduce intensity based on distance (further = darker)
        float intensity = 1.0f - (distanceFromCenter * 0.3f);
        if (intensity < 0.2f) intensity = 0.2f; // Minimum visibility
        
        red = (uint8_t)(red * intensity);
        green = (uint8_t)(green * intensity);
        blue = (uint8_t)(blue * intensity);
        
        // Gradient color calculation completed
        
        return lv_color_hex((red << 16) | (green << 8) | blue);
    }
    else
    {
        // Day theme - shades of gray based on distance  
        uint32_t baseColor = 0xEEEEEE; // Light gray from day theme
        uint8_t gray = (baseColor >> 16) & 0xFF;
        
        // Reduce intensity based on distance (further = darker)
        float intensity = 1.0f - (distanceFromCenter * 0.3f);
        if (intensity < 0.2f) intensity = 0.2f; // Minimum visibility
        
        gray = (uint8_t)(gray * intensity);
        // Grayscale color calculation completed
        return lv_color_hex((gray << 16) | (gray << 8) | gray);
    }
}

void ConfigComponent::CreateUI()
{
    log_v("CreateUI() called");
    
    // Note: Circular styling and background theme are now applied to container in Init()

    // Create title label (child of container, not screen)
    titleLabel_ = lv_label_create(container_);
    lv_label_set_text(titleLabel_, currentTitle_.c_str());
    lv_obj_set_style_text_color(titleLabel_, lv_color_hex(0xCCCCCC), LV_PART_MAIN); // Default color
    lv_obj_set_style_text_font(titleLabel_, &lv_font_montserrat_18, LV_PART_MAIN);
    lv_obj_align(titleLabel_, LV_ALIGN_TOP_MID, 0, 15);

    // Create menu container - covers most of the screen for round display (child of container)
    menuContainer_ = lv_obj_create(container_);
    lv_obj_set_size(menuContainer_, 240, 180); // Full width, most height
    lv_obj_align(menuContainer_, LV_ALIGN_CENTER, 0, 10);
    lv_obj_set_style_bg_opa(menuContainer_, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(menuContainer_, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(menuContainer_, 0, LV_PART_MAIN);
    lv_obj_clear_flag(menuContainer_, LV_OBJ_FLAG_SCROLLABLE);

    // Create menu item labels
    menuLabels_.clear();
    for (int i = 0; i < VISIBLE_ITEMS; ++i)
    {
        lv_obj_t *item = lv_label_create(menuContainer_);
        lv_obj_set_width(item, MENU_WIDTH);
        lv_obj_set_style_text_align(item, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);

        // Position items vertically
        int yOffset = -60 + (i * ITEM_HEIGHT);
        lv_obj_align(item, LV_ALIGN_CENTER, 0, yOffset);

        // Set initial style
        lv_obj_set_style_text_font(item, &lv_font_montserrat_16, LV_PART_MAIN);

        menuLabels_.push_back(item);
    }

    // Create hint label (child of container, not screen)
    hintLabel_ = lv_label_create(container_);
    lv_label_set_text(hintLabel_, "Short: Next | Long: Select");
    lv_obj_set_style_text_color(hintLabel_, lv_color_hex(0x888888), LV_PART_MAIN); // Default color
    lv_obj_set_style_text_font(hintLabel_, &lv_font_montserrat_10, LV_PART_MAIN);
    lv_obj_align(hintLabel_, LV_ALIGN_BOTTOM_MID, 0, -15);

    // Initial display update
    UpdateMenuDisplay();
}

void ConfigComponent::UpdateMenuDisplay()
{
    log_v("UpdateMenuDisplay() called");
    
    if (menuItems_.empty() || menuLabels_.empty())
    {
        log_w("Cannot update menu display - empty menu items or labels");
        return;
    }


    // Update menu items with scrolling effect
    for (int i = 0; i < VISIBLE_ITEMS && i < static_cast<int>(menuLabels_.size()); ++i)
    {
        // Calculate which menu item to show at this position
        int menuItemIndex = (static_cast<int>(currentIndex_) - CENTER_INDEX + i + static_cast<int>(menuItems_.size())) %
                            static_cast<int>(menuItems_.size());

        if (menuItemIndex >= 0 && menuItemIndex < static_cast<int>(menuItems_.size()))
        {
            lv_label_set_text(menuLabels_[i], menuItems_[menuItemIndex].label.c_str());

            // Calculate distance from center
            int distanceFromCenter = abs(i - CENTER_INDEX);

            // Apply fading effect based on distance from center
            if (i == CENTER_INDEX)
            {
                ApplyCenterItemStyle(menuLabels_[i]);
            }
            else
            {
                // Apply progressive fading and font size reduction based on distance
                uint8_t opacity = LV_OPA_100 - (distanceFromCenter * 35);

                // Use theme-appropriate gradient colors
                lv_obj_set_style_text_color(menuLabels_[i], GetThemeGradientColor(distanceFromCenter), LV_PART_MAIN);
                
                // Progressive font size reduction - more dramatic stepping
                const lv_font_t* font;
                switch (distanceFromCenter) {
                    case 1:
                        font = &lv_font_montserrat_14;
                        break;
                    case 2:
                        font = &lv_font_montserrat_12;
                        break;
                    default:
                        font = &lv_font_montserrat_8;
                        break;
                }
                lv_obj_set_style_text_font(menuLabels_[i], font, LV_PART_MAIN);
                lv_obj_set_style_text_opa(menuLabels_[i], opacity, LV_PART_MAIN);

                // Remove background and border from non-selected items
                lv_obj_set_style_bg_opa(menuLabels_[i], LV_OPA_0, LV_PART_MAIN);
                lv_obj_set_style_border_width(menuLabels_[i], 0, LV_PART_MAIN);
                lv_obj_set_style_pad_all(menuLabels_[i], 0, LV_PART_MAIN);
            }

            // Apply distance-based positioning with adjusted padding
            int baseYOffset = -60 + (i * ITEM_HEIGHT);
            int adjustedYOffset = baseYOffset;
            
            // Adjust positioning based on distance from center
            if (distanceFromCenter == 1) {
                // Adjacent settings - increase gap from selected item (bigger breathing room)
                adjustedYOffset = baseYOffset + (i < CENTER_INDEX ? -6 : 6);
            } else if (distanceFromCenter >= 2) {
                // Outermost settings - compress slightly toward center
                adjustedYOffset = baseYOffset + (i < CENTER_INDEX ? -4 : 4);
            }
            
            lv_obj_align(menuLabels_[i], LV_ALIGN_CENTER, 0, adjustedYOffset);

            // Show the label
            lv_obj_clear_flag(menuLabels_[i], LV_OBJ_FLAG_HIDDEN);
        }
        else
        {
            // Hide labels that don't have corresponding menu items
            lv_obj_add_flag(menuLabels_[i], LV_OBJ_FLAG_HIDDEN);
        }
    }
}

void ConfigComponent::ApplyCenterItemStyle(lv_obj_t* label)
{
    if (!label) return;
    
    // Center item - fully highlighted with bold styling
    lv_obj_set_style_text_color(label, GetThemeGradientColor(0, true), LV_PART_MAIN);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_20, LV_PART_MAIN);
    lv_obj_set_style_text_opa(label, LV_OPA_100, LV_PART_MAIN);

    // Enhanced background highlight
    ApplyCenterItemBackground(label);
    
    lv_obj_set_style_bg_opa(label, LV_OPA_70, LV_PART_MAIN);
    lv_obj_set_style_radius(label, 6, LV_PART_MAIN);
    lv_obj_set_style_pad_all(label, 6, LV_PART_MAIN);
    lv_obj_set_style_border_width(label, 1, LV_PART_MAIN);
}

void ConfigComponent::ApplyCenterItemBackground(lv_obj_t* label)
{
    if (!styleService_)
    {
        ApplyDefaultCenterBackground(label);
        return;
    }

    const std::string& theme = styleService_->GetCurrentTheme();
    if (theme == "Night")
    {
        lv_obj_set_style_bg_color(label, lv_color_hex(0x4D1F1F), LV_PART_MAIN);
        lv_obj_set_style_border_color(label, lv_color_hex(0x993333), LV_PART_MAIN);
        return;
    }

    // Default day theme
    lv_obj_set_style_bg_color(label, lv_color_hex(0x555555), LV_PART_MAIN);
    lv_obj_set_style_border_color(label, lv_color_hex(0x888888), LV_PART_MAIN);
}

void ConfigComponent::ApplyDefaultCenterBackground(lv_obj_t* label)
{
    lv_obj_set_style_bg_color(label, lv_color_hex(0x555555), LV_PART_MAIN);
    lv_obj_set_style_border_color(label, lv_color_hex(0x888888), LV_PART_MAIN);
}