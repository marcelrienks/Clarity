#include "panels/config_panel.h"
#include "managers/style_manager.h"
#include "managers/trigger_manager.h"
#include "utilities/types.h"
#include <Arduino.h>
#include <algorithm>

// Constructors and Destructors
ConfigPanel::ConfigPanel(IGpioProvider* gpio, IDisplayProvider* display, IStyleService* styleService)
    : gpioProvider_(gpio), displayProvider_(display), styleService_(styleService), panelService_(nullptr),
      currentMenuIndex_(0)
{
    // Menu items will be initialized after preferences are injected
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
    
    // Reset menu to first item and main menu state
    currentMenuIndex_ = 0;
    currentMenuState_ = MenuState::MainMenu;
    
    // Creating menu UI
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
    // CreateMenuUI: Starting
    
    if (!screen_) {
        log_e("CreateMenuUI: screen_ is NULL!");
        return;
    }
    
    // CreateMenuUI: Setting background color
    // Apply grey theme for settings
    lv_obj_set_style_bg_color(screen_, lv_color_hex(0x2C2C2C), LV_PART_MAIN);
    
    // CreateMenuUI: Creating title label
    // Create title
    titleLabel_ = lv_label_create(screen_);
    // CreateMenuUI: Setting title text
    lv_label_set_text(titleLabel_, "Configuration");
    // CreateMenuUI: Setting title color
    lv_obj_set_style_text_color(titleLabel_, lv_color_hex(0xCCCCCC), LV_PART_MAIN);
    // CreateMenuUI: Setting title font
    lv_obj_set_style_text_font(titleLabel_, &lv_font_montserrat_20, LV_PART_MAIN);
    // CreateMenuUI: Aligning title
    lv_obj_align(titleLabel_, LV_ALIGN_TOP_MID, 0, 20);
    
    // CreateMenuUI: Creating menu container
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
    lv_label_set_text(hintLabel_, "Short: Cycle | Long: Apply");
    lv_obj_set_style_text_color(hintLabel_, lv_color_hex(0x888888), LV_PART_MAIN);
    lv_obj_set_style_text_font(hintLabel_, &lv_font_montserrat_12, LV_PART_MAIN);
    lv_obj_align(hintLabel_, LV_ALIGN_BOTTOM_MID, 0, -20);
    
    // Update display to show current selection
    UpdateMenuDisplay();
}

void ConfigPanel::UpdateMenuDisplay()
{
    // Update title based on current menu state
    if (titleLabel_) {
        switch (currentMenuState_) {
            case MenuState::MainMenu:
                lv_label_set_text(titleLabel_, "Configuration");
                break;
            case MenuState::PanelSubmenu:
                lv_label_set_text(titleLabel_, "Select Panel");
                break;
            case MenuState::ThemeSubmenu:
                lv_label_set_text(titleLabel_, "Select Theme");
                break;
            case MenuState::UpdateRateSubmenu:
                lv_label_set_text(titleLabel_, "Update Rate");
                break;
        }
    }
    
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
    // Config panel loaded successfully
    
    auto* panel = static_cast<ConfigPanel*>(lv_event_get_user_data(event));
    if (panel && panel->callbackFunction_)
    {
        panel->callbackFunction_();
    }
}

// IInputService Interface Implementation

Action ConfigPanel::GetShortPressAction()
{
    // Short press cycles through menu options
    return Action([this]() {
        currentMenuIndex_ = (currentMenuIndex_ + 1) % menuItems_.size();
        log_i("ConfigPanel: Short press - selected '%s'", menuItems_[currentMenuIndex_].label.c_str());
        UpdateMenuDisplay();
    });
}

Action ConfigPanel::GetLongPressAction()
{
    // Long press executes current option
    return Action([this]() {
        log_i("ConfigPanel: Long press - executing '%s'", menuItems_[currentMenuIndex_].label.c_str());
        ExecuteCurrentOption();
    });
}

bool ConfigPanel::CanProcessInput() const
{
    // ConfigPanel can always process input (no animations that block input)
    return true;
}

// Manager injection method
void ConfigPanel::SetManagers(IPanelService* panelService, IStyleService* styleService)
{
    panelService_ = panelService;
    // styleService_ is already set in constructor, but update if different instance provided
    if (styleService != styleService_) {
        styleService_ = styleService;
    }
    // Managers injected successfully
}

