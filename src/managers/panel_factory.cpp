#include "managers/panel_factory.h"

PanelFactory& PanelFactory::get_instance() {
    static PanelFactory instance;
    return instance;
}

std::shared_ptr<IPanel> PanelFactory::create_panel(const std::string& typeName, PanelIteration iteration) {
    auto it = _creators.find(typeName);
    if (it != _creators.end())
        return it->second(iteration);

    return nullptr;
}

bool PanelFactory::is_panel_type_registered(const std::string& typeName) {
    return _creators.find(typeName) != _creators.end();
}