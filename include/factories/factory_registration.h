#pragma once

/**
 * @file factory_registration.h
 * @brief Central registration of all panels and components with their respective factories
 * 
 * @details This header provides registration functions that initialize the
 * ComponentFactory and PanelFactory with all available UI elements. This
 * centralizes the mapping between string identifiers and concrete classes.
 * 
 * @usage:
 * - Call RegisterAllPanels() during system initialization
 * - Call RegisterAllComponents() during system initialization
 * - These functions must be called before any factory Create* methods
 * 
 * @design_benefits:
 * - Single location to manage all UI element registrations
 * - Easy to add new panels/components
 * - Clear separation between registration and usage
 */

/**
 * @brief Register all available panel types with PanelFactory
 * @details Registers all panels including:
 * - SplashPanel: Startup branding screen
 * - OemOilPanel: Main monitoring dashboard
 * - ErrorPanel: System error display
 * - ConfigPanel: Configuration interface
 * - KeyPanel: Key presence display
 * - LockPanel: Lock status display
 * 
 * @note Must be called during system initialization before panel creation
 */
void RegisterAllPanels();

/**
 * @brief Register all available component types with ComponentFactory
 * @details Registers all components including:
 * - ClarityComponent: Branding/logo component
 * - OemOilPressureComponent: Pressure gauge
 * - OemOilTemperatureComponent: Temperature gauge
 * - ErrorComponent: Error display component
 * - KeyComponent: Key status indicator
 * - LockComponent: Lock status indicator
 * 
 * @note Must be called during system initialization before component creation
 */
void RegisterAllComponents();

/**
 * @brief Initialize both panel and component factories
 * @details Convenience function that calls both registration functions
 * in the correct order. Use this instead of calling individual functions.
 */
void InitializeFactories();