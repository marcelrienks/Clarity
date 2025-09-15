#pragma once

#include <memory>

// Forward declarations
class IGpioProvider;
class IDisplayProvider;
class IStyleService;
class IPreferenceService;
class PanelManager;
class StyleManager;
class PreferenceManager;
class InterruptManager;
class ErrorManager;

/**
 * @interface IManagerFactory
 * @brief Factory interface for creating manager instances with dependency injection
 * 
 * @details This interface enables testability by allowing mock manager injection
 * in test scenarios. The concrete ManagerFactory implementation creates real
 * manager instances, while test code can provide mock implementations.
 * 
 * @design_pattern Abstract Factory Pattern
 * @testability Enables dependency injection of mock managers
 * @memory Manager creation methods return unique_ptr for clear ownership transfer
 * @dependency_injection All managers receive their dependencies through constructor injection
 * 
 * @usage
 * Production:
 * @code
 * auto managerFactory = std::make_unique<ManagerFactory>();
 * auto panelManager = managerFactory->CreatePanelManager(display, gpio, style, prefs, interrupt);
 * @endcode
 * 
 * Testing:
 * @code
 * auto mockManagerFactory = std::make_unique<MockManagerFactory>();
 * auto mockPanelManager = mockManagerFactory->CreatePanelManager(mockDisplay, mockGpio, mockStyle, mockPrefs, mockInterrupt);
 * @endcode
 */
class IManagerFactory
{
public:
    /// @brief Virtual destructor for interface
    virtual ~IManagerFactory() = default;
    
    /// @brief Create PanelManager with injected dependencies
    /// @param display Display provider for UI operations
    /// @param gpio GPIO provider for hardware access
    /// @param styleService Style service for UI theming
    /// @param preferenceService Preference service for configuration settings
    /// @param interruptManager Interrupt manager for button function injection
    /// @return Unique pointer to configured PanelManager instance or nullptr on failure
    virtual std::unique_ptr<PanelManager> CreatePanelManager(IDisplayProvider *display, 
                                                              IGpioProvider *gpio,
                                                              IStyleService *styleService, 
                                                              IPreferenceService *preferenceService,
                                                              InterruptManager *interruptManager) = 0;

    /// @brief Create StyleManager with optional theme
    /// @param theme Initial theme to apply (defaults to DAY theme)
    /// @return Unique pointer to configured StyleManager instance or nullptr on failure
    virtual std::unique_ptr<StyleManager> CreateStyleManager(const char *theme = nullptr) = 0;

    /// @brief Create PreferenceService (no dependencies currently)
    /// @return Unique pointer to configured IPreferenceService instance or nullptr on failure
    virtual std::unique_ptr<IPreferenceService> CreatePreferenceManager() = 0;

    /// @brief Initialize InterruptManager singleton instance with GPIO provider
    /// @param gpioProvider GPIO provider for handler sensor ownership
    /// @return Pointer to configured InterruptManager instance or nullptr on failure
    virtual InterruptManager* CreateInterruptManager(IGpioProvider* gpioProvider) = 0;

    /// @brief Create ErrorManager singleton instance
    /// @return Pointer to configured ErrorManager instance or nullptr on failure
    virtual ErrorManager* CreateErrorManager() = 0;
};