void ConfigPanel::SetPreferenceService(IPreferenceService* preferenceService)
{
    preferenceService_ = preferenceService;
    // Initialize menu items now that we have access to preferences
    InitializeMenuItems();
}

void ConfigPanel::InitializeMenuItems()
{
    if (!preferenceService_) {
        log_e("Cannot initialize menu items without preference service");
        return;
    }

    UpdateMenuItemsWithCurrentValues();
}

void ConfigPanel::UpdateMenuItemsWithCurrentValues()
{
    if (!preferenceService_) return;
    
    const Configs& config = preferenceService_->GetConfig();
    
    // Format panel name for display (remove "Panel" suffix)
    std::string panelDisplay = config.panelName;
    if (panelDisplay.find("Panel") != std::string::npos) {
        panelDisplay = panelDisplay.substr(0, panelDisplay.find("Panel"));
    }
    
    menuItems_ = {
        {"Panel: " + panelDisplay, [this]() { EnterSubmenu(MenuState::PanelSubmenu); }},
        {"Theme: " + config.theme, [this]() { EnterSubmenu(MenuState::ThemeSubmenu); }},
        {"Splash: " + std::string(config.showSplash ? "On" : "Off"), [this]() { EnterSubmenu(MenuState::SplashSubmenu); }},
        {"Splash Time: " + std::to_string(config.splashDuration) + "ms", [this]() { EnterSubmenu(MenuState::SplashDurationSubmenu); }},
        {"Rate: " + std::to_string(config.updateRate) + "ms", [this]() { EnterSubmenu(MenuState::UpdateRateSubmenu); }},
        {"Pressure: " + config.pressureUnit, [this]() { EnterSubmenu(MenuState::PressureUnitSubmenu); }},
        {"Temp: " + config.tempUnit, [this]() { EnterSubmenu(MenuState::TempUnitSubmenu); }},
        {"Exit", [this]() { 
            log_i("Exiting config panel and saving settings");
            // Save configuration before exiting
            if (preferenceService_) {
                preferenceService_->SaveConfig();
                log_i("Configuration saved");
            }
            // Return to previous panel using restoration panel
            if (panelService_) {
                const char* restorationPanel = panelService_->GetRestorationPanel();
                log_i("Returning to restoration panel: %s", restorationPanel);
                panelService_->CreateAndLoadPanel(restorationPanel, []() {
                    // Panel switch callback handled by service
                }, false);
            }
        }}
    };
}

void ConfigPanel::CycleDefaultPanel()
{
    if (!preferenceService_) return;
    
    Configs config = preferenceService_->GetConfig();
    
    // Cycle through available panels
    std::vector<const char*> panels = {
        PanelNames::OIL,
        PanelNames::KEY,
        PanelNames::LOCK
    };
    
    // Find current panel and move to next
    auto it = std::find(panels.begin(), panels.end(), config.panelName);
    if (it != panels.end()) {
        ++it;
        if (it == panels.end()) {
            it = panels.begin(); // Wrap around
        }
    } else {
        it = panels.begin(); // Default to first if not found
    }
    
    config.panelName = *it;
    preferenceService_->SetConfig(config);
    preferenceService_->SaveConfig();
    
    log_i("Default panel changed to: %s", config.panelName.c_str());
}

void ConfigPanel::CycleTheme()
{
    if (!preferenceService_) return;
    
    Configs config = preferenceService_->GetConfig();
    
    // Cycle through available themes
    if (config.theme == Themes::DAY) {
        config.theme = Themes::NIGHT;
    } else {
        config.theme = Themes::DAY;
    }
    
    preferenceService_->SetConfig(config);
    preferenceService_->SaveConfig();
    
    // Apply theme immediately
    if (styleService_) {
        styleService_->SetTheme(config.theme.c_str());
    }
    
    log_i("Theme changed to: %s", config.theme.c_str());
}

