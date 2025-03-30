#include "handlers/panel_factory.h"

/// @brief Get singleton instance of PanelFactory
/// @return singleton instance of PanelFactory
PanelFactory &PanelFactory::get_instance() {
    static PanelFactory instance;
    return instance;
}

/// @brief Create a panel based on the given type name
/// @param panel_name the type name of the panel to be created
/// @param panel_iteration the panel iteration type to be set on the newly created panel
/// @return Interface type representing the panel
std::shared_ptr<IPanel> PanelFactory::create_panel(const std::string &panel_name, PanelIteration panel_iteration) {
    auto iterator = _panel_creators.find(panel_name);
    if (iterator != _panel_creators.end())
        return iterator->second(panel_iteration);// Return the function stored in the map

    return nullptr;
}

/// @brief Is the given panel type registered in the panel creators map
/// @param panel_name the name of the panel type
/// @return true or false if the panel type is found
bool PanelFactory::is_panel_type_registered(const std::string &panel_name) {
    return _panel_creators.find(panel_name) != _panel_creators.end();
}