#pragma once

#include "interfaces/i_action_manager.h"
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
class ActionManager;
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
    /// @param actionManager Action manager interface for button handling
    /// @param preferenceService Preference service for configuration settings
    /// @return Unique pointer to configured PanelManager instance or nullptr on failure
    static std::unique_ptr<PanelManager> createPanelManager(IDisplayProvider *display, IGpioProvider *gpio,
                                                            IStyleService *styleService, IActionManager *actionManager,
                                                            IPreferenceService *preferenceService);

    /// @brief Create StyleManager with optional theme
    /// @param theme Initial theme to apply (defaults to DAY theme)
    /// @return Unique pointer to configured StyleManager instance or nullptr on failure
    static std::unique_ptr<StyleManager> createStyleManager(const char *theme = nullptr);


    /// @brief Create PreferenceManager (no dependencies currently)
    /// @return Unique pointer to configured PreferenceManager instance or nullptr on failure
    static std::unique_ptr<PreferenceManager> createPreferenceManager();

    /// @brief Create ActionManager with injected sensor dependencies
    /// @param gpio GPIO provider for creating ActionButtonSensor
    /// @param panelService Panel service for triggering panel switches (can be nullptr)
    /// @return Unique pointer to configured ActionManager instance or nullptr on failure
    static std::unique_ptr<ActionManager> createActionManager(IGpioProvider *gpio, IPanelService *panelService);

    /// @brief Initialize InterruptManager singleton instance
    /// @return Pointer to configured InterruptManager instance or nullptr on failure
    static InterruptManager* createInterruptManager();

    /// @brief Create ErrorManager singleton instance
    /// @return Pointer to configured ErrorManager instance or nullptr on failure
    static class ErrorManager *createErrorManager();

  private:
    /// @brief Register all system interrupts with InterruptManager
    /// @param interruptManager The InterruptManager instance to register with
    static void RegisterSystemInterrupts(InterruptManager* interruptManager);
    // Private constructor to prevent instantiation
    ManagerFactory() = delete;
    ~ManagerFactory() = delete;
    ManagerFactory(const ManagerFactory &) = delete;
    ManagerFactory &operator=(const ManagerFactory &) = delete;
};