#pragma once  // preventing duplicate definitions, alternative to the traditional include guards

#include "interfaces/i_panel.h"

#include <string>
#include <memory>
#include <map>
#include <functional>

class PanelFactory {
public:
    static PanelFactory &get_instance();
    
    // Register a panel type with the factory
    template<typename T> // Note the implementation of a template type must exist in header
    void register_panel(const std::string &panel_name) {
        _panel_creators[panel_name] = [](IDevice *device, PanelIteration panel_iteration) -> std::shared_ptr<IPanel> { 
            return std::make_shared<T>(device, panel_iteration); 
        };
    }
    
    // Create a panel instance by type name
    std::shared_ptr<IPanel> create_panel(IDevice *device, const std::string &panel_name, PanelIteration panel_iteration = PanelIteration::Infinite);
    
    // Check if a panel type is registered
    bool is_panel_type_registered(const std::string &panel_name);

private:
    PanelFactory() = default;
    ~PanelFactory() = default;
    
    // Map of panel type names to creator functions for each of those names
    std::map<std::string, std::function<std::shared_ptr<IPanel>(IDevice *, PanelIteration)>> _panel_creators;
};