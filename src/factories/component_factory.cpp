#include "factories/component_factory.h"
#include "components/clarity_component.h"
#include "components/oem/oem_oil_pressure_component.h"
#include "components/oem/oem_oil_temperature_component.h"
#include "components/error_component.h"
#include "components/key_component.h"
#include "components/lock_component.h"
#include "components/config_component.h"
#include "interfaces/i_style_service.h"

#include "esp32-hal-log.h"

// ========== Static Methods ==========

/**
 * @brief Gets the singleton instance of ComponentFactory
 * @return Reference to the singleton ComponentFactory
 *
 * Provides global access to the component factory using the singleton pattern.
 * The instance is created on first access and persists for the application lifetime.
 */
ComponentFactory& ComponentFactory::Instance()
{
    static ComponentFactory instance;
    return instance;
}

// ========== Public Interface Methods ==========

/**
 * @brief Creates a Clarity splash screen component
 * @param style Style service for theme management
 * @return Unique pointer to ClarityComponent or nullptr on failure
 *
 * Instantiates a ClarityComponent with style service dependency.
 * Logs error and returns nullptr if allocation fails.
 */
std::unique_ptr<ClarityComponent> ComponentFactory::CreateClarityComponent(IStyleService* style)
{
    auto component = std::make_unique<ClarityComponent>(style);
    if (!component) {
        log_e("Failed to create ClarityComponent - allocation failed");
        return nullptr;
    }
    return component;
}

/**
 * @brief Creates an oil pressure gauge component
 * @param style Style service for theme management
 * @return Unique pointer to IComponent interface or nullptr on failure
 *
 * Instantiates an OemOilPressureComponent for displaying oil pressure readings.
 * Returns as IComponent interface for polymorphic usage.
 */
std::unique_ptr<IComponent> ComponentFactory::CreateOilPressureComponent(IStyleService* style)
{
    auto component = std::make_unique<OemOilPressureComponent>(style);
    if (!component) {
        log_e("Failed to create OemOilPressureComponent - allocation failed");
        return nullptr;
    }
    return component;
}

/**
 * @brief Creates an oil temperature gauge component
 * @param style Style service for theme management
 * @return Unique pointer to IComponent interface or nullptr on failure
 *
 * Instantiates an OemOilTemperatureComponent for displaying oil temperature readings.
 * Returns as IComponent interface for polymorphic usage.
 */
std::unique_ptr<IComponent> ComponentFactory::CreateOilTemperatureComponent(IStyleService* style)
{
    auto component = std::make_unique<OemOilTemperatureComponent>(style);
    if (!component) {
        log_e("Failed to create OemOilTemperatureComponent - allocation failed");
        return nullptr;
    }
    return component;
}

/**
 * @brief Creates an error display component
 * @param style Style service for theme management
 * @return Unique pointer to ErrorComponent or nullptr on failure
 *
 * Instantiates an ErrorComponent for displaying system errors.
 * Provides full error display functionality with navigation.
 */
std::unique_ptr<ErrorComponent> ComponentFactory::CreateErrorComponent(IStyleService* style)
{
    auto component = std::make_unique<ErrorComponent>(style);
    if (!component) {
        log_e("Failed to create ErrorComponent - allocation failed");
        return nullptr;
    }
    return component;
}

/**
 * @brief Creates a key icon component
 * @param style Style service for theme management
 * @return Unique pointer to KeyComponent or nullptr on failure
 *
 * Instantiates a KeyComponent for displaying key presence status.
 * Shows colored key icon based on ignition key state.
 */
std::unique_ptr<KeyComponent> ComponentFactory::CreateKeyComponent(IStyleService* style)
{
    auto component = std::make_unique<KeyComponent>(style);
    if (!component) {
        log_e("Failed to create KeyComponent - allocation failed");
        return nullptr;
    }
    return component;
}

/**
 * @brief Creates a lock icon component
 * @param style Style service for theme management
 * @return Unique pointer to LockComponent or nullptr on failure
 *
 * Instantiates a LockComponent for displaying security lock status.
 * Shows lock icon to indicate system security state.
 */
std::unique_ptr<LockComponent> ComponentFactory::CreateLockComponent(IStyleService* style)
{
    auto component = std::make_unique<LockComponent>(style);
    if (!component) {
        log_e("Failed to create LockComponent - allocation failed");
        return nullptr;
    }
    return component;
}

/**
 * @brief Creates a configuration menu component
 * @param style Style service for theme management (unused)
 * @return Unique pointer to ConfigComponent or nullptr on failure
 *
 * Instantiates a ConfigComponent for configuration menu display.
 * Note: ConfigComponent doesn't use style service in constructor.
 */
std::unique_ptr<ConfigComponent> ComponentFactory::CreateConfigComponent(IStyleService* style)
{
    // ConfigComponent doesn't use style service in constructor
    auto component = std::make_unique<ConfigComponent>();
    if (!component) {
        log_e("Failed to create ConfigComponent - allocation failed");
        return nullptr;
    }
    return component;
}