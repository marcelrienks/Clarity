#include "panels/config_panel.h"
#include "managers/error_manager.h"
#include "managers/style_manager.h"
#include "utilities/types.h"
#include "utilities/logging.h"
#include <Arduino.h>
#include <algorithm>
#include <cstring>
#include <sstream>
#include <cctype>

/**
 * @brief Constructs configuration panel with required service dependencies
 * @param gpio GPIO provider for hardware interaction
 * @param display Display provider for screen management
 * @param styleService Style service for theme management
 *
 * Creates configuration panel with stack-allocated configuration component.
 * Initializes menu system for automotive settings management including
 * theme selection, calibration, and system configuration options.
 */
ConfigPanel::ConfigPanel(IGpioProvider *gpio, IDisplayProvider *display, IStyleService *styleService)
    : gpioProvider_(gpio), displayProvider_(display), styleService_(styleService), panelService_(nullptr),
      currentMenuIndex_(0), configComponent_(), componentInitialized_(false)
{
    log_v("ConfigPanel constructor called");
}

/**
 * @brief Destructor cleans up configuration panel resources
 *
 * Safely deletes LVGL screen objects and cleans up menu state.
 * Stack-allocated component is automatically destroyed. Ensures
 * proper cleanup when exiting configuration interface.
 */
ConfigPanel::~ConfigPanel()
{
    log_v("~ConfigPanel() destructor called");

    // Component is now stack-allocated and will be automatically destroyed

    if (screen_)
    {
        lv_obj_delete(screen_);
        screen_ = nullptr;
    }
}

// Core Functionality Methods

/**
 * @brief Initializes configuration panel UI structure and menu system
 *
 * Creates LVGL screen, validates display provider, sets up component location,
 * and initializes configuration menu structure. Builds menu hierarchy for
 * automotive system configuration including themes, calibration, and settings.
 */
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

/**
 * @brief Loads configuration panel UI and displays main menu
 *
 * Renders configuration component, sets up main menu items, applies current
 * theme styling, and loads the LVGL screen. Sets up button action handlers
 * for menu navigation and selection. Critical for configuration interface.
 */
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
        const std::string& theme = styleService_->GetCurrentTheme();
        if (theme == "Night") {
            lv_obj_set_style_bg_color(screen_, lv_color_hex(0x1A0000), LV_PART_MAIN); // Very dark red
            lv_obj_set_style_bg_opa(screen_, LV_OPA_COVER, LV_PART_MAIN);
        }
    }

    // Initialize the config component with the screen
    if (!screen_)
        return;

    // Mark component as initialized
    componentInitialized_ = true;

    configComponent_.Init(screen_);
    configComponent_.SetStyleService(styleService_);

    // Set initial menu items
    UpdateMenuItemsWithCurrentValues();
    configComponent_.SetMenuItems(menuItems_);
    configComponent_.SetCurrentIndex(currentMenuIndex_);

    // Register the screen loaded event callback
    lv_obj_add_event_cb(screen_, ConfigPanel::ShowPanelCompletionCallback, LV_EVENT_SCREEN_LOADED, this);
    
    lv_screen_load(screen_);
    
    log_t("ConfigPanel loaded successfully");
}

/**
 * @brief Updates configuration panel display with current settings
 *
 * Monitors for theme changes and updates component styling accordingly.
 * Configuration panels are primarily static with updates driven by
 * user interactions rather than continuous data changes.
 */
void ConfigPanel::Update()
{
    log_v("Update() called");

    // Config panel is static - no updates needed, but must reset UI state to IDLE
    if (panelService_) {
        panelService_->SetUiState(UIState::IDLE);
    }
}

// Private methods

/**
 * @brief Executes the currently selected configuration menu option
 *
 * Processes the selected menu item based on its action type, handling
 * panel navigation, submenu entry, and configuration changes. Core menu
 * interaction logic for automotive configuration interface.
 */
