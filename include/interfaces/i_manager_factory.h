#pragma once

#include <memory>

// Forward declarations
class IGpioProvider;
class IDisplayProvider;
class IStyleManager;
class IConfigurationManager;
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
    virtual ~IManagerFactory() = default;
    
    virtual std::unique_ptr<PanelManager> CreatePanelManager(IDisplayProvider *display, 
                                                              IGpioProvider *gpio,
                                                              IStyleManager *styleService,
                                                              IConfigurationManager *preferenceService,
                                                              InterruptManager *interruptManager) = 0;

    virtual std::unique_ptr<StyleManager> CreateStyleManager(const char *theme = nullptr) = 0;

    virtual std::unique_ptr<IConfigurationManager> CreatePreferenceManager() = 0;

    virtual InterruptManager* CreateInterruptManager(IGpioProvider* gpioProvider) = 0;

    virtual ErrorManager* CreateErrorManager() = 0;
};