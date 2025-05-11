#include "handlers/panel_factory.h"

/// @brief Get singleton instance of PanelFactory
/// @return singleton instance of PanelFactory
PanelFactory &PanelFactory::get_instance() //TODO: consider converting all classes to this method?
{
    static PanelFactory instance; // this ensures that the instance is created only once
    return instance;
}

/// @brief Create a panel based on the given type name
/// @param panel_name the type name of the panel to be created
/// @return Interface type representing the panel
std::shared_ptr<IPanel> PanelFactory::create_panel(IDevice *device, const std::string &panel_name)
{
    auto iterator = _panel_creators.find(panel_name);
    if (iterator != _panel_creators.end())
        return iterator->second(device); // Return the function stored in the map

    return nullptr;
}

/// @brief Is the given panel type registered in the panel creators map
/// @param panel_name the name of the panel type
/// @return true or false if the panel type is found
bool PanelFactory::is_panel_type_registered(const std::string &panel_name)
{
    return _panel_creators.find(panel_name) != _panel_creators.end();
}