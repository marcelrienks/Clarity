#pragma once

/**
 * @file component_factory.h
 * @brief Factory for creating UI components with runtime registration
 * 
 * @details This factory provides a flexible, registry-based approach to component
 * creation. Components can be registered at runtime, enabling dynamic component
 * loading and better testability through dependency injection.
 * 
 * @design_pattern Registry-based Factory Pattern
 * @benefits:
 * - Runtime registration of component types
 * - Decoupled component creation from factory implementation
 * - Easy addition of new component types without modifying factory
 * - Supports mock components for testing
 * 
 * @usage:
 * 1. Register component creators during initialization
 * 2. Create components by name when needed
 * 3. Factory handles dependency injection automatically
 */

// System/Library Includes
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

// Project Includes
#include "interfaces/i_component.h"
#include "interfaces/i_style_service.h"

/**
 * @class ComponentFactory
 * @brief Registry-based factory for creating UI components
 * 
 * @details Manages component creation through a registration system where
 * component creators are registered with string identifiers. This allows
 * for runtime configuration and easy extension of available components.
 * 
 * @thread_safety Static methods use internal synchronization (if needed)
 * @memory_management Returns unique_ptr for automatic cleanup
 */
class ComponentFactory
{
public:
    /// @brief Function type for component creation
    /// @param styleService Style service for theme management
    /// @return Newly created component instance
    using ComponentCreator = std::function<std::unique_ptr<IComponent>(IStyleService*)>;

    /**
     * @brief Register a component creator with the factory
     * @param name Unique identifier for the component type
     * @param creator Function that creates the component
     * @details Overwrites any existing registration with the same name
     */
    static void RegisterComponent(const std::string& name, ComponentCreator creator);

    /**
     * @brief Create a component by name
     * @param name Identifier of the component type to create
     * @param styleService Style service for component styling
     * @return New component instance or nullptr if type not found
     */
    static std::unique_ptr<IComponent> CreateComponent(const std::string& name, 
                                                       IStyleService* styleService);

    /**
     * @brief Check if a component type is registered
     * @param name Component type identifier to check
     * @return true if component type is registered
     */
    static bool IsComponentRegistered(const std::string& name);

    /**
     * @brief Clear all registered component types
     * @details Useful for testing or cleanup
     */
    static void ClearRegistrations();

    /**
     * @brief Get list of all registered component types
     * @return Vector of registered component names
     */
    static std::vector<std::string> GetRegisteredComponents();

private:
    /// @brief Registry of component creators indexed by name
    static std::unordered_map<std::string, ComponentCreator> componentCreators_;
    
    // Prevent instantiation - static class only
    ComponentFactory() = delete;
    ~ComponentFactory() = delete;
    ComponentFactory(const ComponentFactory&) = delete;
    ComponentFactory& operator=(const ComponentFactory&) = delete;
};