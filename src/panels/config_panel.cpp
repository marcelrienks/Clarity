#include "panels/config_panel.h"
#include "managers/error_manager.h"
#include "managers/style_manager.h"
#include "managers/trigger_manager.h"
#include "utilities/types.h"
#include <Arduino.h>
#include <algorithm>
#include <cstring>

// Constructors and Destructors
ConfigPanel::ConfigPanel(IGpioProvider *gpio, IDisplayProvider *display, IStyleService *styleService)
    : gpioProvider_(gpio), displayProvider_(display), styleService_(styleService), panelService_(nullptr),
      currentMenuIndex_(0), configComponent_(std::make_unique<ConfigComponent>())
{
    // Menu items will be initialized after preferences are injected
}

ConfigPanel::~ConfigPanel()
{

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
}

void ConfigPanel::Load(std::function<void()> callbackFunction)
{
    log_i("Loading ConfigPanel");

    callbackFunction_ = callbackFunction;
    
    // Set BUSY at start of load
    if (panelService_)
    {
        panelService_->SetUiState(UIState::BUSY);
    }

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
}

void ConfigPanel::Update(std::function<void()> callbackFunction)
{
    // Set BUSY at start of update
    if (panelService_)
    {
        panelService_->SetUiState(UIState::BUSY);
    }
    
    // Config panel is static, no regular updates needed
    // Set IDLE immediately since no processing is needed
    if (panelService_)
    {
        panelService_->SetUiState(UIState::IDLE);
    }
    
    callbackFunction();
}

// Private methods

void ConfigPanel::ExecuteCurrentOption()
{
    if (currentMenuIndex_ >= menuItems_.size())
        return;
    
    if (!menuItems_[currentMenuIndex_].action)
        return;
    
    menuItems_[currentMenuIndex_].action();
}

// Static callbacks

void ConfigPanel::ShowPanelCompletionCallback(lv_event_t *event)
{
    if (!event)
        return;

    auto *panel = static_cast<ConfigPanel *>(lv_event_get_user_data(event));
    if (!panel)
        return;

    // Set IDLE state when loading is complete
    if (panel->panelService_)
    {
        panel->panelService_->SetUiState(UIState::IDLE);
    }
    
    if (panel->callbackFunction_)
    {
        panel->callbackFunction_();
    }
}

// IInputService Interface Implementation

Action ConfigPanel::GetShortPressAction()
{
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
    // Long press executes current option
    return Action([this]() { ExecuteCurrentOption(); });
}

// Manager injection method
void ConfigPanel::SetManagers(IPanelService *panelService, IStyleService *styleService)
{
    panelService_ = panelService;
    
    // styleService_ is already set in constructor, but update if different instance provided
    if (styleService != styleService_)
    {
        styleService_ = styleService;
    }
}

void ConfigPanel::SetPreferenceService(IPreferenceService *preferenceService)
{
    preferenceService_ = preferenceService;
    // Initialize menu items now that we have access to preferences
    InitializeMenuItems();
}

void ConfigPanel::InitializeMenuItems()
{
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
        {"Panel: " + panelDisplay, [this]() { EnterSubmenu(MenuState::PanelSubmenu); }},
        {"Theme: " + config.theme, [this]() { EnterSubmenu(MenuState::ThemeSubmenu); }},
        {"Splash: " + std::string(config.showSplash ? "On" : "Off"),
         [this]() { EnterSubmenu(MenuState::SplashSubmenu); }},
        {"Splash T: " + std::to_string(config.splashDuration) + "ms",
         [this]() { EnterSubmenu(MenuState::SplashDurationSubmenu); }},
        {"Rate: " + std::to_string(config.updateRate) + "ms", [this]() { EnterSubmenu(MenuState::UpdateRateSubmenu); }},
        {"Press: " + config.pressureUnit, [this]() { EnterSubmenu(MenuState::PressureUnitSubmenu); }},
        {"Temp: " + config.tempUnit, [this]() { EnterSubmenu(MenuState::TempUnitSubmenu); }},
        {"Exit", [this]()
         {
             // Save configuration before exiting
             if (preferenceService_)
             {
                 preferenceService_->SaveConfig();
             }
             // Return to previous panel using restoration panel
             if (panelService_)
             {
                 const char *restorationPanel = panelService_->GetRestorationPanel();
                 // Use isTriggerDriven=true to prevent splash screen on programmatic panel switches
                 panelService_->CreateAndLoadPanel(
                     restorationPanel,
                     []()
                     {
                         // Panel switch callback handled by service
                     },
                     true);
             }
         }}};

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
    currentMenuState_ = submenu;
    currentMenuIndex_ = 0;
    UpdateSubmenuItems();
}