void ConfigPanel::CycleUpdateRate()
{
    if (!preferenceService_) return;
    
    Configs config = preferenceService_->GetConfig();
    
    // Cycle through common update rates
    std::vector<int> rates = {250, 500, 1000, 2000};
    
    auto it = std::find(rates.begin(), rates.end(), config.updateRate);
    if (it != rates.end()) {
        ++it;
        if (it == rates.end()) {
            it = rates.begin(); // Wrap around
        }
    } else {
        it = rates.begin(); // Default to first if not found
    }
    
    config.updateRate = *it;
    preferenceService_->SetConfig(config);
    preferenceService_->SaveConfig();
    
    log_i("Update rate changed to: %d ms", config.updateRate);
}

void ConfigPanel::EnterSubmenu(MenuState submenu)
{
    log_i("Entering submenu: %d", static_cast<int>(submenu));
    currentMenuState_ = submenu;
    currentMenuIndex_ = 0;
    UpdateSubmenuItems();
    UpdateMenuDisplay();
}

void ConfigPanel::ExitSubmenu()
{
    log_i("Exiting submenu, returning to main menu");
    currentMenuState_ = MenuState::MainMenu;
    currentMenuIndex_ = 0;
    UpdateMenuItemsWithCurrentValues();
    UpdateMenuDisplay();
}

