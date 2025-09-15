#pragma once

#include "interfaces/i_manager_factory.h"
#include "interfaces/i_provider_factory.h"
#include "interfaces/i_display_provider.h"
#include "interfaces/i_gpio_provider.h"
#include "interfaces/i_panel_service.h"
#include "interfaces/i_preference_service.h"
#include "interfaces/i_style_service.h"
#include <Arduino.h>
#include <memory>

// Forward declarations
class PanelManager;
class StyleManager;
class DeviceProvider;
class PreferenceManager;
class InterruptManager;
class IProviderFactory;

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
class ManagerFactory : public IManagerFactory
{
public:
    /// @brief Constructor accepting provider factory for dependency injection
    /// @param providerFactory Factory for creating hardware providers (takes ownership)
    explicit ManagerFactory(std::unique_ptr<IProviderFactory> providerFactory);
    
    /// @brief Default constructor for backward compatibility (creates ProviderFactory internally)
    ManagerFactory();
    
    /// @brief Default destructor
    ~ManagerFactory() override = default;
    
    // IManagerFactory implementation

    /// @brief Create PanelManager with injected dependencies
    /// @param display Display provider for UI operations
    /// @param gpio GPIO provider for hardware access
    /// @param styleService Style service for UI theming
    /// @param preferenceService Preference service for configuration settings
    /// @param interruptManager Interrupt manager for button function injection
    /// @return Unique pointer to configured PanelManager instance or nullptr on failure
    /// @note ActionManager dependency removed - button handling moved to handler-based system
    std::unique_ptr<PanelManager> CreatePanelManager(IDisplayProvider *display, IGpioProvider *gpio,
                                                      IStyleService *styleService, 
                                                      IPreferenceService *preferenceService,
                                                      InterruptManager *interruptManager) override;

    /// @brief Create StyleManager with optional theme
    /// @param theme Initial theme to apply (defaults to DAY theme)
    /// @return Unique pointer to configured StyleManager instance or nullptr on failure
    std::unique_ptr<StyleManager> CreateStyleManager(const char *theme = nullptr) override;

    /// @brief Create PreferenceService (no dependencies currently)
    /// @return Unique pointer to configured IPreferenceService instance or nullptr on failure
    std::unique_ptr<IPreferenceService> CreatePreferenceManager() override;

    /// @brief Initialize InterruptManager singleton instance with GPIO provider
    /// @param gpioProvider GPIO provider for handler sensor ownership
    /// @return Pointer to configured InterruptManager instance or nullptr on failure
    InterruptManager* CreateInterruptManager(IGpioProvider* gpioProvider) override;

    /// @brief Create ErrorManager singleton instance
    /// @return Pointer to configured ErrorManager instance or nullptr on failure
    ErrorManager* CreateErrorManager() override;

private:
    /// @brief Provider factory for creating hardware providers
    std::unique_ptr<IProviderFactory> providerFactory_;
    
    /// @brief Cached providers created from provider factory
    std::unique_ptr<IGpioProvider> gpioProvider_;
    std::unique_ptr<IDisplayProvider> displayProvider_;
    std::unique_ptr<DeviceProvider> deviceProvider_;
    
    /// @brief Initialize providers from factory if not already created
    bool InitializeProviders();
    
    /// @brief Implementation helper for creating managers
    /// @param display Display provider for UI operations
    /// @param gpio GPIO provider for hardware access  
    /// @param styleService Style service for UI theming
    /// @param preferenceService Preference service for configuration settings
    /// @param interruptManager Interrupt manager for button function injection
    /// @return Unique pointer to configured PanelManager instance or nullptr on failure
    static std::unique_ptr<PanelManager> CreatePanelManagerImpl(IDisplayProvider *display, IGpioProvider *gpio,
                                                                IStyleService *styleService, 
                                                                IPreferenceService *preferenceService,
                                                                InterruptManager *interruptManager);

    /// @brief Implementation helper for creating StyleManager
    static std::unique_ptr<StyleManager> CreateStyleManagerImpl(const char *theme = nullptr);

    /// @brief Implementation helper for creating PreferenceManager
    static std::unique_ptr<IPreferenceService> CreatePreferenceManagerImpl();

    /// @brief Implementation helper for creating InterruptManager
    static InterruptManager* CreateInterruptManagerImpl(IGpioProvider* gpioProvider);

    /// @brief Implementation helper for creating ErrorManager
    static ErrorManager* CreateErrorManagerImpl();
};