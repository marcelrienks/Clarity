#pragma once  // preventing duplicate definitions, alternative to the traditional include guards

#include "interfaces/i_panel.h"
#include <string>
#include <memory>
#include <map>
#include <functional>

class PanelFactory {
public:
    static PanelFactory& get_instance();
    
    // Register a panel type with the factory
    template<typename T> // Note the implementation of a template type must exist in header
    void register_panel_type(const std::string &type_name) {
        _creators[type_name] = [](PanelIteration iteration) -> std::shared_ptr<IPanel> { 
            return std::make_shared<T>(iteration); 
        };
    }
    
    // Create a panel instance by type name
    std::shared_ptr<IPanel> create_panel(const std::string& type_name, PanelIteration iteration = PanelIteration::Infinite);
    
    // Check if a panel type is registered
    bool is_panel_type_registered(const std::string& type_name);

private:
    PanelFactory() = default;
    ~PanelFactory() = default;
    
    // Map of panel type names to creator functions
    std::map<std::string, std::function<std::shared_ptr<IPanel>(PanelIteration)>> _creators;
};