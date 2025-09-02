#pragma once
#include <memory>

class IStyleService;
class IComponent;
class ClarityComponent;
class OemOilPressureComponent;
class OemOilTemperatureComponent;
class ErrorComponent;
class KeyComponent;
class LockComponent;
class ConfigComponent;

/**
 * @interface IComponentFactory
 * @brief Factory interface for creating UI components with dependency injection
 *
 * @details This interface provides the contract for creating various UI components
 * used throughout the Clarity system. It implements the Abstract Factory pattern
 * to enable testability and proper dependency injection of style services into
 * component constructors.
 *
 * @design_pattern Abstract Factory Pattern for component creation
 * @testability Enables dependency injection of mock factories for testing
 * @memory_management All methods return unique_ptr for clear ownership transfer
 * @dependency_injection Components receive required services during construction
 *
 * @component_types:
 * - ClarityComponent: Branding/logo display component
 * - OilPressureComponent: Engine oil pressure gauge component
 * - OilTemperatureComponent: Engine oil temperature gauge component
 * - ErrorComponent: Error message display component
 * - KeyComponent: Key presence status indicator component
 * - LockComponent: Vehicle lock status indicator component
 * - ConfigComponent: Configuration UI component
 *
 * @usage_pattern:
 * 1. Panels request components from factory during initialization
 * 2. Factory creates component with proper style service injection
 * 3. Panel uses component for rendering and updates
 * 4. Unique pointer ownership transfers to panel
 *
 * @example
 * @code
 * auto component = componentFactory->CreateOilPressureComponent(styleService);
 * component->Render(screen, location, displayProvider);
 * component->Refresh(sensorReading);
 * @endcode
 *
 * @context This factory centralizes component creation and ensures all
 * components receive proper dependencies. It enables testing by allowing
 * mock component injection and maintains clean separation between component
 * creation and usage.
 */
class IComponentFactory {
public:
    /// @brief Virtual destructor for interface
    virtual ~IComponentFactory() = default;

    /// @brief Create a Clarity branding/logo component
    /// @param style Style service for theme and appearance management
    /// @return Unique pointer to ClarityComponent instance
    virtual std::unique_ptr<ClarityComponent> CreateClarityComponent(IStyleService* style) = 0;

    /// @brief Create an oil pressure gauge component
    /// @param style Style service for theme and gauge styling
    /// @return Unique pointer to IComponent instance (OilPressureComponent)
    virtual std::unique_ptr<IComponent> CreateOilPressureComponent(IStyleService* style) = 0;

    /// @brief Create an oil temperature gauge component
    /// @param style Style service for theme and gauge styling
    /// @return Unique pointer to IComponent instance (OilTemperatureComponent)
    virtual std::unique_ptr<IComponent> CreateOilTemperatureComponent(IStyleService* style) = 0;

    /// @brief Create an error message display component
    /// @param style Style service for error styling and themes
    /// @return Unique pointer to ErrorComponent instance
    virtual std::unique_ptr<ErrorComponent> CreateErrorComponent(IStyleService* style) = 0;

    /// @brief Create a key presence status indicator component
    /// @param style Style service for indicator styling
    /// @return Unique pointer to KeyComponent instance
    virtual std::unique_ptr<KeyComponent> CreateKeyComponent(IStyleService* style) = 0;

    /// @brief Create a vehicle lock status indicator component
    /// @param style Style service for indicator styling
    /// @return Unique pointer to LockComponent instance
    virtual std::unique_ptr<LockComponent> CreateLockComponent(IStyleService* style) = 0;

    /// @brief Create a configuration UI component
    /// @param style Style service for configuration UI styling
    /// @return Unique pointer to ConfigComponent instance
    virtual std::unique_ptr<ConfigComponent> CreateConfigComponent(IStyleService* style) = 0;
};