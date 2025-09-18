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
    

    // Reset menu to first item
    currentMenuIndex_ = 0;

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

    // Build dynamic menu from registered configuration sections
    BuildDynamicMenus();
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

// ========== IActionService Interface Implementation ==========

static void ConfigPanelShortPress(void* panelContext)
{
    log_v("ConfigPanelShortPress() called");
    auto* panel = static_cast<ConfigPanel*>(panelContext);

    if (panel) {
        panel->HandleShortPress();
    }
}

static void ConfigPanelLongPress(void* panelContext)
{
    log_v("ConfigPanelLongPress() called");
    auto* panel = static_cast<ConfigPanel*>(panelContext);

    if (panel) {
        panel->HandleLongPress();
    }
}

/**
 * @brief Gets the short press callback function for this panel
 * @return Function pointer to the short press handler
 */
void (*ConfigPanel::GetShortPressFunction())(void* panelContext)
{
    return ConfigPanelShortPress;
}

/**
 * @brief Gets the long press callback function for this panel
 * @return Function pointer to the long press handler
 */
void (*ConfigPanel::GetLongPressFunction())(void* panelContext)
{
    return ConfigPanelLongPress;
}

/**
 * @brief Gets the panel context for button callbacks
 * @return Pointer to this panel instance
 */
void* ConfigPanel::GetPanelContext()
{
    return this;
}

// ========== Public Action Handlers ==========

void ConfigPanel::HandleShortPress()
{
    log_v("HandleShortPress() called");

    if (menuItems_.empty()) {
        log_w("No menu items available");
        return;
    }

    // Cycle to next menu item
    currentMenuIndex_ = (currentMenuIndex_ + 1) % menuItems_.size();

    if (componentInitialized_) {
        configComponent_.SetCurrentIndex(currentMenuIndex_);
    }

    log_i("Menu index: %zu/%zu", currentMenuIndex_, menuItems_.size());
}

void ConfigPanel::HandleLongPress()
{
    log_v("HandleLongPress() called");

    if (menuItems_.empty()) {
        log_w("No menu items available");
        return;
    }

    if (currentMenuIndex_ >= menuItems_.size()) {
        log_w("Invalid menu index: %zu", currentMenuIndex_);
        return;
    }

    const auto& item = menuItems_[currentMenuIndex_];
    log_i("Executing option: %s", item.label.c_str());

    // Handle different action types for dynamic configuration
    if (item.actionType == "enter_section") {
        // Navigate to section menu
        currentSectionName_ = item.actionParam;
        BuildSectionMenu(currentSectionName_);
    } else if (item.actionType == "toggle_boolean") {
        // Handle boolean toggle
        auto [sectionName, itemKey] = ParseConfigKey(item.actionParam);
        if (auto section = preferenceService_->GetConfigSection(sectionName)) {
            for (const auto& configItem : section->items) {
                if (configItem.key == itemKey) {
                    ShowBooleanToggle(item.actionParam, configItem);
                    break;
                }
            }
        }
    } else if (item.actionType == "show_enum_selector") {
        // Handle enum selection
        auto [sectionName, itemKey] = ParseConfigKey(item.actionParam);
        if (auto section = preferenceService_->GetConfigSection(sectionName)) {
            for (const auto& configItem : section->items) {
                if (configItem.key == itemKey) {
                    ShowEnumSelector(item.actionParam, configItem);
                    break;
                }
            }
        }
    } else if (item.actionType == "set_config_value") {
        // Handle direct value setting
        size_t colonPos = item.actionParam.find(':');
        if (colonPos != std::string::npos) {
            std::string fullKey = item.actionParam.substr(0, colonPos);
            std::string value = item.actionParam.substr(colonPos + 1);

            auto [sectionName, itemKey] = ParseConfigKey(fullKey);
            if (auto section = preferenceService_->GetConfigSection(sectionName)) {
                for (const auto& configItem : section->items) {
                    if (configItem.key == itemKey) {
                        // Update the configuration based on type
                        switch (configItem.type) {
                            case Config::ConfigValueType::String:
                                preferenceService_->UpdateConfig(fullKey, value);
                                break;
                            case Config::ConfigValueType::Integer:
                                preferenceService_->UpdateConfig(fullKey, std::stoi(value));
                                break;
                            case Config::ConfigValueType::Float:
                                preferenceService_->UpdateConfig(fullKey, std::stof(value));
                                break;
                            case Config::ConfigValueType::Boolean:
                                preferenceService_->UpdateConfig(fullKey, value == "true");
                                break;
                            case Config::ConfigValueType::Enum:
                                preferenceService_->UpdateConfig(fullKey, value);
                                break;
                        }

                        // Go back to section menu
                        BuildSectionMenu(sectionName);
                        log_i("Updated %s to %s", fullKey.c_str(), value.c_str());
                        break;
                    }
                }
            }
        }
    } else if (item.actionType == "back") {
        // Go back to main menu
        BuildDynamicMenus();
    } else if (item.actionType == "panel_exit") {
        // Exit config panel
        if (panelService_) {
            const char *restorationPanel = panelService_->GetRestorationPanel();
            panelService_->CreateAndLoadPanel(restorationPanel, true);
        }
    }
}

// ========== Dynamic Configuration Implementation ==========