void ConfigPanel::UpdateSubmenuItems()
{
    if (!preferenceService_) return;
    
    const Configs& config = preferenceService_->GetConfig();
    menuItems_.clear();
    
    switch (currentMenuState_) {
        case MenuState::PanelSubmenu:
            {
                // Currently only OemOilPanel is configurable
                // In the future, this will dynamically build from all configurable panels
                menuItems_ = {
                    {"Oil Panel", [this]() {
                        Configs cfg = preferenceService_->GetConfig();
                        cfg.panelName = PanelNames::OIL;
                        preferenceService_->SetConfig(cfg);
                        preferenceService_->SaveConfig();
                        ExitSubmenu();
                    }},
                    {"Back", [this]() { ExitSubmenu(); }}
                };
            }
            break;
            
        case MenuState::ThemeSubmenu:
            {
                menuItems_ = {
                    {"Day", [this]() {
                        Configs cfg = preferenceService_->GetConfig();
                        cfg.theme = Themes::DAY;
                        preferenceService_->SetConfig(cfg);
                        preferenceService_->SaveConfig();
                        if (styleService_) {
                            styleService_->SetTheme(cfg.theme.c_str());
                        }
                        ExitSubmenu();
                    }},
                    {"Night", [this]() {
                        Configs cfg = preferenceService_->GetConfig();
                        cfg.theme = Themes::NIGHT;
                        preferenceService_->SetConfig(cfg);
                        preferenceService_->SaveConfig();
                        if (styleService_) {
                            styleService_->SetTheme(cfg.theme.c_str());
                        }
                        ExitSubmenu();
                    }},
                    {"Back", [this]() { ExitSubmenu(); }}
                };
            }
            break;
            
        case MenuState::UpdateRateSubmenu:
            {
                menuItems_ = {
                    {"250ms", [this]() {
                        Configs cfg = preferenceService_->GetConfig();
                        cfg.updateRate = 250;
                        preferenceService_->SetConfig(cfg);
                        preferenceService_->SaveConfig();
                        ExitSubmenu();
                    }},
                    {"500ms", [this]() {
                        Configs cfg = preferenceService_->GetConfig();
                        cfg.updateRate = 500;
                        preferenceService_->SetConfig(cfg);
                        preferenceService_->SaveConfig();
                        ExitSubmenu();
                    }},
                    {"1000ms", [this]() {
                        Configs cfg = preferenceService_->GetConfig();
                        cfg.updateRate = 1000;
                        preferenceService_->SetConfig(cfg);
                        preferenceService_->SaveConfig();
                        ExitSubmenu();
                    }},
                    {"2000ms", [this]() {
                        Configs cfg = preferenceService_->GetConfig();
                        cfg.updateRate = 2000;
                        preferenceService_->SetConfig(cfg);
                        preferenceService_->SaveConfig();
                        ExitSubmenu();
                    }},
                    {"Back", [this]() { ExitSubmenu(); }}
                };
            }
            break;
            
        case MenuState::SplashSubmenu:
            {
                menuItems_ = {
                    {"On", [this]() {
                        Configs cfg = preferenceService_->GetConfig();
                        cfg.showSplash = true;
                        preferenceService_->SetConfig(cfg);
                        preferenceService_->SaveConfig();
                        ExitSubmenu();
                    }},
                    {"Off", [this]() {
                        Configs cfg = preferenceService_->GetConfig();
                        cfg.showSplash = false;
                        preferenceService_->SetConfig(cfg);
                        preferenceService_->SaveConfig();
                        ExitSubmenu();
                    }},
                    {"Back", [this]() { ExitSubmenu(); }}
                };
            }
            break;
            
        case MenuState::SplashDurationSubmenu:
            {
                menuItems_ = {
                    {"1000ms", [this]() {
                        Configs cfg = preferenceService_->GetConfig();
                        cfg.splashDuration = 1000;
                        preferenceService_->SetConfig(cfg);
                        preferenceService_->SaveConfig();
                        ExitSubmenu();
                    }},
                    {"2000ms", [this]() {
                        Configs cfg = preferenceService_->GetConfig();
                        cfg.splashDuration = 2000;
                        preferenceService_->SetConfig(cfg);
                        preferenceService_->SaveConfig();
                        ExitSubmenu();
                    }},
                    {"3000ms", [this]() {
                        Configs cfg = preferenceService_->GetConfig();
                        cfg.splashDuration = 3000;
                        preferenceService_->SetConfig(cfg);
                        preferenceService_->SaveConfig();
                        ExitSubmenu();
                    }},
                    {"5000ms", [this]() {
                        Configs cfg = preferenceService_->GetConfig();
                        cfg.splashDuration = 5000;
                        preferenceService_->SetConfig(cfg);
                        preferenceService_->SaveConfig();
                        ExitSubmenu();
                    }},
                    {"Back", [this]() { ExitSubmenu(); }}
                };
            }
            break;
            
        case MenuState::PressureUnitSubmenu:
            {
                menuItems_ = {
                    {"PSI", [this]() {
                        Configs cfg = preferenceService_->GetConfig();
                        cfg.pressureUnit = "PSI";
                        preferenceService_->SetConfig(cfg);
                        preferenceService_->SaveConfig();
                        ExitSubmenu();
                    }},
                    {"Bar", [this]() {
                        Configs cfg = preferenceService_->GetConfig();
                        cfg.pressureUnit = "Bar";
                        preferenceService_->SetConfig(cfg);
                        preferenceService_->SaveConfig();
                        ExitSubmenu();
                    }},
                    {"kPa", [this]() {
                        Configs cfg = preferenceService_->GetConfig();
                        cfg.pressureUnit = "kPa";
                        preferenceService_->SetConfig(cfg);
                        preferenceService_->SaveConfig();
                        ExitSubmenu();
                    }},
                    {"Back", [this]() { ExitSubmenu(); }}
                };
            }
            break;
            
        case MenuState::TempUnitSubmenu:
            {
                menuItems_ = {
                    {"C", [this]() {
                        Configs cfg = preferenceService_->GetConfig();
                        cfg.tempUnit = "C";
                        preferenceService_->SetConfig(cfg);
                        preferenceService_->SaveConfig();
                        ExitSubmenu();
                    }},
                    {"F", [this]() {
                        Configs cfg = preferenceService_->GetConfig();
                        cfg.tempUnit = "F";
                        preferenceService_->SetConfig(cfg);
                        preferenceService_->SaveConfig();
                        ExitSubmenu();
                    }},
                    {"Back", [this]() { ExitSubmenu(); }}
                };
            }
            break;
            
        default:
            // Should not happen
            break;
    }
    
    // Rebuild UI with new menu items
    if (menuContainer_ && !menuLabels_.empty()) {
        // Clear existing labels
        for (auto* label : menuLabels_) {
            lv_obj_delete(label);
        }
        menuLabels_.clear();
        
        // Create new labels
        for (size_t i = 0; i < menuItems_.size(); ++i) {
            lv_obj_t* item = lv_label_create(menuContainer_);
            lv_label_set_text(item, menuItems_[i].label.c_str());
            lv_obj_set_style_text_color(item, lv_color_hex(0xAAAAAA), LV_PART_MAIN);
            lv_obj_set_style_text_font(item, &lv_font_montserrat_16, LV_PART_MAIN);
            lv_obj_set_width(item, lv_pct(100));
            lv_obj_set_style_pad_ver(item, 5, LV_PART_MAIN);
            
            menuLabels_.push_back(item);
        }
    }
}