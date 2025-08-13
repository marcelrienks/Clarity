#include "factories/panel_factory.h"
#include "managers/error_manager.h"
#include <algorithm>

#ifdef CLARITY_DEBUG
    #include "esp32-hal-log.h"
    #define LOG_TAG "PanelFactory"
#else
    #define log_d(...)
    #define log_e(...)
#endif

// Static member initialization
std::unordered_map<std::string, PanelFactory::PanelCreator> PanelFactory::panelCreators_;

/// @brief Register a panel creator with the factory
void PanelFactory::RegisterPanel(const std::string& name, PanelCreator creator)
{
    if (name.empty())
    {
        log_e("Cannot register panel with empty name");
        return;
    }
    
    if (!creator)
    {
        log_e("Cannot register null creator for panel: %s", name.c_str());
        return;
    }
    
    panelCreators_[name] = creator;
    log_d("Registered panel type: %s", name.c_str());
}

/// @brief Create a panel by name
std::unique_ptr<IPanel> PanelFactory::CreatePanel(const std::string& name,
                                                   IGpioProvider* gpioProvider,
                                                   IDisplayProvider* displayProvider,
                                                   IStyleService* styleService)
{
    auto it = panelCreators_.find(name);
    if (it == panelCreators_.end())
    {
        log_e("Panel type not found: %s", name.c_str());
        ErrorManager::Instance().ReportError(ErrorLevel::ERROR, "PanelFactory",
                                           "Unknown panel type: " + name);
        return nullptr;
    }
    
    // Validate required dependencies
    if (!gpioProvider)
    {
        log_e("Cannot create panel '%s' with null GPIO provider", name.c_str());
        ErrorManager::Instance().ReportError(ErrorLevel::ERROR, "PanelFactory",
                                           "Null GPIO provider for panel: " + name);
        return nullptr;
    }
    
    if (!displayProvider)
    {
        log_e("Cannot create panel '%s' with null display provider", name.c_str());
        ErrorManager::Instance().ReportError(ErrorLevel::ERROR, "PanelFactory",
                                           "Null display provider for panel: " + name);
        return nullptr;
    }
    
    if (!styleService)
    {
        log_e("Cannot create panel '%s' with null style service", name.c_str());
        ErrorManager::Instance().ReportError(ErrorLevel::ERROR, "PanelFactory",
                                           "Null style service for panel: " + name);
        return nullptr;
    }
    
    try
    {
        auto panel = it->second(gpioProvider, displayProvider, styleService);
        if (!panel)
        {
            log_e("Panel creator returned null for type: %s", name.c_str());
            ErrorManager::Instance().ReportError(ErrorLevel::ERROR, "PanelFactory",
                                               "Failed to create panel: " + name);
            return nullptr;
        }
        
        log_d("Successfully created panel: %s", name.c_str());
        return panel;
    }
    catch (const std::exception& e)
    {
        log_e("Exception creating panel '%s': %s", name.c_str(), e.what());
        ErrorManager::Instance().ReportError(ErrorLevel::ERROR, "PanelFactory",
                                           "Exception creating " + name + ": " + e.what());
        return nullptr;
    }
}

/// @brief Check if a panel type is registered
bool PanelFactory::IsPanelRegistered(const std::string& name)
{
    return panelCreators_.find(name) != panelCreators_.end();
}

/// @brief Clear all registered panel types
void PanelFactory::ClearRegistrations()
{
    panelCreators_.clear();
    log_d("Cleared all panel registrations");
}

/// @brief Get list of all registered panel types
std::vector<std::string> PanelFactory::GetRegisteredPanels()
{
    std::vector<std::string> names;
    names.reserve(panelCreators_.size());
    
    for (const auto& pair : panelCreators_)
    {
        names.push_back(pair.first);
    }
    
    // Sort for consistent ordering
    std::sort(names.begin(), names.end());
    return names;
}