void ConfigPanel::BuildDynamicMenus()
{
    log_v("BuildDynamicMenus() called");

    if (!preferenceService_) {
        log_e("Cannot build dynamic menus - preference service not available");
        return;
    }

    // Get all registered sections
    auto sectionNames = preferenceService_->GetRegisteredSectionNames();

    // Build main menu dynamically
    menuItems_.clear();

    // Add sections as menu items
    for (const auto& sectionName : sectionNames) {
        if (auto section = preferenceService_->GetConfigSection(sectionName)) {
            ConfigComponent::MenuItem item;
            item.label = section->displayName;
            item.actionType = "enter_section";
            item.actionParam = sectionName;
            menuItems_.push_back(item);
        }
    }

    // Add exit option
    menuItems_.push_back({"Exit", "panel_exit", ""});

    // Update component
    if (componentInitialized_) {
        configComponent_.SetTitle("Configuration");
        configComponent_.SetMenuItems(menuItems_);
        configComponent_.SetCurrentIndex(0);
    }

    currentMenuIndex_ = 0;
    log_i("Built dynamic menu with %zu sections", sectionNames.size());
}

void ConfigPanel::BuildSectionMenu(const std::string& sectionName)
{
    log_v("BuildSectionMenu() called for section: %s", sectionName.c_str());

    auto section = preferenceService_->GetConfigSection(sectionName);
    if (!section) {
        log_e("Section not found: %s", sectionName.c_str());
        return;
    }

    menuItems_.clear();

    // Add items from the section
    for (const auto& item : section->items) {
        ConfigComponent::MenuItem menuItem;
        menuItem.label = FormatItemLabel(item);

        // Create action based on item type
        std::string fullKey = sectionName + "." + item.key;
        switch (item.type) {
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
        menuItem.actionParam = fullKey;
        menuItems_.push_back(menuItem);
    }

    // Add back option
    menuItems_.push_back({"Back", "back", ""});

    // Update component
    if (componentInitialized_) {
        configComponent_.SetTitle(section->displayName);
        configComponent_.SetMenuItems(menuItems_);
        configComponent_.SetCurrentIndex(0);
    }

    currentMenuIndex_ = 0;
}

std::string ConfigPanel::FormatItemLabel(const Config::ConfigItem& item) const
{
    std::string valueStr = Config::ConfigValueHelper::ToString(item.value);
    return item.displayName + ": " + valueStr;
}

void ConfigPanel::ShowEnumSelector(const std::string& fullKey, const Config::ConfigItem& item)
{
    log_i("ShowEnumSelector() for key: %s", fullKey.c_str());

    auto options = ParseOptions(item.metadata.constraints);
    if (options.empty()) {
        log_w("No options available for enum: %s", fullKey.c_str());
        return;
    }

    menuItems_.clear();

    // Add each option as a menu item
    for (const auto& option : options) {
        ConfigComponent::MenuItem menuItem;
        menuItem.label = option;
        menuItem.actionType = "set_config_value";
        menuItem.actionParam = fullKey + ":" + option;
        menuItems_.push_back(menuItem);
    }

    // Add back option
    menuItems_.push_back({"Back", "back", ""});

    // Update component
    if (componentInitialized_) {
        configComponent_.SetTitle(item.displayName);
        configComponent_.SetMenuItems(menuItems_);
        configComponent_.SetCurrentIndex(0);
    }

    currentMenuIndex_ = 0;
}

void ConfigPanel::ShowNumericEditor(const std::string& fullKey, const Config::ConfigItem& item)
{
    log_i("ShowNumericEditor() for key: %s", fullKey.c_str());
    // For now, just show current value - full numeric editing would require input handling
    log_i("Current value: %s", Config::ConfigValueHelper::ToString(item.value).c_str());
}

void ConfigPanel::ShowBooleanToggle(const std::string& fullKey, const Config::ConfigItem& item)
{
    log_i("ShowBooleanToggle() for key: %s", fullKey.c_str());

    if (auto currentValue = Config::ConfigValueHelper::GetValue<bool>(item.value)) {
        bool newValue = !(*currentValue);
        preferenceService_->UpdateConfig(fullKey, newValue);

        // Refresh the section menu to show updated value
        auto [sectionName, itemKey] = ParseConfigKey(fullKey);
        BuildSectionMenu(sectionName);

        log_i("Toggled %s from %s to %s", fullKey.c_str(),
              *currentValue ? "true" : "false", newValue ? "true" : "false");
    }
}

std::vector<std::string> ConfigPanel::ParseOptions(const std::string& constraints) const
{
    std::vector<std::string> options;
    if (constraints.empty()) return options;

    std::stringstream ss(constraints);
    std::string option;

    while (std::getline(ss, option, ',')) {
        // Trim whitespace
        option.erase(0, option.find_first_not_of(" \t"));
        option.erase(option.find_last_not_of(" \t") + 1);
        if (!option.empty()) {
            options.push_back(option);
        }
    }

    return options;
}

std::pair<std::string, std::string> ConfigPanel::ParseConfigKey(const std::string& fullKey) const
{
    size_t dotPos = fullKey.find('.');
    if (dotPos == std::string::npos) {
        return {"", fullKey};
    }
    return {fullKey.substr(0, dotPos), fullKey.substr(dotPos + 1)};
}
