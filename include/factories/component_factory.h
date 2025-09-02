#pragma once

#include <memory>
#include "interfaces/i_component_factory.h"

class IStyleService;
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
 * @dependency_injection Components receive IStyleService for theme management
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
    static ComponentFactory& Instance();
    
    // IComponentFactory implementation
    std::unique_ptr<ClarityComponent> CreateClarityComponent(IStyleService* style) override;
    std::unique_ptr<IComponent> CreateOilPressureComponent(IStyleService* style) override;
    std::unique_ptr<IComponent> CreateOilTemperatureComponent(IStyleService* style) override;
    std::unique_ptr<ErrorComponent> CreateErrorComponent(IStyleService* style) override;
    std::unique_ptr<KeyComponent> CreateKeyComponent(IStyleService* style) override;
    std::unique_ptr<LockComponent> CreateLockComponent(IStyleService* style) override;
    std::unique_ptr<ConfigComponent> CreateConfigComponent(IStyleService* style) override;
    
private:
    ComponentFactory() = default;
    ~ComponentFactory() = default;
    ComponentFactory(const ComponentFactory&) = delete;
    ComponentFactory& operator=(const ComponentFactory&) = delete;
};