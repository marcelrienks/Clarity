#include "panels/config_panel.h"
#include "managers/error_manager.h"
#include "managers/style_manager.h"
#include "utilities/types.h"
#include <Arduino.h>
#include <algorithm>
#include <cstring>

// Constructors and Destructors
ConfigPanel::ConfigPanel(IGpioProvider *gpio, IDisplayProvider *display, IStyleService *styleService)
    : gpioProvider_(gpio), displayProvider_(display), styleService_(styleService), panelService_(nullptr),
      currentMenuIndex_(0), configComponent_(std::make_unique<ConfigComponent>())
{
    log_v("ConfigPanel constructor called");
}

ConfigPanel::~ConfigPanel()
{
    log_v("~ConfigPanel() destructor called");
    
    configComponent_.reset();

    if (screen_)
    {
        lv_obj_delete(screen_);
        screen_ = nullptr;
    }
}

// Core Functionality Methods

void ConfigPanel::Init()
{
    log_v("Init() called");
    
    if (!displayProvider_)
    {
        log_e("ConfigPanel requires display provider");
        ErrorManager::Instance().ReportCriticalError("ConfigPanel", "Missing required display provider");
        return;
    }

    screen_ = displayProvider_->CreateScreen();

    if (!styleService_)
        return;

    styleService_->ApplyThemeToScreen(screen_);
    
    log_i("ConfigPanel initialization completed");
}

void ConfigPanel::Load()
{
    log_v("Load() called");
    

    // Reset menu to first item and main menu state
    currentMenuIndex_ = 0;
    currentMenuState_ = MenuState::MainMenu;

    // Apply current theme to config panel (don't change the theme, just apply existing)
    if (styleService_)
    {
        styleService_->ApplyThemeToScreen(screen_);
        
        // For night theme, override the screen background to use dark red instead of black
        const char* theme = styleService_->GetCurrentTheme();
        if (strcmp(theme, "Night") == 0) {
            lv_obj_set_style_bg_color(screen_, lv_color_hex(0x1A0000), LV_PART_MAIN); // Very dark red
            lv_obj_set_style_bg_opa(screen_, LV_OPA_COVER, LV_PART_MAIN);
        }
    }

    // Initialize the config component with the screen
    if (!configComponent_ || !screen_)
        return;

    configComponent_->Init(screen_);
    configComponent_->SetStyleService(styleService_);

    // Set initial menu items
    UpdateMenuItemsWithCurrentValues();
    configComponent_->SetMenuItems(menuItems_);
    configComponent_->SetCurrentIndex(currentMenuIndex_);

    // Register the screen loaded event callback
    lv_obj_add_event_cb(screen_, ConfigPanel::ShowPanelCompletionCallback, LV_EVENT_SCREEN_LOADED, this);
    
    lv_screen_load(screen_);
    
    log_i("ConfigPanel loaded successfully");
}

void ConfigPanel::Update()
{
    log_v("Update() called");

    // Config panel is static - no updates needed
    // No notification needed for static panels
}

// Private methods

void ConfigPanel::ExecuteCurrentOption()
{
    log_v("ExecuteCurrentOption() called");

    if (currentMenuIndex_ >= menuItems_.size())
        return;
    
    const auto& item = menuItems_[currentMenuIndex_];
    if (item.actionType.empty())
        return;
    
    // Use the component's action execution method
    if (configComponent_)
    {
        configComponent_->ExecuteAction(item.actionType, item.actionParam);
    }
}

// Static callbacks

void ConfigPanel::ShowPanelCompletionCallback(lv_event_t *event)
{
    log_v("ShowPanelCompletionCallback() called");
    if (!event)
        return;

    auto *panel = static_cast<ConfigPanel *>(lv_event_get_user_data(event));
    if (!panel)
        return;

    // Config panel completion handled - no callback needed
}

// IInputService Interface Implementation

