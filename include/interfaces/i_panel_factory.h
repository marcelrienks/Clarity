#pragma once

// System/Library Includes
#include <memory>
#include <string>

// Project Includes
#include "interfaces/i_panel.h"
#include "interfaces/i_component_factory.h"
#include "interfaces/i_display_provider.h"
#include "interfaces/i_gpio_provider.h"

/**
 * @interface IPanelFactory
 * @brief Interface for dynamic panel creation with dependency injection
 * 
 * @details This interface abstracts panel factory operations, providing
 * a focused interface for creating UI panels with proper dependency injection.
 * Separates panel creation concerns from component creation for better
 * architectural separation.
 * 
 * @design_pattern Interface Segregation - Focused only on panel creation
 * @design_pattern Abstract Factory - Creates panel objects with dependencies
 * @testability Enables mocking for unit tests with test panels
 * @dependency_injection Supports injecting component factory and providers
 */
class IPanelFactory
{
public:
    virtual ~IPanelFactory() = default;

    /**
     * @brief Create a panel instance by type with injected dependencies
     * @param panelType Panel type identifier (e.g., "splash", "oil", "key", "lock")
     * @return Unique pointer to created panel instance
     * @throws std::runtime_error if panel type is not supported
     */
    virtual std::unique_ptr<IPanel> createPanel(const std::string& panelType) = 0;

    /**
     * @brief Check if a panel type is supported
     * @param panelType Panel type identifier to check
     * @return True if panel type is supported, false otherwise
     */
    virtual bool supportsPanel(const std::string& panelType) const = 0;
};