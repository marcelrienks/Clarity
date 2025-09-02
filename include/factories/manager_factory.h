#pragma once

#include "interfaces/i_manager_factory.h"
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
class PreferenceManager;
class InterruptManager;

/**
 * @class ManagerFactory
 * @brief Concrete factory for creating manager instances with error handling and logging
 *
 * @details This factory implements IManagerFactory interface and provides static methods 
 * for creating all manager types used in the Clarity system. Each factory method includes 
 * proper error handling, null checking, dependency validation, and debug logging for 
 * initialization tracking.
 *
 * @design_pattern Concrete Factory (implements Abstract Factory)
 * @error_handling All methods return nullptr on failure with error logging
 * @logging Debug level logging for successful creations, error level for failures
 * @dependency_validation All required dependencies are validated before construction
 * @testability Implements IManagerFactory interface for test injection
 */
class ManagerFactory : public IManagerFactory
{
public:
    /// @brief Default constructor
    ManagerFactory() = default;
    
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

    /// @brief Create PreferenceManager (no dependencies currently)
    /// @return Unique pointer to configured PreferenceManager instance or nullptr on failure
    std::unique_ptr<PreferenceManager> CreatePreferenceManager() override;

    /// @brief Initialize InterruptManager singleton instance with GPIO provider
    /// @param gpioProvider GPIO provider for handler sensor ownership
    /// @return Pointer to configured InterruptManager instance or nullptr on failure
    InterruptManager* CreateInterruptManager(IGpioProvider* gpioProvider) override;

    /// @brief Create ErrorManager singleton instance
    /// @return Pointer to configured ErrorManager instance or nullptr on failure
    ErrorManager* CreateErrorManager() override;
    
    // Static convenience methods for backward compatibility
    
    /// @brief Static convenience method - Create PanelManager with injected dependencies
    static std::unique_ptr<PanelManager> CreatePanelManagerStatic(IDisplayProvider *display, IGpioProvider *gpio,
                                                                  IStyleService *styleService, 
                                                                  IPreferenceService *preferenceService,
                                                                  InterruptManager *interruptManager);

    /// @brief Static convenience method - Create StyleManager with optional theme
    static std::unique_ptr<StyleManager> CreateStyleManagerStatic(const char *theme = nullptr);

    /// @brief Static convenience method - Create PreferenceManager
    static std::unique_ptr<PreferenceManager> CreatePreferenceManagerStatic();

    /// @brief Static convenience method - Initialize InterruptManager singleton
    static InterruptManager* CreateInterruptManagerStatic(IGpioProvider* gpioProvider);

    /// @brief Static convenience method - Create ErrorManager singleton
    static ErrorManager* CreateErrorManagerStatic();

private:
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
    static std::unique_ptr<PreferenceManager> CreatePreferenceManagerImpl();

    /// @brief Implementation helper for creating InterruptManager
    static InterruptManager* CreateInterruptManagerImpl(IGpioProvider* gpioProvider);

    /// @brief Implementation helper for creating ErrorManager
    static ErrorManager* CreateErrorManagerImpl();
};