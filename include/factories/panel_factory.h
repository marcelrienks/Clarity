#pragma once

/**
 * @file panel_factory.h
 * @brief Factory for creating UI panels with runtime registration
 * 
 * @details This factory provides a flexible, registry-based approach to panel
 * creation. Panels can be registered at runtime, enabling dynamic panel
 * loading and better testability through dependency injection.
 * 
 * @design_pattern Registry-based Factory Pattern
 * @benefits:
 * - Runtime registration of panel types
 * - Decoupled panel creation from factory implementation
 * - Easy addition of new panel types without modifying factory
 * - Supports mock panels for testing
 * - Eliminates string-based if-else chains
 * 
 * @usage:
 * 1. Register panel creators during initialization
 * 2. Create panels by name when needed
 * 3. Factory handles dependency injection automatically
 */

// System/Library Includes
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

// Project Includes
#include "interfaces/i_panel.h"
#include "interfaces/i_gpio_provider.h"
#include "interfaces/i_display_provider.h"
#include "interfaces/i_style_service.h"

/**
 * @class PanelFactory
 * @brief Registry-based factory for creating UI panels
 * 
 * @details Manages panel creation through a registration system where
 * panel creators are registered with string identifiers. This allows
 * for runtime configuration and easy extension of available panels.
 * 
 * @thread_safety Static methods are not thread-safe by default
 * @memory_management Returns unique_ptr for automatic cleanup
 * @extensibility New panels can be added without modifying this class
 */
class PanelFactory
{
public:
    /// @brief Function type for panel creation
    /// @param gpioProvider GPIO provider for hardware access
    /// @param displayProvider Display provider for screen operations
    /// @param styleService Style service for theme management
    /// @return Newly created panel instance
    using PanelCreator = std::function<std::unique_ptr<IPanel>(
        IGpioProvider*, IDisplayProvider*, IStyleService*)>;

    /**
     * @brief Register a panel creator with the factory
     * @param name Unique identifier for the panel type (e.g., "OemOilPanel")
     * @param creator Function that creates the panel
     * @details Overwrites any existing registration with the same name
     */
    static void RegisterPanel(const std::string& name, PanelCreator creator);

    /**
     * @brief Create a panel by name
     * @param name Identifier of the panel type to create
     * @param gpioProvider GPIO provider for hardware access
     * @param displayProvider Display provider for screen operations
     * @param styleService Style service for theme management
     * @return New panel instance or nullptr if type not found
     * @throws May propagate exceptions from panel constructors
     */
    static std::unique_ptr<IPanel> CreatePanel(const std::string& name,
                                               IGpioProvider* gpioProvider,
                                               IDisplayProvider* displayProvider,
                                               IStyleService* styleService);

    /**
     * @brief Check if a panel type is registered
     * @param name Panel type identifier to check
     * @return true if panel type is registered
     */
    static bool IsPanelRegistered(const std::string& name);

    /**
     * @brief Clear all registered panel types
     * @details Useful for testing or cleanup
     */
    static void ClearRegistrations();

    /**
     * @brief Get list of all registered panel types
     * @return Vector of registered panel names sorted alphabetically
     */
    static std::vector<std::string> GetRegisteredPanels();

private:
    /// @brief Registry of panel creators indexed by name
    static std::unordered_map<std::string, PanelCreator> panelCreators_;
    
    // Prevent instantiation - static class only
    PanelFactory() = delete;
    ~PanelFactory() = delete;
    PanelFactory(const PanelFactory&) = delete;
    PanelFactory& operator=(const PanelFactory&) = delete;
};