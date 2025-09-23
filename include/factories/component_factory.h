#pragma once

#include <memory>
#include "interfaces/i_component_factory.h"

class IStyleManager;
class ClarityComponent;
class OilPressureComponent;
class OilTemperatureComponent;
class ErrorComponent;
class KeyComponent;
class LockComponent;
class ConfigComponent;

/**
 * @class ComponentFactory
 * @brief Singleton factory for creating UI component instances
 * 
 * @details This factory creates LVGL-based UI components used within panels. 
 * It follows the singleton pattern to provide a global component creation service.
 * This factory supplements the main dual factory pattern (ProviderFactory/ManagerFactory)
 * by handling component-level object creation.
 * 
 * @design_pattern Singleton Factory Pattern
 * @architectural_role Component creation for MVP architecture Views layer
 * @dependency_injection Components receive IStyleManager for theme management
 * @testability Implements IComponentFactory interface for test injection
 * 
 * @architecture_note
 * While not documented in the main architectural overview, this factory serves
 * a crucial role in the MVP pattern by creating View layer components that
 * panels (Presenters) coordinate. It complements the documented dual factory
 * pattern by handling finer-grained component creation.
 * 
 * @example
 * @code
 * // Usage within panels
 * auto& factory = ComponentFactory::Instance();
 * auto pressureComponent = factory.CreateOilPressureComponent(styleService);
 * auto temperatureComponent = factory.CreateOilTemperatureComponent(styleService);
 * @endcode
 */
class ComponentFactory : public IComponentFactory {
public:
    // ========== Static Methods ==========
    static ComponentFactory& Instance();
    
    // ========== Public Interface Methods ==========
    // IComponentFactory implementation
    std::unique_ptr<ClarityComponent> CreateClarityComponent(IStyleManager* style) override;
    std::unique_ptr<IComponent> CreateOilPressureComponent(IStyleManager* style) override;
    std::unique_ptr<IComponent> CreateOilTemperatureComponent(IStyleManager* style) override;
    std::unique_ptr<ErrorComponent> CreateErrorComponent(IStyleManager* style) override;
    std::unique_ptr<KeyComponent> CreateKeyComponent(IStyleManager* style) override;
    std::unique_ptr<LockComponent> CreateLockComponent(IStyleManager* style) override;
    std::unique_ptr<ConfigComponent> CreateConfigComponent(IStyleManager* style) override;
    
private:
    // ========== Constructors and Destructor ==========
    ComponentFactory() = default;
    ~ComponentFactory() = default;
    ComponentFactory(const ComponentFactory&) = delete;
    ComponentFactory& operator=(const ComponentFactory&) = delete;
};