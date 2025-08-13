#include "factories/component_factory.h"
#include "managers/error_manager.h"
#include <algorithm>

#ifdef CLARITY_DEBUG
    #include "esp32-hal-log.h"
    #define LOG_TAG "ComponentFactory"
#else
    #define log_d(...)
    #define log_e(...)
#endif

// Static member initialization
std::unordered_map<std::string, ComponentFactory::ComponentCreator> ComponentFactory::componentCreators_;

/// @brief Register a component creator with the factory
void ComponentFactory::RegisterComponent(const std::string& name, ComponentCreator creator)
{
    if (name.empty())
    {
        log_e("Cannot register component with empty name");
        return;
    }
    
    if (!creator)
    {
        log_e("Cannot register null creator for component: %s", name.c_str());
        return;
    }
    
    componentCreators_[name] = creator;
    log_d("Registered component type: %s", name.c_str());
}

/// @brief Create a component by name
std::unique_ptr<IComponent> ComponentFactory::CreateComponent(const std::string& name, 
                                                              IStyleService* styleService)
{
    auto it = componentCreators_.find(name);
    if (it == componentCreators_.end())
    {
        log_e("Component type not found: %s", name.c_str());
        ErrorManager::Instance().ReportError(ErrorLevel::ERROR, "ComponentFactory",
                                           "Unknown component type: " + name);
        return nullptr;
    }
    
    if (!styleService)
    {
        log_e("Cannot create component '%s' with null style service", name.c_str());
        ErrorManager::Instance().ReportError(ErrorLevel::ERROR, "ComponentFactory",
                                           "Null style service for component: " + name);
        return nullptr;
    }
    
    try
    {
        auto component = it->second(styleService);
        if (!component)
        {
            log_e("Component creator returned null for type: %s", name.c_str());
            ErrorManager::Instance().ReportError(ErrorLevel::ERROR, "ComponentFactory",
                                               "Failed to create component: " + name);
            return nullptr;
        }
        
        log_d("Successfully created component: %s", name.c_str());
        return component;
    }
    catch (const std::exception& e)
    {
        log_e("Exception creating component '%s': %s", name.c_str(), e.what());
        ErrorManager::Instance().ReportError(ErrorLevel::ERROR, "ComponentFactory",
                                           "Exception creating " + name + ": " + e.what());
        return nullptr;
    }
}

/// @brief Check if a component type is registered
bool ComponentFactory::IsComponentRegistered(const std::string& name)
{
    return componentCreators_.find(name) != componentCreators_.end();
}

/// @brief Clear all registered component types
void ComponentFactory::ClearRegistrations()
{
    componentCreators_.clear();
    log_d("Cleared all component registrations");
}

/// @brief Get list of all registered component types
std::vector<std::string> ComponentFactory::GetRegisteredComponents()
{
    std::vector<std::string> names;
    names.reserve(componentCreators_.size());
    
    for (const auto& pair : componentCreators_)
    {
        names.push_back(pair.first);
    }
    
    // Sort for consistent ordering
    std::sort(names.begin(), names.end());
    return names;
}