// Legacy Action interface methods (retained for reference)
/*
Action ConfigPanel::GetShortPressAction()
{
    log_v("GetShortPressAction() called");

    // Short press cycles through menu options
    return Action(
        [this]()
        {
            currentMenuIndex_ = (currentMenuIndex_ + 1) % menuItems_.size();
            if (configComponent_)
            {
                configComponent_->SetCurrentIndex(currentMenuIndex_);

                // Update hint text based on current menu item
                if (menuItems_[currentMenuIndex_].label == "Exit")
                {
                    configComponent_->SetHintText("Short: Next | Long: Press to exit");
                }
                else
                {
                    configComponent_->SetHintText("Short: Next | Long: Select");
                }
            }
        });
}

Action ConfigPanel::GetLongPressAction()
{
    log_v("GetLongPressAction() called");

    // Long press executes current option
    return Action([this]() { ExecuteCurrentOption(); });
}
*/

// Manager injection method
void ConfigPanel::SetManagers(IPanelService *panelService, IStyleService *styleService)
{
    log_v("SetManagers() called");
    panelService_ = panelService;
    
    // styleService_ is already set in constructor, but update if different instance provided
    if (styleService != styleService_)
    {
        styleService_ = styleService;
    }
}

void ConfigPanel::SetPreferenceService(IPreferenceService *preferenceService)
{
    log_v("SetPreferenceService() called");
    preferenceService_ = preferenceService;
    // Initialize menu items now that we have access to preferences
    InitializeMenuItems();
}

void ConfigPanel::InitializeMenuItems()
{
    log_v("InitializeMenuItems() called");

    if (!preferenceService_)
    {
        log_e("Cannot initialize menu items without preference service");
        ErrorManager::Instance().ReportError(ErrorLevel::ERROR, "ConfigPanel",
                                             "Cannot initialize menu - preference service is null");
        return;
    }

    UpdateMenuItemsWithCurrentValues();
}

void ConfigPanel::UpdateMenuItemsWithCurrentValues()
{
    log_v("UpdateMenuItemsWithCurrentValues() called");

    if (!preferenceService_)
        return;

    const Configs &config = preferenceService_->GetConfig();

    // Format panel name for display (remove "Panel" suffix)
    std::string panelDisplay = config.panelName;
    if (panelDisplay.find("Panel") != std::string::npos)
    {
        panelDisplay = panelDisplay.substr(0, panelDisplay.find("Panel"));
    }

    menuItems_ = {
        {"Panel: " + panelDisplay, "submenu", "PanelSubmenu"},
        {"Theme: " + config.theme, "submenu", "ThemeSubmenu"},
        {"Splash: " + std::string(config.showSplash ? "On" : "Off"), "submenu", "SplashSubmenu"},
        {"Splash T: " + std::to_string(config.splashDuration) + "ms", "submenu", "SplashDurationSubmenu"},
        {"Rate: " + std::to_string(config.updateRate) + "ms", "submenu", "UpdateRateSubmenu"},
        {"Press: " + config.pressureUnit, "submenu", "PressureUnitSubmenu"},
        {"Temp: " + config.tempUnit, "submenu", "TempUnitSubmenu"},
        {"Calibration", "submenu", "CalibrationSubmenu"},
        {"Exit", "panel_exit", ""}};

    // Update component with new menu items
    if (configComponent_)
    {
        configComponent_->SetTitle("Configuration");
        configComponent_->SetMenuItems(menuItems_);
        configComponent_->SetCurrentIndex(currentMenuIndex_);
    }
}

void ConfigPanel::EnterSubmenu(MenuState submenu)
{
    log_v("EnterSubmenu() called");

    currentMenuState_ = submenu;
    currentMenuIndex_ = 0;
    UpdateSubmenuItems();
}

void ConfigPanel::ExitSubmenu()
{
    log_v("ExitSubmenu() called");
    currentMenuState_ = MenuState::MainMenu;
    currentMenuIndex_ = 0;
    UpdateMenuItemsWithCurrentValues();
}