void ConfigPanel::ExitSubmenu()
{
    currentMenuState_ = MenuState::MainMenu;
    currentMenuIndex_ = 0;
    UpdateMenuItemsWithCurrentValues();
}

void ConfigPanel::UpdateSubmenuItems()
{
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
            menuItems_ = {{"Oil Panel",
                           [this]()
                           {
                               Configs cfg = preferenceService_->GetConfig();
                               cfg.panelName = PanelNames::OIL;
                               preferenceService_->SetConfig(cfg);
                               preferenceService_->SaveConfig();
                               ExitSubmenu();
                           }},
                          {"Back", [this]() { ExitSubmenu(); }}};
        }
        break;

        case MenuState::ThemeSubmenu:
        {
            title = "Select Theme";
            menuItems_ = {{"Day",
                           [this]()
                           {
                               Configs cfg = preferenceService_->GetConfig();
                               cfg.theme = Themes::DAY;
                               preferenceService_->SetConfig(cfg);
                               preferenceService_->SaveConfig();
                               if (styleService_)
                               {
                                   styleService_->SetTheme(cfg.theme.c_str());
                                   styleService_->ApplyThemeToScreen(screen_);
                                   // Override screen background for config panel
                                   lv_obj_set_style_bg_color(screen_, lv_color_hex(0x121212), LV_PART_MAIN); // Day theme gray
                                   lv_obj_set_style_bg_opa(screen_, LV_OPA_COVER, LV_PART_MAIN);
                                   // Update component theme colors
                                   if (configComponent_) {
                                       configComponent_->UpdateThemeColors();
                                   }
                               }
                               ExitSubmenu();
                           }},
                          {"Night",
                           [this]()
                           {
                               Configs cfg = preferenceService_->GetConfig();
                               cfg.theme = Themes::NIGHT;
                               preferenceService_->SetConfig(cfg);
                               preferenceService_->SaveConfig();
                               if (styleService_)
                               {
                                   styleService_->SetTheme(cfg.theme.c_str());
                                   styleService_->ApplyThemeToScreen(screen_);
                                   // Override screen background for config panel
                                   lv_obj_set_style_bg_color(screen_, lv_color_hex(0x1A0000), LV_PART_MAIN); // Night theme dark red
                                   lv_obj_set_style_bg_opa(screen_, LV_OPA_COVER, LV_PART_MAIN);
                                   // Update component theme colors
                                   if (configComponent_) {
                                       configComponent_->UpdateThemeColors();
                                   }
                               }
                               ExitSubmenu();
                           }},
                          {"Back", [this]() { ExitSubmenu(); }}};
        }
        break;

        case MenuState::UpdateRateSubmenu:
        {
            title = "Update Rate";
            menuItems_ = {{"250ms",
                           [this]()
                           {
                               Configs cfg = preferenceService_->GetConfig();
                               cfg.updateRate = 250;
                               preferenceService_->SetConfig(cfg);
                               preferenceService_->SaveConfig();
                               ExitSubmenu();
                           }},
                          {"500ms",
                           [this]()
                           {
                               Configs cfg = preferenceService_->GetConfig();
                               cfg.updateRate = 500;
                               preferenceService_->SetConfig(cfg);
                               preferenceService_->SaveConfig();
                               ExitSubmenu();
                           }},
                          {"1000ms",
                           [this]()
                           {
                               Configs cfg = preferenceService_->GetConfig();
                               cfg.updateRate = 1000;
                               preferenceService_->SetConfig(cfg);
                               preferenceService_->SaveConfig();
                               ExitSubmenu();
                           }},
                          {"2000ms",
                           [this]()
                           {
                               Configs cfg = preferenceService_->GetConfig();
                               cfg.updateRate = 2000;
                               preferenceService_->SetConfig(cfg);
                               preferenceService_->SaveConfig();
                               ExitSubmenu();
                           }},
                          {"Back", [this]() { ExitSubmenu(); }}};
        }
        break;

        case MenuState::SplashSubmenu:
        {
            title = "Splash Screen";
            menuItems_ = {{"On",
                           [this]()
                           {
                               Configs cfg = preferenceService_->GetConfig();
                               cfg.showSplash = true;
                               preferenceService_->SetConfig(cfg);
                               preferenceService_->SaveConfig();
                               ExitSubmenu();
                           }},
                          {"Off",
                           [this]()
                           {
                               Configs cfg = preferenceService_->GetConfig();
                               cfg.showSplash = false;
                               preferenceService_->SetConfig(cfg);
                               preferenceService_->SaveConfig();
                               ExitSubmenu();
                           }},
                          {"Back", [this]() { ExitSubmenu(); }}};
        }
        break;

        case MenuState::SplashDurationSubmenu:
        {
            title = "Splash Duration";
            menuItems_ = {{"1500ms",
                           [this]()
                           {
                               Configs cfg = preferenceService_->GetConfig();
                               cfg.splashDuration = 1500;
                               preferenceService_->SetConfig(cfg);
                               preferenceService_->SaveConfig();
                               ExitSubmenu();
                           }},
                          {"1750ms",
                           [this]()
                           {
                               Configs cfg = preferenceService_->GetConfig();
                               cfg.splashDuration = 1750;
                               preferenceService_->SetConfig(cfg);
                               preferenceService_->SaveConfig();
                               ExitSubmenu();
                           }},
                          {"2000ms",
                           [this]()
                           {
                               Configs cfg = preferenceService_->GetConfig();
                               cfg.splashDuration = 2000;
                               preferenceService_->SetConfig(cfg);
                               preferenceService_->SaveConfig();
                               ExitSubmenu();
                           }},
                          {"2000ms",
                           [this]()
                           {
                               Configs cfg = preferenceService_->GetConfig();
                               cfg.splashDuration = 2500;
                               preferenceService_->SetConfig(cfg);
                               preferenceService_->SaveConfig();
                               ExitSubmenu();
                           }},
                          {"Back", [this]() { ExitSubmenu(); }}};
        }
        break;

        case MenuState::PressureUnitSubmenu:
        {
            title = "Pressure Unit";
            menuItems_ = {{"PSI",
                           [this]()
                           {
                               Configs cfg = preferenceService_->GetConfig();
                               cfg.pressureUnit = "PSI";
                               preferenceService_->SetConfig(cfg);
                               preferenceService_->SaveConfig();
                               ExitSubmenu();
                           }},
                          {"Bar",
                           [this]()
                           {
                               Configs cfg = preferenceService_->GetConfig();
                               cfg.pressureUnit = "Bar";
                               preferenceService_->SetConfig(cfg);
                               preferenceService_->SaveConfig();
                               ExitSubmenu();
                           }},
                          {"kPa",
                           [this]()
                           {
                               Configs cfg = preferenceService_->GetConfig();
                               cfg.pressureUnit = "kPa";
                               preferenceService_->SetConfig(cfg);
                               preferenceService_->SaveConfig();
                               ExitSubmenu();
                           }},
                          {"Back", [this]() { ExitSubmenu(); }}};
        }
        break;

        case MenuState::TempUnitSubmenu:
        {
            title = "Temperature Unit";
            menuItems_ = {{"C",
                           [this]()
                           {
                               Configs cfg = preferenceService_->GetConfig();
                               cfg.tempUnit = "C";
                               preferenceService_->SetConfig(cfg);
                               preferenceService_->SaveConfig();
                               ExitSubmenu();
                           }},
                          {"F",
                           [this]()
                           {
                               Configs cfg = preferenceService_->GetConfig();
                               cfg.tempUnit = "F";
                               preferenceService_->SetConfig(cfg);
                               preferenceService_->SaveConfig();
                               ExitSubmenu();
                           }},
                          {"Back", [this]() { ExitSubmenu(); }}};
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