void ConfigPanel::ExecuteCurrentOption()
{
    log_v("ExecuteCurrentOption() called");

    if (currentMenuIndex_ >= menuItems_.size())
    {
        log_w("Menu index %zu out of bounds (size: %zu)", currentMenuIndex_, menuItems_.size());
        return;
    }
    
    const auto& item = menuItems_[currentMenuIndex_];
    log_i("Executing option at index %zu: '%s' (type: %s, param: %s)", 
          currentMenuIndex_, item.label.c_str(), item.actionType.c_str(), item.actionParam.c_str());
    
    if (item.actionType.empty())
    {
        log_w("Action type is empty for menu item '%s'", item.label.c_str());
        return;
    }
    
    // Handle panel actions directly in the panel
    if (item.actionType == "panel_exit")
    {
        // Exit config panel and return to restoration panel
        log_i("Exiting config panel - returning to restoration panel");
        log_t("ConfigPanel: Executing exit function");
        if (panelService_)
        {
            const char *restorationPanel = panelService_->GetRestorationPanel();
            panelService_->CreateAndLoadPanel(restorationPanel, true);
        }
        return;
    }
    else if (item.actionType == "submenu")
    {
        // Enter the appropriate submenu based on the parameter
        if (item.actionParam == "PanelSubmenu")
        {
            log_i("Entering Panel submenu");
            EnterSubmenu(MenuState::PanelSubmenu);
        }
        else if (item.actionParam == "ThemeSubmenu")
        {
            log_i("Entering Theme submenu");
            log_t("ConfigPanel: Entering theme submenu");
            EnterSubmenu(MenuState::ThemeSubmenu);
        }
        else if (item.actionParam == "UpdateRateSubmenu")
        {
            log_i("Entering Update Rate submenu");
            EnterSubmenu(MenuState::UpdateRateSubmenu);
        }
        else if (item.actionParam == "SplashSubmenu")
        {
            log_i("Entering Splash submenu");
            EnterSubmenu(MenuState::SplashSubmenu);
        }
        else if (item.actionParam == "SplashDurationSubmenu")
        {
            log_i("Entering Splash Duration submenu");
            EnterSubmenu(MenuState::SplashDurationSubmenu);
        }
        else if (item.actionParam == "PressureUnitSubmenu")
        {
            log_i("Entering Pressure Unit submenu");
            EnterSubmenu(MenuState::PressureUnitSubmenu);
        }
        else if (item.actionParam == "TempUnitSubmenu")
        {
            log_i("Entering Temperature Unit submenu");
            EnterSubmenu(MenuState::TempUnitSubmenu);
        }
        else if (item.actionParam == "CalibrationSubmenu")
        {
            log_i("Entering Calibration submenu");
            EnterSubmenu(MenuState::CalibrationSubmenu);
        }
        else if (item.actionParam == "PressureOffsetSubmenu")
        {
            log_i("Entering Pressure Offset submenu");
            EnterSubmenu(MenuState::PressureOffsetSubmenu);
        }
        else if (item.actionParam == "PressureScaleSubmenu")
        {
            log_i("Entering Pressure Scale submenu");
            EnterSubmenu(MenuState::PressureScaleSubmenu);
        }
        else if (item.actionParam == "TempOffsetSubmenu")
        {
            log_i("Entering Temperature Offset submenu");
            EnterSubmenu(MenuState::TempOffsetSubmenu);
        }
        else if (item.actionParam == "TempScaleSubmenu")
        {
            log_i("Entering Temperature Scale submenu");
            EnterSubmenu(MenuState::TempScaleSubmenu);
        }
        else
        {
            log_w("Unknown submenu: %s", item.actionParam.c_str());
        }
    }
    else if (item.actionType == "submenu_back")
    {
        // Return to main menu from submenu
        log_i("Returning to main menu from submenu");
        ExitSubmenu();
    }
    else if (item.actionType == "theme_set")
    {
        // Set the theme immediately
        log_i("Setting theme to: %s", item.actionParam.c_str());
        if (preferenceService_)
        {
            preferenceService_->SetPreference("system.theme", item.actionParam);
            log_i("Theme saved to preferences: %s", item.actionParam.c_str());

            // Apply the theme immediately to the current screen
            if (styleService_)
            {
                log_t("ConfigPanel: Applying night theme setting");
                styleService_->SetTheme(item.actionParam.c_str());
                styleService_->ApplyThemeToScreen(screen_);
                log_i("Theme applied to current config panel");
            }

            // Return to main menu after setting
            ExitSubmenu();
        }
    }
    else if (item.actionType == "config_set")
    {
        // Parse the parameter (format: "key:value")
        size_t colonPos = item.actionParam.find(':');
        if (colonPos != std::string::npos)
        {
            std::string key = item.actionParam.substr(0, colonPos);
            std::string value = item.actionParam.substr(colonPos + 1);
            log_i("Setting config %s to %s", key.c_str(), value.c_str());

            // Use dynamic preference system
            if (preferenceService_)
            {
                // Map old config keys to new preference keys
                std::string prefKey;
                if (key == "panelName") prefKey = "system.panel_name";
                else if (key == "updateRate") prefKey = "system.update_rate";
                else if (key == "showSplash") prefKey = "system.show_splash";
                else if (key == "splashDuration") prefKey = "system.splash_duration";
                else if (key == "pressureUnit") prefKey = "oil_pressure.unit";
                else if (key == "tempUnit") prefKey = "oil_temperature.unit";
                else if (key == "pressureOffset") prefKey = "oil_pressure.offset";
                else if (key == "pressureScale") prefKey = "oil_pressure.scale";
                else if (key == "tempOffset") prefKey = "oil_temperature.offset";
                else if (key == "tempScale") prefKey = "oil_temperature.scale";
                else {
                    log_w("Unknown config key: %s", key.c_str());
                    return;
                }

                preferenceService_->SetPreference(prefKey, value);
                log_i("Config saved to preferences: %s = %s", prefKey.c_str(), value.c_str());

                // Return to main menu after setting
                ExitSubmenu();
            }
        }
    }
    // ========== Dynamic Configuration Actions ==========
    else if (item.actionType == "enter_section")
    {
        // Enter a configuration section
        log_i("Entering configuration section: %s", item.actionParam.c_str());
        currentMenuState_ = MenuState::MainMenu; // Use MainMenu as generic submenu state
        BuildSectionMenu(item.actionParam);
    }
    else if (item.actionType == "show_enum_selector")
    {
        // Show enum value selector
        log_i("Showing enum selector for: %s", item.actionParam.c_str());

        if (preferenceService_)
        {
            auto [sectionName, itemKey] = ParseConfigKey(item.actionParam);
            auto sectionOpt = preferenceService_->GetConfigSection(sectionName);
            if (sectionOpt)
            {
                auto itemPtr = sectionOpt->FindItem(itemKey);
                if (itemPtr)
                {
                    ShowEnumSelector(item.actionParam, *itemPtr);
                }
            }
        }
    }
    else if (item.actionType == "show_numeric_editor")
    {
        // Show numeric value editor
        log_i("Showing numeric editor for: %s", item.actionParam.c_str());

        if (preferenceService_)
        {
            auto [sectionName, itemKey] = ParseConfigKey(item.actionParam);
            auto sectionOpt = preferenceService_->GetConfigSection(sectionName);
            if (sectionOpt)
            {
                auto itemPtr = sectionOpt->FindItem(itemKey);
                if (itemPtr)
                {
                    ShowNumericEditor(item.actionParam, *itemPtr);
                }
            }
        }
    }
    else if (item.actionType == "toggle_boolean")
    {
        // Toggle boolean value directly or show selector
        log_i("Toggling boolean for: %s", item.actionParam.c_str());

        if (preferenceService_)
        {
            auto [sectionName, itemKey] = ParseConfigKey(item.actionParam);
            auto sectionOpt = preferenceService_->GetConfigSection(sectionName);
            if (sectionOpt)
            {
                auto itemPtr = sectionOpt->FindItem(itemKey);
                if (itemPtr)
                {
                    ShowBooleanToggle(item.actionParam, *itemPtr);
                }
            }
        }
    }
    else if (item.actionType == "show_string_editor")
    {
        // String editing not implemented yet - show placeholder
        log_i("String editor not yet implemented for: %s", item.actionParam.c_str());
    }
    else
    {
        // Use the component's action execution method for other actions
        if (componentInitialized_)
        {
            configComponent_.ExecuteAction(item.actionType, item.actionParam);
        }
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

    // Set UI state to IDLE after static panel loads so triggers can be evaluated again
    if (panel->panelService_)
    {
        panel->panelService_->SetUiState(UIState::IDLE);
    }
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
            if (componentInitialized_)
            {
                configComponent_.SetCurrentIndex(currentMenuIndex_);

                // Update hint text based on current menu item
                if (menuItems_[currentMenuIndex_].label == "Exit")
                {
                    configComponent_.SetHintText("Short: Next | Long: Press to exit");
                }
                else
                {
                    configComponent_.SetHintText("Short: Next | Long: Select");
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

    // Check if this is a dynamic config service
    if (preferenceService_) {
        // Since we can't do proper casting without RTTI and the interfaces are separate,
        // we'll disable dynamic config for now and use legacy mode
        preferenceService_ = nullptr;
        useDynamicConfig_ = false;

        log_i("ConfigPanel using %s configuration system (dynamic mode disabled for Phase 3)",
              useDynamicConfig_ ? "dynamic" : "legacy");
    }

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

    // For Phase 3, we'll use legacy menus but the infrastructure is ready
    // for future dynamic menu implementation
    log_i("Using legacy configuration menus (dynamic UI infrastructure ready)");
    UpdateMenuItemsWithCurrentValues();
}

void ConfigPanel::UpdateMenuItemsWithCurrentValues()
{
    log_v("UpdateMenuItemsWithCurrentValues() called");

    if (!preferenceService_)
        return;

    // Get individual preferences with defaults
    std::string panelName = preferenceService_->GetPreference("system.panel_name");
    if (panelName.empty()) panelName = PanelNames::OIL;

    std::string theme = preferenceService_->GetPreference("system.theme");
    if (theme.empty()) theme = "day";

    std::string showSplashStr = preferenceService_->GetPreference("system.show_splash");
    bool showSplash = showSplashStr == "true" || showSplashStr.empty(); // default true

    std::string splashDurationStr = preferenceService_->GetPreference("system.splash_duration");
    int splashDuration = splashDurationStr.empty() ? 1500 : std::stoi(splashDurationStr);

    std::string updateRateStr = preferenceService_->GetPreference("system.update_rate");
    int updateRate = updateRateStr.empty() ? 500 : std::stoi(updateRateStr);

    std::string pressureUnit = preferenceService_->GetPreference("oil_pressure.unit");
    if (pressureUnit.empty()) pressureUnit = "Bar";

    std::string tempUnit = preferenceService_->GetPreference("oil_temperature.unit");
    if (tempUnit.empty()) tempUnit = "C";

    // Format panel name for display (remove "Panel" suffix)
    std::string panelDisplay = panelName;
    if (panelDisplay.find("Panel") != std::string::npos)
    {
        panelDisplay = panelDisplay.substr(0, panelDisplay.find("Panel"));
    }

    // Use static buffers to avoid dynamic allocations in menu creation
    static char panelItem[64], themeItem[64], splashItem[64], splashTimeItem[64], rateItem[64], pressureItem[64], tempItem[64];

    snprintf(panelItem, sizeof(panelItem), "Panel: %s", panelDisplay.c_str());
    snprintf(themeItem, sizeof(themeItem), "Theme: %s", theme.c_str());
    snprintf(splashItem, sizeof(splashItem), "Splash: %s", showSplash ? "On" : "Off");
    snprintf(splashTimeItem, sizeof(splashTimeItem), "Splash T: %dms", splashDuration);
    snprintf(rateItem, sizeof(rateItem), "Rate: %dms", updateRate);
    snprintf(pressureItem, sizeof(pressureItem), "Press: %s", pressureUnit.c_str());
    snprintf(tempItem, sizeof(tempItem), "Temp: %s", tempUnit.c_str());

    menuItems_ = {
        {panelItem, "submenu", "PanelSubmenu"},
        {themeItem, "submenu", "ThemeSubmenu"},
        {splashItem, "submenu", "SplashSubmenu"},
        {splashTimeItem, "submenu", "SplashDurationSubmenu"},
        {rateItem, "submenu", "UpdateRateSubmenu"},
        {pressureItem, "submenu", "PressureUnitSubmenu"},
        {tempItem, "submenu", "TempUnitSubmenu"},
        {"Calibration", "submenu", "CalibrationSubmenu"},
        {"Exit", "panel_exit", ""}};

    // Update component with new menu items
    if (componentInitialized_)
    {
        configComponent_.SetTitle("Configuration");
        configComponent_.SetMenuItems(menuItems_);
        configComponent_.SetCurrentIndex(currentMenuIndex_);
    }
}

void ConfigPanel::EnterSubmenu(MenuState submenu)
{
    log_v("EnterSubmenu() called");

    currentMenuState_ = submenu;
    currentMenuIndex_ = 0;
    UpdateSubmenuItems();
    
    // Add test log for theme submenu loaded
    if (submenu == MenuState::ThemeSubmenu) {
        log_t("ConfigPanel: Theme submenu loaded");
    }
}

void ConfigPanel::ExitSubmenu()
{
    log_v("ExitSubmenu() called");
    currentMenuState_ = MenuState::MainMenu;
    currentMenuIndex_ = 0;
    UpdateMenuItemsWithCurrentValues();
    
    // Update the screen background based on current theme
    if (styleService_ && screen_)
    {
        styleService_->ApplyThemeToScreen(screen_);
        
        // For night theme, override the screen background to use dark red instead of black
        const std::string& theme = styleService_->GetCurrentTheme();
        if (theme == "Night") {
            lv_obj_set_style_bg_color(screen_, lv_color_hex(0x1A0000), LV_PART_MAIN); // Very dark red
            lv_obj_set_style_bg_opa(screen_, LV_OPA_COVER, LV_PART_MAIN);
        }
    }
    
    // Update the config component's theme colors after returning to main menu
    // This ensures the menu reflects any theme changes made in submenus
    if (componentInitialized_)
    {
        configComponent_.UpdateThemeColors();
        configComponent_.SetMenuItems(menuItems_);
        configComponent_.SetCurrentIndex(currentMenuIndex_);
    }
    
    // Add test log for returning to main config menu
    log_t("ConfigPanel: Returned to main config menu");
}

void ConfigPanel::UpdateSubmenuItems()
{
    log_v("UpdateSubmenuItems() called");

    if (!preferenceService_)
        return;

    // Get individual preferences with defaults
    std::string panelName = preferenceService_->GetPreference("system.panel_name");
    if (panelName.empty()) panelName = PanelNames::OIL;

    std::string theme = preferenceService_->GetPreference("system.theme");
    if (theme.empty()) theme = "day";

    std::string showSplashStr = preferenceService_->GetPreference("system.show_splash");
    bool showSplash = showSplashStr == "true" || showSplashStr.empty(); // default true

    std::string splashDurationStr = preferenceService_->GetPreference("system.splash_duration");
    int splashDuration = splashDurationStr.empty() ? 1500 : std::stoi(splashDurationStr);

    std::string updateRateStr = preferenceService_->GetPreference("system.update_rate");
    int updateRate = updateRateStr.empty() ? 500 : std::stoi(updateRateStr);

    std::string pressureUnit = preferenceService_->GetPreference("oil_pressure.unit");
    if (pressureUnit.empty()) pressureUnit = "Bar";

    std::string tempUnit = preferenceService_->GetPreference("oil_temperature.unit");
    if (tempUnit.empty()) tempUnit = "C";

    std::string pressureOffsetStr = preferenceService_->GetPreference("oil_pressure.offset");
    float pressureOffset = pressureOffsetStr.empty() ? 0.0f : std::stof(pressureOffsetStr);

    std::string pressureScaleStr = preferenceService_->GetPreference("oil_pressure.scale");
    float pressureScale = pressureScaleStr.empty() ? 1.0f : std::stof(pressureScaleStr);

    std::string tempOffsetStr = preferenceService_->GetPreference("oil_temperature.offset");
    float tempOffset = tempOffsetStr.empty() ? 0.0f : std::stof(tempOffsetStr);

    std::string tempScaleStr = preferenceService_->GetPreference("oil_temperature.scale");
    float tempScale = tempScaleStr.empty() ? 1.0f : std::stof(tempScaleStr);

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
            snprintf(offsetStr, sizeof(offsetStr), "%.2f", pressureOffset);
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
            snprintf(scaleStr, sizeof(scaleStr), "%.3f", pressureScale);
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
            snprintf(offsetStr, sizeof(offsetStr), "%.1f", tempOffset);
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
            snprintf(scaleStr, sizeof(scaleStr), "%.3f", tempScale);
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
    if (componentInitialized_)
    {
        configComponent_.SetTitle(title);
        configComponent_.SetMenuItems(menuItems_);
        configComponent_.SetCurrentIndex(currentMenuIndex_);
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

/**
 * @brief Gets the short press callback function for this panel
 * @return Function pointer to the short press handler
 *
 * Returns the static callback function that will be invoked when a short
 * button press is detected while this panel is active. The config panel uses
 * short presses to cycle through menu options and navigate the interface.
 */
void (*ConfigPanel::GetShortPressFunction())(void* panelContext)
{
    return ConfigPanelShortPress;
}

/**
 * @brief Gets the long press callback function for this panel
 * @return Function pointer to the long press handler
 *
 * Returns the static callback function that will be invoked when a long
 * button press is detected while this panel is active. The config panel uses
 * long presses to select menu items and execute configuration actions.
 */
void (*ConfigPanel::GetLongPressFunction())(void* panelContext)
{
    return ConfigPanelLongPress;
}

/**
 * @brief Gets the panel context pointer for callback functions
 * @return Pointer to this panel instance
 *
 * Returns a void pointer to this panel instance that will be passed to
 * the button press callback functions. Enables the static callback functions
 * to access the specific panel instance that should handle the events.
 */
void* ConfigPanel::GetPanelContext()
{
    return this;
}

/**
 * @brief Handles short button press events for menu navigation
 *
 * Processes short button press events by cycling to the next menu item.
 * Works in both main menu and submenu contexts, wrapping back to the first
 * item after reaching the end. Updates the UI component to highlight the
 * newly selected menu item and includes test logging for automation.
 */
void ConfigPanel::HandleShortPress()
{
    if (menuItems_.empty())
    {
        log_w("ConfigPanel: Cannot cycle menu - no menu items");
        return;
    }
    
    // Cycle to next menu item (works for both main menu and submenus)
    currentMenuIndex_ = (currentMenuIndex_ + 1) % menuItems_.size();
    log_i("ConfigPanel: Cycled to menu item %zu: %s (state: %s)", 
          currentMenuIndex_, 
          menuItems_[currentMenuIndex_].label.c_str(),
          currentMenuState_ == MenuState::MainMenu ? "MainMenu" : "Submenu");
    
    // Add test logs for specific navigation events
    if (currentMenuState_ == MenuState::MainMenu && 
        menuItems_[currentMenuIndex_].label.find("Theme:") == 0) {
        log_t("ConfigPanel: Theme Settings option highlighted");
    }
    if (currentMenuState_ == MenuState::ThemeSubmenu && 
        menuItems_[currentMenuIndex_].label == "Night") {
        log_t("ConfigPanel: Night theme option highlighted");
    }
    if (currentMenuState_ == MenuState::MainMenu && 
        menuItems_[currentMenuIndex_].label == "Exit") {
        log_t("ConfigPanel: Exit option highlighted");
    }
    
    // Update the UI
    if (componentInitialized_)
    {
        configComponent_.SetCurrentIndex(currentMenuIndex_);
    }
}

void ConfigPanel::HandleLongPress()
{
    log_i("ConfigPanel: Long press at index %zu (state: %s)", 
          currentMenuIndex_,
          currentMenuState_ == MenuState::MainMenu ? "MainMenu" : "Submenu");
    
    if (currentMenuState_ == MenuState::MainMenu)
    {
        // In main menu: execute the selected option (enter submenu or exit)
        log_i("Main menu: Executing current option");
        ExecuteCurrentOption();
    }
    else
    {
        // In submenu: execute the selection and return to main menu
        log_i("Submenu: Selecting option and returning to main menu");
        
        if (currentMenuIndex_ < menuItems_.size())
        {
            const auto& item = menuItems_[currentMenuIndex_];
            
            // Handle special submenu actions
            if (item.actionType == "submenu_back")
            {
                // Back button - just exit submenu
                log_i("Back button selected - exiting submenu");
                ExitSubmenu();
            }
            else if (item.actionType == "display_only")
            {
                // Display only item - do nothing
                log_i("Display only item - no action");
            }
            else if (item.actionType == "panel_set")
            {
                // Set panel preference
                log_i("Setting panel preference: %s", item.actionParam.c_str());
                if (preferenceService_)
                {
                    preferenceService_->SetPreference("system.panel_name", item.actionParam);
                }
                ExitSubmenu();
            }
            else if (item.actionType == "theme_set")
            {
                // Set theme preference
                log_i("Setting theme preference: %s", item.actionParam.c_str());
                log_t("ConfigPanel: Applying night theme setting");
                if (preferenceService_)
                {
                    preferenceService_->SetPreference("system.theme", item.actionParam);
                }
                ExitSubmenu();
                log_t("ConfigPanel: Returned to main config menu");
            }
            else if (item.actionType == "update_rate_set")
            {
                // Set update rate preference
                log_i("Setting update rate: %s", item.actionParam.c_str());
                if (preferenceService_)
                {
                    preferenceService_->SetPreference("system.update_rate", item.actionParam);
                }
                ExitSubmenu();
            }
            else if (item.actionType == "splash_set")
            {
                // Set splash preference
                log_i("Setting splash preference: %s", item.actionParam.c_str());
                if (preferenceService_)
                {
                    preferenceService_->SetPreference("system.show_splash", item.actionParam);
                }
                ExitSubmenu();
            }
            else if (item.actionType == "splash_duration_set")
            {
                // Set splash duration
                log_i("Setting splash duration: %s", item.actionParam.c_str());
                if (preferenceService_)
                {
                    preferenceService_->SetPreference("system.splash_duration", item.actionParam);
                }
                ExitSubmenu();
            }
            else if (item.actionType == "pressure_unit_set")
            {
                // Set pressure unit preference
                log_i("Setting pressure unit: %s", item.actionParam.c_str());
                if (preferenceService_)
                {
                    preferenceService_->SetPreference("oil_pressure.unit", item.actionParam);
                }
                ExitSubmenu();
            }
            else if (item.actionType == "temp_unit_set")
            {
                // Set temperature unit preference
                log_i("Setting temperature unit: %s", item.actionParam.c_str());
                if (preferenceService_)
                {
                    preferenceService_->SetPreference("oil_temperature.unit", item.actionParam);
                }
                ExitSubmenu();
            }
            else if (item.actionType == "calibration_set")
            {
                // Set calibration value
                log_i("Setting calibration: %s", item.actionParam.c_str());
                size_t colonPos = item.actionParam.find(':');
                if (colonPos != std::string::npos)
                {
                    std::string key = item.actionParam.substr(0, colonPos);
                    float value = std::stof(item.actionParam.substr(colonPos + 1));
                    UpdateCalibration(key, value);
                }
                ExitSubmenu();
            }
            else
            {
                // Unknown action type - just execute through component
                log_w("Unknown action type in submenu: %s", item.actionType.c_str());
                if (componentInitialized_)
                {
                    configComponent_.ExecuteAction(item.actionType, item.actionParam);
                }
            }
        }
    }
}

void ConfigPanel::UpdateCalibration(const std::string& key, float value)
{
    log_v("UpdateCalibration() called for key: %s, value: %.3f", key.c_str(), value);
    
    if (!preferenceService_)
        return;

    std::string prefKey;
    if (key == "pressure_offset")
    {
        prefKey = "oil_pressure.offset";
    }
    else if (key == "pressure_scale")
    {
        prefKey = "oil_pressure.scale";
    }
    else if (key == "temp_offset")
    {
        prefKey = "oil_temperature.offset";
    }
    else if (key == "temp_scale")
    {
        prefKey = "oil_temperature.scale";
    }

    if (!prefKey.empty())
    {
        preferenceService_->SetPreference(prefKey, std::to_string(value));
    }

    // Refresh the submenu to show updated values
    UpdateSubmenuItems();
}

// ========== Dynamic Configuration Methods ==========

void ConfigPanel::BuildDynamicMenus()
{
    log_v("BuildDynamicMenus() called");

    if (!preferenceService_)
    {
        log_w("Cannot build dynamic menus - dynamic config service not available");
        UpdateMenuItemsWithCurrentValues(); // Fallback to legacy
        return;
    }

    // Get all registered sections
    auto sectionNames = preferenceService_->GetRegisteredSectionNames();

    // Build main menu dynamically
    menuItems_.clear();

    // Sort sections by display order if we had that information
    // For now, we'll use the section names as-is
    for (const auto& sectionName : sectionNames)
    {
        auto sectionOpt = preferenceService_->GetConfigSection(sectionName);
        if (sectionOpt)
        {
            ConfigComponent::MenuItem item;
            item.label = sectionOpt->displayName;
            item.actionType = "enter_section";
            item.actionParam = sectionName;
            menuItems_.push_back(item);
        }
    }

    // Add Exit option
    menuItems_.push_back({"Exit", "panel_exit", ""});

    // Update component with new menu items
    if (componentInitialized_)
    {
        configComponent_.SetTitle("Configuration");
        configComponent_.SetMenuItems(menuItems_);
        configComponent_.SetCurrentIndex(currentMenuIndex_);
    }

    log_i("Built dynamic menu with %zu sections", sectionNames.size());
}

void ConfigPanel::BuildSectionMenu(const std::string& sectionName)
{
    log_v("BuildSectionMenu() called for section: %s", sectionName.c_str());

    if (!preferenceService_)
        return;

    auto sectionOpt = preferenceService_->GetConfigSection(sectionName);
    if (!sectionOpt)
    {
        log_w("Section not found: %s", sectionName.c_str());
        return;
    }

    const auto& section = *sectionOpt;

    // Build section-specific menu
    menuItems_.clear();

    for (const auto& item : section.items)
    {
        ConfigComponent::MenuItem menuItem;
        menuItem.label = FormatItemLabel(item);

        // Determine action type based on config item type
        switch (item.type)
        {
            case Config::ConfigValueType::Enum:
                menuItem.actionType = "show_enum_selector";
                break;
            case Config::ConfigValueType::Boolean:
                menuItem.actionType = "toggle_boolean";
                break;
            case Config::ConfigValueType::Integer:
            case Config::ConfigValueType::Float:
                menuItem.actionType = "show_numeric_editor";
                break;
            case Config::ConfigValueType::String:
                menuItem.actionType = "show_string_editor";
                break;
        }

        menuItem.actionParam = sectionName + "." + item.key;
        menuItems_.push_back(menuItem);
    }

    // Add Back option
    menuItems_.push_back({"Back", "submenu_back", ""});

    // Update component
    if (componentInitialized_)
    {
        configComponent_.SetTitle(section.displayName);
        configComponent_.SetMenuItems(menuItems_);
        configComponent_.SetCurrentIndex(0); // Reset to first item
    }
}

std::string ConfigPanel::FormatItemLabel(const Config::ConfigItem& item) const
{
    std::string label = item.displayName + ": ";

    // Format current value based on type
    std::string valueStr = Config::ConfigValueHelper::ToString(item.value);

    // Add unit if available
    if (!item.metadata.unit.empty())
    {
        valueStr += item.metadata.unit;
    }

    return label + valueStr;
}

void ConfigPanel::ShowEnumSelector(const std::string& fullKey, const Config::ConfigItem& item)
{
    log_v("ShowEnumSelector() called for %s", fullKey.c_str());

    if (!preferenceService_)
        return;

    // Parse options from metadata
    auto options = ParseOptions(item.metadata.constraints);
    if (options.empty())
    {
        log_w("No options available for enum %s", fullKey.c_str());
        return;
    }

    // Build enum selector submenu
    menuItems_.clear();

    // Show current value
    std::string currentValue = Config::ConfigValueHelper::ToString(item.value);
    menuItems_.push_back({"Current: " + currentValue, "display_only", ""});

    // Add options
    for (const auto& option : options)
    {
        ConfigComponent::MenuItem menuItem;
        menuItem.label = option;
        menuItem.actionType = "config_set";
        menuItem.actionParam = fullKey + ":" + option;
        menuItems_.push_back(menuItem);
    }

    // Add Back option
    menuItems_.push_back({"Back", "submenu_back", ""});

    // Update component
    if (componentInitialized_)
    {
        configComponent_.SetTitle(item.displayName);
        configComponent_.SetMenuItems(menuItems_);
        configComponent_.SetCurrentIndex(0);
    }
}

void ConfigPanel::ShowNumericEditor(const std::string& fullKey, const Config::ConfigItem& item)
{
    log_v("ShowNumericEditor() called for %s", fullKey.c_str());

    if (!preferenceService_)
        return;

    // Parse options from metadata (could be range or discrete values)
    auto options = ParseOptions(item.metadata.constraints);

    // Build numeric editor submenu
    menuItems_.clear();

    // Show current value
    std::string currentValue = Config::ConfigValueHelper::ToString(item.value);
    if (!item.metadata.unit.empty())
    {
        currentValue += item.metadata.unit;
    }
    menuItems_.push_back({"Current: " + currentValue, "display_only", ""});

    // Add options (either discrete values or common values for ranges)
    for (const auto& option : options)
    {
        ConfigComponent::MenuItem menuItem;
        menuItem.label = option;
        if (!item.metadata.unit.empty())
        {
            menuItem.label += item.metadata.unit;
        }
        menuItem.actionType = "config_set";
        menuItem.actionParam = fullKey + ":" + option;
        menuItems_.push_back(menuItem);
    }

    // Add Back option
    menuItems_.push_back({"Back", "submenu_back", ""});

    // Update component
    if (componentInitialized_)
    {
        configComponent_.SetTitle(item.displayName);
        configComponent_.SetMenuItems(menuItems_);
        configComponent_.SetCurrentIndex(0);
    }
}

void ConfigPanel::ShowBooleanToggle(const std::string& fullKey, const Config::ConfigItem& item)
{
    log_v("ShowBooleanToggle() called for %s", fullKey.c_str());

    if (!preferenceService_)
        return;

    // Get current boolean value
    bool currentValue = false;
    if (auto boolVal = Config::ConfigValueHelper::GetValue<bool>(item.value))
    {
        currentValue = *boolVal;
    }

    // Build boolean toggle submenu
    menuItems_.clear();

    // Show current value
    menuItems_.push_back({"Current: " + std::string(currentValue ? "On" : "Off"), "display_only", ""});

    // Add toggle options
    menuItems_.push_back({"On", "config_set", fullKey + ":true"});
    menuItems_.push_back({"Off", "config_set", fullKey + ":false"});

    // Add Back option
    menuItems_.push_back({"Back", "submenu_back", ""});

    // Update component
    if (componentInitialized_)
    {
        configComponent_.SetTitle(item.displayName);
        configComponent_.SetMenuItems(menuItems_);
        configComponent_.SetCurrentIndex(0);
    }
}

std::vector<std::string> ConfigPanel::ParseOptions(const std::string& constraints) const
{
    std::vector<std::string> options;

    if (constraints.empty())
        return options;

    std::stringstream ss(constraints);
    std::string item;

    while (std::getline(ss, item, ','))
    {
        // Trim whitespace
        item.erase(0, item.find_first_not_of(" \t"));
        item.erase(item.find_last_not_of(" \t") + 1);

        if (!item.empty())
        {
            options.push_back(item);
        }
    }

    return options;
}

/**
 * @brief Parses a full configuration key into section and item components
 * @param fullKey Full configuration key in format "section.item"
 * @return Pair containing section name and item key
 *
 * Splits a configuration key string at the dot separator to extract the
 * section name and item key. Returns empty section if no dot is found.
 * Used by the dynamic configuration system to map keys to their sections.
 */
std::pair<std::string, std::string> ConfigPanel::ParseConfigKey(const std::string& fullKey) const
{
    size_t dotPos = fullKey.find('.');
    if (dotPos == std::string::npos)
    {
        return {"", fullKey};
    }

    return {fullKey.substr(0, dotPos), fullKey.substr(dotPos + 1)};
}