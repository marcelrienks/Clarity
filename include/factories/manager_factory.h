#pragma once

#include "interfaces/i_panel_service.h"
#include "interfaces/i_style_service.h"
#include "interfaces/i_trigger_service.h"
#include "interfaces/i_preference_service.h"
#include "interfaces/i_gpio_provider.h"
#include "interfaces/i_display_provider.h"
#include <memory>

// Forward declarations
class PanelManager;
class StyleManager;
class TriggerManager;
class PreferenceManager;

/**
 * @class ManagerFactory
 * @brief Factory for creating manager instances with dependency injection
 * 
 * @details This factory replaces the singleton pattern with dependency injection
 * to enable better testability and control over manager lifecycle. Each manager
 * type can be created with its required dependencies injected via constructor.
 * 
 * @design_benefits:
 * - Explicit dependencies make testing easier
 * - No global state between tests
 * - Parallel test execution becomes possible
 * - Managers can be created with mock dependencies for testing
 * 
 * @usage_pattern:
 * ```cpp
 * // Production usage
 * auto panelManager = ManagerFactory::createPanelManager(displayProvider, gpioProvider);
 * panelManager->loadPanel(PanelNames::KEY);
 * 
 * // Test usage
 * auto mockDisplay = std::make_unique<MockDisplayProvider>();
 * auto mockGpio = std::make_unique<MockGpioProvider>();
 * auto panelManager = ManagerFactory::createPanelManager(mockDisplay.get(), mockGpio.get());
 * ```
 */
class ManagerFactory
{
public:
    // Factory Methods
    
    /// @brief Create PanelManager with injected dependencies
    /// @param display Display provider for UI operations
    /// @param gpio GPIO provider for hardware access (optional for some managers)
    /// @param componentFactory Component factory for creating components with DI
    /// @return Unique pointer to configured PanelManager instance
    static std::unique_ptr<PanelManager> createPanelManager(IDisplayProvider* display, IGpioProvider* gpio, IStyleService* styleService);
    
    /// @brief Create StyleManager with optional theme
    /// @param theme Initial theme to apply (defaults to night theme)
    /// @return Unique pointer to configured StyleManager instance
    static std::unique_ptr<StyleManager> createStyleManager(const char* theme = nullptr);
    
    /// @brief Create TriggerManager with injected sensor dependencies
    /// @param gpio GPIO provider for creating sensors
    /// @param panelService Panel service for loading panels
    /// @param styleService Style service for theme management
    /// @return Unique pointer to configured TriggerManager instance
    static std::unique_ptr<TriggerManager> createTriggerManager(IGpioProvider* gpio, IPanelService* panelService, IStyleService* styleService);
    
    /// @brief Create PreferenceManager (no dependencies currently)
    /// @return Unique pointer to configured PreferenceManager instance
    static std::unique_ptr<PreferenceManager> createPreferenceManager();

private:
    // Private constructor to prevent instantiation
    ManagerFactory() = delete;
    ~ManagerFactory() = delete;
    ManagerFactory(const ManagerFactory&) = delete;
    ManagerFactory& operator=(const ManagerFactory&) = delete;
};