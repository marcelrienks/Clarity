#include "components/config_component.h"
#include <cmath>
#include <Arduino.h>

// Constructor
ConfigComponent::ConfigComponent()
{
}

// IComponent interface implementation

void ConfigComponent::Render(lv_obj_t* screen, const ComponentLocation& location, IDisplayProvider* display)
{
    Init(screen);
}

void ConfigComponent::Refresh(const Reading& reading)
{
    // Not used for config menu - updates handled via specific methods
}

void ConfigComponent::SetValue(int32_t value)
{
    // Not used for config menu - updates handled via specific methods
}

// ConfigComponent specific initialization

void ConfigComponent::Init(lv_obj_t* screen)
{
    if (!screen) {
        log_e("ConfigComponent::Init - screen is NULL!");
        return;
    }
    
    screen_ = screen;
    container_ = screen;  // Use screen directly as container
    
    CreateUI();
}

// Configuration menu specific methods

void ConfigComponent::SetTitle(const std::string& title)
{
    currentTitle_ = title;
    if (titleLabel_) {
        lv_label_set_text(titleLabel_, title.c_str());
    }
}

void ConfigComponent::SetMenuItems(const std::vector<MenuItem>& items)
{
    menuItems_ = items;
    UpdateMenuDisplay();
}

void ConfigComponent::SetCurrentIndex(size_t index)
{
    if (index < menuItems_.size()) {
        currentIndex_ = index;
        UpdateMenuDisplay();
    }
}

void ConfigComponent::SetHintText(const std::string& hint)
{
    if (hintLabel_) {
        lv_label_set_text(hintLabel_, hint.c_str());
    }
}

// Private methods

void ConfigComponent::CreateUI()
{
    // Apply circular styling for round display (240px diameter, so radius = 120)
    lv_obj_set_style_radius(screen_, 120, LV_PART_MAIN);
    
    // Note: Background color and theme are applied via StyleManager in ConfigPanel
    
    // Create title label
    titleLabel_ = lv_label_create(screen_);
    lv_label_set_text(titleLabel_, currentTitle_.c_str());
    lv_obj_set_style_text_color(titleLabel_, lv_color_hex(0xCCCCCC), LV_PART_MAIN);
    lv_obj_set_style_text_font(titleLabel_, &lv_font_montserrat_18, LV_PART_MAIN);
    lv_obj_align(titleLabel_, LV_ALIGN_TOP_MID, 0, 15);
    
    // Create menu container - covers most of the screen for round display
    menuContainer_ = lv_obj_create(screen_);
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
        lv_obj_t* item = lv_label_create(menuContainer_);
        lv_obj_set_width(item, MENU_WIDTH);
        lv_obj_set_style_text_align(item, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
        
        // Position items vertically
        int yOffset = -60 + (i * ITEM_HEIGHT);
        lv_obj_align(item, LV_ALIGN_CENTER, 0, yOffset);
        
        // Set initial style
        lv_obj_set_style_text_font(item, &lv_font_montserrat_16, LV_PART_MAIN);
        
        menuLabels_.push_back(item);
    }
    
    // Create hint label
    hintLabel_ = lv_label_create(screen_);
    lv_label_set_text(hintLabel_, "Short: Next | Long: Select");
    lv_obj_set_style_text_color(hintLabel_, lv_color_hex(0x888888), LV_PART_MAIN);
    lv_obj_set_style_text_font(hintLabel_, &lv_font_montserrat_12, LV_PART_MAIN);
    lv_obj_align(hintLabel_, LV_ALIGN_BOTTOM_MID, 0, -15);
    
    // Initial display update
    UpdateMenuDisplay();
}

void ConfigComponent::UpdateMenuDisplay()
{
    // Update menu items with scrolling effect
    for (int i = 0; i < VISIBLE_ITEMS && i < static_cast<int>(menuLabels_.size()); ++i)
    {
        // Calculate which menu item to show at this position
        int menuItemIndex = (static_cast<int>(currentIndex_) - CENTER_INDEX + i + static_cast<int>(menuItems_.size())) 
                           % static_cast<int>(menuItems_.size());
        
        if (menuItemIndex >= 0 && menuItemIndex < static_cast<int>(menuItems_.size()))
        {
            lv_label_set_text(menuLabels_[i], menuItems_[menuItemIndex].label.c_str());
            
            // Calculate distance from center
            int distanceFromCenter = abs(i - CENTER_INDEX);
            
            // Apply fading effect based on distance from center
            if (i == CENTER_INDEX)
            {
                // Center item - fully highlighted
                lv_obj_set_style_text_color(menuLabels_[i], lv_color_hex(0xFFFFFF), LV_PART_MAIN);
                lv_obj_set_style_text_font(menuLabels_[i], &lv_font_montserrat_18, LV_PART_MAIN);
                lv_obj_set_style_text_opa(menuLabels_[i], LV_OPA_100, LV_PART_MAIN);
                
                // Add subtle background highlight
                lv_obj_set_style_bg_color(menuLabels_[i], lv_color_hex(0x444444), LV_PART_MAIN);
                lv_obj_set_style_bg_opa(menuLabels_[i], LV_OPA_50, LV_PART_MAIN);
                lv_obj_set_style_radius(menuLabels_[i], 5, LV_PART_MAIN);
                lv_obj_set_style_pad_all(menuLabels_[i], 8, LV_PART_MAIN);
            }
            else
            {
                // Apply progressive fading based on distance
                uint8_t opacity = LV_OPA_100 - (distanceFromCenter * 30); // Fade by 30% per step
                uint8_t colorValue = 0xCC - (distanceFromCenter * 0x33); // Fade color from 0xCC to 0x66
                
                lv_obj_set_style_text_color(menuLabels_[i], lv_color_hex(colorValue << 16 | colorValue << 8 | colorValue), LV_PART_MAIN);
                lv_obj_set_style_text_font(menuLabels_[i], &lv_font_montserrat_16, LV_PART_MAIN);
                lv_obj_set_style_text_opa(menuLabels_[i], opacity, LV_PART_MAIN);
                
                // Remove background from non-selected items
                lv_obj_set_style_bg_opa(menuLabels_[i], LV_OPA_0, LV_PART_MAIN);
                lv_obj_set_style_pad_all(menuLabels_[i], 0, LV_PART_MAIN);
            }
            
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