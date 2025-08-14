#pragma once

#include "config/config_menu_item.h"
#include "interfaces/i_preference_service.h"
#include <functional>
#include <map>
#include <memory>
#include <vector>

/**
 * @class ConfigMenuLoader
 * @brief Loads and manages dynamic configuration menus from JSON
 *
 * @details This class handles loading menu definitions from JSON files,
 * managing the menu hierarchy, and providing an interface for the ConfigPanel
 * to navigate and interact with the dynamic menu structure.
 *
 * @design_pattern Factory pattern for creating menu items from JSON
 * @storage JSON files can be stored in SPIFFS/LittleFS or embedded
 */
class ConfigMenuLoader
{
  public:
    // Constructor
    ConfigMenuLoader(IPreferenceService *prefService);

    // Loading methods
    bool loadFromFile(const char *filepath);
    bool loadFromString(const char *jsonString);
    bool loadFromEmbedded(); // Load from embedded default menu

    // Menu access
    std::vector<std::shared_ptr<ConfigMenuItem>> getRootMenu() const;
    std::shared_ptr<ConfigMenuItem> findMenuItem(const std::string &id) const;

    // Action registration
    void registerAction(const std::string &actionId, std::function<void()> handler);
    void executeAction(const std::string &actionId);

    // Panel registration (for dynamic panel list)
    void registerConfigurablePanel(const std::string &panelName, const std::string &displayName);
    std::vector<std::string> getConfigurablePanels() const;
    std::vector<std::string> getConfigurablePanelLabels() const;

    // Preference binding helpers
    std::string getPreferenceValue(const std::string &key) const;
    void setPreferenceValue(const std::string &key, const std::string &value);

    // Menu state helpers
    bool isItemVisible(const ConfigMenuItem &item) const;

  private:
    IPreferenceService *preferenceService_;
    std::vector<std::shared_ptr<ConfigMenuItem>> rootMenu_;
    std::map<std::string, std::function<void()>> actionHandlers_;
    std::map<std::string, std::string> configurablePanels_; // name -> displayName

    // Default embedded menu JSON
    static const char *DEFAULT_MENU_JSON;

    // Helper methods
    void updateDynamicChoices();
    std::shared_ptr<ConfigMenuItem> findMenuItemRecursive(const std::vector<std::shared_ptr<ConfigMenuItem>> &items,
                                                          const std::string &id) const;
};