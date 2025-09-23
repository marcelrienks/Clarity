#pragma once

#include "interfaces/i_display_provider.h"
#include "interfaces/i_gpio_provider.h"
#include "interfaces/i_panel_manager.h"
#include "interfaces/i_configuration_manager.h"
#include "interfaces/i_style_manager.h"
#include <Arduino.h>
#include <memory>

// Forward declarations
class PanelManager;
class StyleManager;
class DeviceProvider;
class ConfigurationManager;
class InterruptManager;
class ErrorManager;
class ProviderFactory;

/**
 * @class ManagerFactory
 * @brief Concrete factory for creating manager instances with error handling and logging
 *
 * @details This factory implements IManagerFactory interface and provides methods 
 * for creating all manager types used in the Clarity system. It uses IProviderFactory
 * to obtain hardware providers, implementing the dual factory pattern for clear
 * separation of concerns. Each factory method includes proper error handling,
 * null checking, dependency validation, and debug logging for initialization tracking.
 *
 * @design_pattern Dual Factory Pattern (Concrete Factory using Abstract Provider Factory)
 * @error_handling All methods return nullptr on failure with error logging
 * @logging Debug level logging for successful creations, error level for failures
 * @dependency_validation All required dependencies are validated before construction
 * @testability Implements IManagerFactory interface and accepts IProviderFactory for test injection
 */
class ManagerFactory
{
public:
    // ========== Constructors and Destructor ==========
    explicit ManagerFactory(std::unique_ptr<ProviderFactory> providerFactory);
    ManagerFactory();
    ~ManagerFactory() = default;
    
    // ========== Public Interface Methods ==========
    // Factory methods
    std::unique_ptr<PanelManager> CreatePanelManager(IDisplayProvider *display, IGpioProvider *gpio,
                                                      IStyleManager *styleService,
                                                      IConfigurationManager *preferenceService,
                                                      InterruptManager *interruptManager);

    std::unique_ptr<StyleManager> CreateStyleManager(const char *theme = nullptr);

    std::unique_ptr<IConfigurationManager> CreatePreferenceManager();

    InterruptManager* CreateInterruptManager(IGpioProvider* gpioProvider);

    ErrorManager* CreateErrorManager();

private:
    // ========== Private Methods ==========
    bool InitializeProviders();
    
    // ========== Static Methods ==========
    static std::unique_ptr<PanelManager> CreatePanelManagerImpl(IDisplayProvider *display, IGpioProvider *gpio,
                                                                IStyleManager *styleService,
                                                                IConfigurationManager *preferenceService,
                                                                InterruptManager *interruptManager);

    static std::unique_ptr<StyleManager> CreateStyleManagerImpl(const char *theme = nullptr);

    static std::unique_ptr<IConfigurationManager> CreatePreferenceManagerImpl();

    static InterruptManager* CreateInterruptManagerImpl(IGpioProvider* gpioProvider);

    static ErrorManager* CreateErrorManagerImpl();

    // ========== Private Data Members ==========
    std::unique_ptr<ProviderFactory> providerFactory_;
    
    std::unique_ptr<IGpioProvider> gpioProvider_;
    std::unique_ptr<IDisplayProvider> displayProvider_;
    std::unique_ptr<DeviceProvider> deviceProvider_;
};