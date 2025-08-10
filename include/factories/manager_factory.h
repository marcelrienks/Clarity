#pragma once

#include "interfaces/i_panel_service.h"
#include "interfaces/i_style_service.h"
#include "interfaces/i_trigger_service.h"
#include "interfaces/i_preference_service.h"
#include "interfaces/i_gpio_provider.h"
#include "interfaces/i_display_provider.h"
#include <memory>
#include <Arduino.h>

// Forward declarations
class PanelManager;
class StyleManager;
class TriggerManager;
class PreferenceManager;
class InputManager;
class InterruptManager;

/**
 * @class ManagerFactory
 * @brief Factory for creating manager instances with error handling and logging
 * 
 * @details This factory provides static methods for creating all manager types
 * used in the Clarity system. Each factory method includes proper error handling,
 * null checking, dependency validation, and debug logging for initialization tracking.
 * 
 * @design_pattern Factory Pattern with Dependency Injection
 * @error_handling All methods return nullptr on failure with error logging
 * @logging Debug level logging for successful creations, error level for failures
 * @dependency_validation All required dependencies are validated before construction
 */
class ManagerFactory
{
public:
    // Factory Methods
    
    /// @brief Create PanelManager with injected dependencies
    /// @param display Display provider for UI operations
    /// @param gpio GPIO provider for hardware access
    /// @param styleService Style service for UI theming
    /// @param inputManager Input manager for button handling
    /// @return Unique pointer to configured PanelManager instance or nullptr on failure
    static std::unique_ptr<PanelManager> createPanelManager(IDisplayProvider* display, IGpioProvider* gpio, IStyleService* styleService, InputManager* inputManager);
    
    /// @brief Create StyleManager with optional theme
    /// @param theme Initial theme to apply (defaults to DAY theme)
    /// @return Unique pointer to configured StyleManager instance or nullptr on failure
    static std::unique_ptr<StyleManager> createStyleManager(const char* theme = nullptr);
    
    /// @brief Create TriggerManager with injected sensor dependencies
    /// @param gpio GPIO provider for creating sensors
    /// @param panelService Panel service for loading panels
    /// @param styleService Style service for theme management
    /// @return Unique pointer to configured TriggerManager instance or nullptr on failure
    static std::unique_ptr<TriggerManager> createTriggerManager(IGpioProvider* gpio, IPanelService* panelService, IStyleService* styleService);
    
    /// @brief Create PreferenceManager (no dependencies currently)
    /// @return Unique pointer to configured PreferenceManager instance or nullptr on failure
    static std::unique_ptr<PreferenceManager> createPreferenceManager();
    
    /// @brief Create InputManager with injected sensor dependencies
    /// @param gpio GPIO provider for creating InputButtonSensor
    /// @param panelService Panel service for triggering panel switches (can be nullptr)
    /// @return Unique pointer to configured InputManager instance or nullptr on failure
    static std::unique_ptr<InputManager> createInputManager(IGpioProvider* gpio, IPanelService* panelService);
    
    /// @brief Create InterruptManager (no dependencies currently)
    /// @return Unique pointer to configured InterruptManager instance or nullptr on failure
    static std::unique_ptr<InterruptManager> createInterruptManager();
    
    /// @brief Create ErrorManager singleton instance
    /// @return Pointer to configured ErrorManager instance or nullptr on failure
    static class ErrorManager* createErrorManager();

private:
    // Private constructor to prevent instantiation
    ManagerFactory() = delete;
    ~ManagerFactory() = delete;
    ManagerFactory(const ManagerFactory&) = delete;
    ManagerFactory& operator=(const ManagerFactory&) = delete;
};