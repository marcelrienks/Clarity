#pragma once // preventing duplicate definitions, alternative to the traditional include guards

#include "interfaces/i_panel.h"
#include "interfaces/i_device.h"
#include "panels/splash_panel.h"
#include "panels/demo_panel.h"
#include "panels/oil_panel.h"
#include "utilities/ticker.h"

#include <string>
#include <functional>
#include <memory>
#include <list>
#include <vector>
#include <map>

// NOTE: Define more panel types here as they are added

class PanelManager
{
public:
    static PanelManager &get_instance();

    void init(const char *panel_name);
    void load_panel(const char *panel_name, std::function<void()> completion_callback = nullptr);
    void refresh_panel();

    // Register a panel type with the factory
    template<typename T> // Note the implementation of a template type must exist in header
    void register_panel(const char *panel_name) {
        _registered_panels[panel_name] = []() -> std::shared_ptr<IPanel> { 
            return std::make_shared<T>(); 
        };
    }

private:
    IPanel *_panel = nullptr;
    bool _is_panel_locked = false; // this allows the panel to be locked during loading from show_panel() or change from update_current_panel()

    ~PanelManager();
    void completion_callback();
    std::shared_ptr<IPanel> create_panel(const char *panel_name);
    std::map<std::string, std::function<std::shared_ptr<IPanel>()>> _registered_panels; // Map of panel type names to creator functions for each of those names
};