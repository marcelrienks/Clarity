#pragma once

// System/Library Includes
#include <memory>
#include <string>
#include <functional>

// Project Includes
#include "interfaces/i_component.h"
#include "interfaces/i_panel.h"
#include "interfaces/i_display_provider.h"
#include "interfaces/i_gpio_provider.h"
#include "interfaces/i_style_service.h"

/**
 * @interface IComponentFactory
 * @brief Interface for dynamic component and panel creation
 * 
 * @details This interface abstracts component and panel factory operations,
 * providing a unified interface for registering and creating UI components
 * and panels with dependency injection. Implementations should handle
 * type registration, factory function storage, and dynamic object creation.
 * 
 * @design_pattern Interface Segregation - Focused on factory operations only
 * @design_pattern Abstract Factory - Creates families of related objects
 * @testability Enables mocking for unit tests with test components
 * @dependency_injection Supports injecting dependencies during creation
 */
class IComponentFactory
{
public:
    // Type Definitions
    using PanelFactoryFunction = std::function<std::unique_ptr<IPanel>(IGpioProvider*, IDisplayProvider*)>;
    using ComponentFactoryFunction = std::function<std::unique_ptr<IComponent>(IDisplayProvider*, IStyleService*)>;

    virtual ~IComponentFactory() = default;

    // Panel Factory Methods
    
    /**
     * @brief Register a panel type with a factory function
     * @param name Unique identifier for the panel type
     * @param factory Function that creates panel instances with dependencies
     */
    virtual void registerPanel(const std::string& name, PanelFactoryFunction factory) = 0;

    /**
     * @brief Create a panel instance by name with injected dependencies
     * @param name Panel type identifier
     * @param gpio GPIO provider for hardware access
     * @param display Display provider for UI operations
     * @return Unique pointer to created panel instance
     */
    virtual std::unique_ptr<IPanel> createPanel(const std::string& name, 
                                               IGpioProvider* gpio, 
                                               IDisplayProvider* display) = 0;

    /**
     * @brief Check if a panel type is registered
     * @param name Panel type identifier to check
     * @return True if panel type is registered, false otherwise
     */
    virtual bool hasPanelRegistration(const std::string& name) const = 0;

    // Component Factory Methods
    
    /**
     * @brief Register a component type with a factory function
     * @param name Unique identifier for the component type
     * @param factory Function that creates component instances with dependencies
     */
    virtual void registerComponent(const std::string& name, ComponentFactoryFunction factory) = 0;

    /**
     * @brief Create a component instance by name with injected dependencies
     * @param name Component type identifier
     * @param display Display provider for UI operations
     * @param style Style service for theme and styling operations
     * @return Unique pointer to created component instance
     */
    virtual std::unique_ptr<IComponent> createComponent(const std::string& name, 
                                                       IDisplayProvider* display,
                                                       IStyleService* style) = 0;

    /**
     * @brief Check if a component type is registered
     * @param name Component type identifier to check
     * @return True if component type is registered, false otherwise
     */
    virtual bool hasComponentRegistration(const std::string& name) const = 0;

    // Utility Methods
    
    /**
     * @brief Clear all registered factories (useful for testing)
     */
    virtual void clear() = 0;
};