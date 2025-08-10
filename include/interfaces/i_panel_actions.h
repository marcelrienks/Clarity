#pragma once

#include <functional>

/**
 * @interface IPanelActions
 * @brief Interface for panel switching and management functions that panels can use in their actions
 * 
 * @details This interface provides the functions that panels need to create Actions
 * that can switch panels, change themes, or perform other system-level operations.
 * By using this interface, panels can get the necessary functions without depending
 * on concrete manager implementations.
 */
class IPanelActions
{
public:
    virtual ~IPanelActions() = default;
    
    /// @brief Get function for panel switching that panels can use in their actions
    /// @return Function that takes panel name and switches to that panel
    virtual std::function<void(const char*)> GetPanelSwitchFunction() = 0;
    
    /// @brief Get function for theme switching that panels can use in their actions
    /// @return Function that takes theme name and switches to that theme
    virtual std::function<void(const char*)> GetThemeSwitchFunction() = 0;
};