void ConfigPanel::UpdateSubmenuItems()
{
    log_v("UpdateSubmenuItems() called");

    if (!preferenceService_)
        return;

    const Configs &config = preferenceService_->GetConfig();
    menuItems_.clear();
    std::string title = "Configuration";

    switch (currentMenuState_)
    {
        case MenuState::PanelSubmenu:
        {
            title = "Select Panel";
            // Currently only OemOilPanel is configurable
            // In the future, this will dynamically build from all configurable panels
            menuItems_ = {{"Oil Panel", "config_set", "panelName:OilPanel"},
                          {"Back", "submenu_back", ""}};
        }
        break;

        case MenuState::ThemeSubmenu:
        {
            title = "Select Theme";
            menuItems_ = {{"Day", "theme_set", "Day"},
                          {"Night", "theme_set", "Night"},
                          {"Back", "submenu_back", ""}};
        }
        break;

        case MenuState::UpdateRateSubmenu:
        {
            title = "Update Rate";
            menuItems_ = {{"250ms", "config_set", "updateRate:250"},
                          {"500ms", "config_set", "updateRate:500"},
                          {"1000ms", "config_set", "updateRate:1000"},
                          {"2000ms", "config_set", "updateRate:2000"},
                          {"Back", "submenu_back", ""}};
        }
        break;

        case MenuState::SplashSubmenu:
        {
            title = "Splash Screen";
            menuItems_ = {{"On", "config_set", "showSplash:true"},
                          {"Off", "config_set", "showSplash:false"},
                          {"Back", "submenu_back", ""}};
        }
        break;

        case MenuState::SplashDurationSubmenu:
        {
            title = "Splash Duration";
            menuItems_ = {{"1500ms", "config_set", "splashDuration:1500"},
                          {"1750ms", "config_set", "splashDuration:1750"},
                          {"2000ms", "config_set", "splashDuration:2000"},
                          {"2500ms", "config_set", "splashDuration:2500"},
                          {"Back", "submenu_back", ""}};
        }
        break;

        case MenuState::PressureUnitSubmenu:
        {
            title = "Pressure Unit";
            menuItems_ = {{"PSI", "config_set", "pressureUnit:PSI"},
                          {"Bar", "config_set", "pressureUnit:Bar"},
                          {"kPa", "config_set", "pressureUnit:kPa"},
                          {"Back", "submenu_back", ""}};
        }
        break;

        case MenuState::TempUnitSubmenu:
        {
            title = "Temperature Unit";
            menuItems_ = {{"C", "config_set", "tempUnit:C"},
                          {"F", "config_set", "tempUnit:F"},
                          {"Back", "submenu_back", ""}};
        }
        break;

        case MenuState::CalibrationSubmenu:
        {
            title = "Sensor Calibration";
            menuItems_ = {{"Pressure Offset", "submenu_enter", "PressureOffsetSubmenu"},
                          {"Pressure Scale", "submenu_enter", "PressureScaleSubmenu"},
                          {"Temp Offset", "submenu_enter", "TempOffsetSubmenu"},
                          {"Temp Scale", "submenu_enter", "TempScaleSubmenu"},
                          {"Back", "submenu_back", ""}};
        }
        break;

        case MenuState::PressureOffsetSubmenu:
        {
            title = "Pressure Offset";
            char offsetStr[16];
            snprintf(offsetStr, sizeof(offsetStr), "%.2f", config.pressureOffset);
            menuItems_ = {{"Current: " + std::string(offsetStr), "display_only", ""},
                          {"-1.0", "calibration_set", "pressure_offset:-1.0"},
                          {"-0.1", "calibration_set", "pressure_offset:-0.1"},
                          {"0.0", "calibration_set", "pressure_offset:0.0"},
                          {"+0.1", "calibration_set", "pressure_offset:0.1"},
                          {"+1.0", "calibration_set", "pressure_offset:1.0"},
                          {"Back", "submenu_back", ""}};
        }
        break;

        case MenuState::PressureScaleSubmenu:
        {
            title = "Pressure Scale";
            char scaleStr[16];
            snprintf(scaleStr, sizeof(scaleStr), "%.3f", config.pressureScale);
            menuItems_ = {{"Current: " + std::string(scaleStr), "display_only", ""},
                          {"0.900", "calibration_set", "pressure_scale:0.900"},
                          {"0.950", "calibration_set", "pressure_scale:0.950"},
                          {"1.000", "calibration_set", "pressure_scale:1.000"},
                          {"1.050", "calibration_set", "pressure_scale:1.050"},
                          {"1.100", "calibration_set", "pressure_scale:1.100"},
                          {"Back", "submenu_back", ""}};
        }
        break;

        case MenuState::TempOffsetSubmenu:
        {
            title = "Temperature Offset";
            char offsetStr[16];
            snprintf(offsetStr, sizeof(offsetStr), "%.1f", config.tempOffset);
            menuItems_ = {{"Current: " + std::string(offsetStr), "display_only", ""},
                          {"-5.0", "calibration_set", "temp_offset:-5.0"},
                          {"-1.0", "calibration_set", "temp_offset:-1.0"},
                          {"0.0", "calibration_set", "temp_offset:0.0"},
                          {"+1.0", "calibration_set", "temp_offset:1.0"},
                          {"+5.0", "calibration_set", "temp_offset:5.0"},
                          {"Back", "submenu_back", ""}};
        }
        break;

        case MenuState::TempScaleSubmenu:
        {
            title = "Temperature Scale";
            char scaleStr[16];
            snprintf(scaleStr, sizeof(scaleStr), "%.3f", config.tempScale);
            menuItems_ = {{"Current: " + std::string(scaleStr), "display_only", ""},
                          {"0.900", "calibration_set", "temp_scale:0.900"},
                          {"0.950", "calibration_set", "temp_scale:0.950"},
                          {"1.000", "calibration_set", "temp_scale:1.000"},
                          {"1.050", "calibration_set", "temp_scale:1.050"},
                          {"1.100", "calibration_set", "temp_scale:1.100"},
                          {"Back", "submenu_back", ""}};
        }
        break;

        default:
            // Should not happen
            break;
    }

    // Update component with new menu items and title
    if (configComponent_)
    {
        configComponent_->SetTitle(title);
        configComponent_->SetMenuItems(menuItems_);
        configComponent_->SetCurrentIndex(currentMenuIndex_);
    }
}

