#include "panels/config_panel.h"
#include "managers/style_manager.h"
#include "managers/trigger_manager.h"
#include "actions/input_actions.h"
#include <Arduino.h>

// Constructors and Destructors
ConfigPanel::ConfigPanel(IGpioProvider* gpio, IDisplayProvider* display, IStyleService* styleService)
    : gpioProvider_(gpio), displayProvider_(display), styleService_(styleService),
      currentMenuIndex_(0)
{
    // Initialize menu items (placeholder options for Phase 3)
    menuItems_ = {
        {"Option 1", []() { log_i("Option 1 selected"); }},
        {"Option 2", []() { log_i("Option 2 selected"); }},
        {"Exit", [this]() { 
            log_i("Exiting config panel");
            // Return to previous panel
            // For now, just go back to OIL panel as default
            // In Phase 4, this will use proper panel restoration
        }}
    };
}

ConfigPanel::~ConfigPanel()
{
    log_d("Destroying ConfigPanel...");
    
    if (screen_)
    {
        lv_obj_delete(screen_);
        screen_ = nullptr;
    }
}

// Core Functionality Methods

void ConfigPanel::Init()
{
    log_d("Initializing ConfigPanel");
    
    if (!displayProvider_)
    {
        log_e("ConfigPanel requires display provider");
        return;
    }
    
    screen_ = displayProvider_->CreateScreen();
}

void ConfigPanel::Load(std::function<void()> callbackFunction)
{
    log_i("Loading ConfigPanel");
    
    callbackFunction_ = callbackFunction;
    
    // Reset menu to first item
    currentMenuIndex_ = 0;
    
    log_d("Creating menu UI...");
    // Create the menu UI
    CreateMenuUI();
    
    log_v("loading...");
    lv_screen_load(screen_);
    
    // Call the completion callback directly (like other panels do)
    if (callbackFunction_) {
        callbackFunction_();
    }
}

void ConfigPanel::Update(std::function<void()> callbackFunction)
{
    // Config panel is static, no regular updates needed
    callbackFunction();
}

// Private methods