// IActionService Interface Implementation

static void ConfigPanelShortPress(void* panelContext)
{
    log_v("ConfigPanelShortPress() called");
    auto* panel = static_cast<ConfigPanel*>(panelContext);
    
    if (panel)
    {
        panel->HandleShortPress();
    }
}

static void ConfigPanelLongPress(void* panelContext)
{
    log_v("ConfigPanelLongPress() called");
    auto* panel = static_cast<ConfigPanel*>(panelContext);
    
    if (panel)
    {
        panel->HandleLongPress();
    }
}

void (*ConfigPanel::GetShortPressFunction())(void* panelContext)
{
    return ConfigPanelShortPress;
}

void (*ConfigPanel::GetLongPressFunction())(void* panelContext)
{
    return ConfigPanelLongPress;
}

void* ConfigPanel::GetPanelContext()
{
    return this;
}

void ConfigPanel::HandleShortPress()
{
    if (menuItems_.empty())
    {
        log_w("ConfigPanel: Cannot cycle menu - no menu items");
        return;
    }
    
    // Cycle to next menu item
    currentMenuIndex_ = (currentMenuIndex_ + 1) % menuItems_.size();
    log_i("ConfigPanel: Cycled to menu item %zu: %s", currentMenuIndex_, 
          menuItems_[currentMenuIndex_].label.c_str());
    
    // Update the UI
    if (configComponent_)
    {
        configComponent_->SetCurrentIndex(currentMenuIndex_);
    }
}

void ConfigPanel::HandleLongPress()
{
    log_i("ConfigPanel: Executing current option at index %zu", currentMenuIndex_);
    ExecuteCurrentOption();
}

void ConfigPanel::UpdateCalibration(const std::string& key, float value)
{
    log_v("UpdateCalibration() called for key: %s, value: %.3f", key.c_str(), value);
    
    if (!preferenceService_)
        return;
    
    Configs cfg = preferenceService_->GetConfig();
    
    if (key == "pressure_offset")
    {
        cfg.pressureOffset = value;
    }
    else if (key == "pressure_scale")
    {
        cfg.pressureScale = value;
    }
    else if (key == "temp_offset")
    {
        cfg.tempOffset = value;
    }
    else if (key == "temp_scale")
    {
        cfg.tempScale = value;
    }
    
    preferenceService_->SetConfig(cfg);
    preferenceService_->SaveConfig();
    
    // Refresh the submenu to show updated values
    UpdateSubmenuItems();
}