void ConfigPanel::CreateMenuUI()
{
    log_d("CreateMenuUI: Starting, screen_ = %p", screen_);
    
    if (!screen_) {
        log_e("CreateMenuUI: screen_ is NULL!");
        return;
    }
    
    log_d("CreateMenuUI: Setting background color...");
    // Apply grey theme for settings
    lv_obj_set_style_bg_color(screen_, lv_color_hex(0x2C2C2C), LV_PART_MAIN);
    
    log_d("CreateMenuUI: Creating title label...");
    // Create title
    titleLabel_ = lv_label_create(screen_);
    log_d("CreateMenuUI: Setting title text...");
    lv_label_set_text(titleLabel_, "Configuration");
    log_d("CreateMenuUI: Setting title color...");
    lv_obj_set_style_text_color(titleLabel_, lv_color_hex(0xCCCCCC), LV_PART_MAIN);
    log_d("CreateMenuUI: Setting title font...");
    lv_obj_set_style_text_font(titleLabel_, &lv_font_montserrat_20, LV_PART_MAIN);
    log_d("CreateMenuUI: Aligning title...");
    lv_obj_align(titleLabel_, LV_ALIGN_TOP_MID, 0, 20);
    
    log_d("CreateMenuUI: Creating menu container...");
    // Create menu container
    menuContainer_ = lv_obj_create(screen_);
    lv_obj_set_size(menuContainer_, 200, 120);
    lv_obj_align(menuContainer_, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_bg_color(menuContainer_, lv_color_hex(0x3C3C3C), LV_PART_MAIN);
    lv_obj_set_style_border_width(menuContainer_, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(menuContainer_, 10, LV_PART_MAIN);
    lv_obj_set_flex_flow(menuContainer_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(menuContainer_, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    
    // Create menu items
    menuLabels_.clear();
    for (size_t i = 0; i < menuItems_.size(); ++i)
    {
        lv_obj_t* item = lv_label_create(menuContainer_);
        lv_label_set_text(item, menuItems_[i].label.c_str());
        lv_obj_set_style_text_color(item, lv_color_hex(0xAAAAAA), LV_PART_MAIN);
        lv_obj_set_style_text_font(item, &lv_font_montserrat_16, LV_PART_MAIN);
        lv_obj_set_width(item, lv_pct(100));
        lv_obj_set_style_pad_ver(item, 5, LV_PART_MAIN);
        
        menuLabels_.push_back(item);
    }
    
    // Create hint label
    hintLabel_ = lv_label_create(screen_);
    lv_label_set_text(hintLabel_, "Short: Next | Long: Select");
    lv_obj_set_style_text_color(hintLabel_, lv_color_hex(0x888888), LV_PART_MAIN);
    lv_obj_set_style_text_font(hintLabel_, &lv_font_montserrat_12, LV_PART_MAIN);
    lv_obj_align(hintLabel_, LV_ALIGN_BOTTOM_MID, 0, -20);
    
    // Update display to show current selection
    UpdateMenuDisplay();
}

void ConfigPanel::UpdateMenuDisplay()
{
    // Update all menu items
    for (size_t i = 0; i < menuLabels_.size(); ++i)
    {
        if (i == currentMenuIndex_)
        {
            // Highlight current item
            lv_obj_set_style_text_color(menuLabels_[i], lv_color_hex(0xFFFFFF), LV_PART_MAIN);
            lv_obj_set_style_bg_color(menuLabels_[i], lv_color_hex(0x555555), LV_PART_MAIN);
            lv_obj_set_style_bg_opa(menuLabels_[i], LV_OPA_100, LV_PART_MAIN);
            lv_obj_set_style_pad_hor(menuLabels_[i], 10, LV_PART_MAIN);
        }
        else
        {
            // Normal item
            lv_obj_set_style_text_color(menuLabels_[i], lv_color_hex(0xAAAAAA), LV_PART_MAIN);
            lv_obj_set_style_bg_opa(menuLabels_[i], LV_OPA_0, LV_PART_MAIN);
            lv_obj_set_style_pad_hor(menuLabels_[i], 0, LV_PART_MAIN);
        }
    }
}

void ConfigPanel::ExecuteCurrentOption()
{
    if (currentMenuIndex_ < menuItems_.size() && menuItems_[currentMenuIndex_].action)
    {
        menuItems_[currentMenuIndex_].action();
    }
}

// Static callbacks

void ConfigPanel::ShowPanelCompletionCallback(lv_event_t *event)
{
    log_d("Config panel loaded successfully");
    
    auto* panel = static_cast<ConfigPanel*>(lv_event_get_user_data(event));
    if (panel && panel->callbackFunction_)
    {
        panel->callbackFunction_();
    }
}

// IInputService Interface Implementation

std::unique_ptr<IInputAction> ConfigPanel::GetShortPressAction()
{
    // Short press cycles through menu options
    return std::make_unique<MenuNavigationAction>(MenuNavigationAction::NEXT,
        [this](MenuNavigationAction::Direction) {
            currentMenuIndex_ = (currentMenuIndex_ + 1) % menuItems_.size();
            log_i("ConfigPanel: Short press - selected '%s'", menuItems_[currentMenuIndex_].label.c_str());
            UpdateMenuDisplay();
        }
    );
}

std::unique_ptr<IInputAction> ConfigPanel::GetLongPressAction()
{
    // Long press executes current option
    return std::make_unique<MenuNavigationAction>(MenuNavigationAction::SELECT,
        [this](MenuNavigationAction::Direction) {
            log_i("ConfigPanel: Long press - executing '%s'", menuItems_[currentMenuIndex_].label.c_str());
            ExecuteCurrentOption();
        }
    );
}

bool ConfigPanel::CanProcessInput() const
{
    // ConfigPanel can always process input (no animations that block input)
    return true;
}