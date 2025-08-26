#pragma once

#include "interfaces/i_provider_factory.h"
#include <memory>

// Forward declarations
class PanelManager;
class StyleManager;
class PreferenceManager;
class InterruptManager;
class ErrorManager;
class IDisplayProvider;
class IGpioProvider;
class IStyleService;
class IPreferenceService;

/**
 * @class ManagerFactory
 * @brief Factory for creating manager instances with dependency injection
 * 
 * @details This factory creates all manager types used in the Clarity system.
 * It receives an IProviderFactory instance to obtain hardware providers,
 * enabling testability through mock provider injection.
 * 
 * @design_pattern Factory Pattern with Dependency Injection
 * @dependency IProviderFactory for hardware provider creation
 * @ownership Created managers are returned as unique_ptr (except singletons)
 * @error_handling All methods return nullptr on failure with error logging
 * 
 * @architecture_notes
 * - Receives IProviderFactory for provider creation
 * - Creates and wires all manager dependencies
 * - Enables test injection through IProviderFactory interface
 * - Manages singleton instances appropriately
 * 
 * @usage
 * @code
 * // Production
 * auto providerFactory = std::make_unique<ProviderFactory>();
 * auto managerFactory = std::make_unique<ManagerFactory>(providerFactory.get());
 * auto panelManager = managerFactory->CreatePanelManager();
 * 
 * // Testing
 * auto mockProviderFactory = std::make_unique<MockProviderFactory>();
 * auto managerFactory = std::make_unique<ManagerFactory>(mockProviderFactory.get());
 * @endcode
 */
class ManagerFactory
{
public:
    /// @brief Constructor with provider factory injection
    /// @param providerFactory Factory for creating hardware providers
    explicit ManagerFactory(IProviderFactory* providerFactory);
    
    /// @brief Default destructor
    ~ManagerFactory() = default;
    
    // Delete copy/move operations
    ManagerFactory(const ManagerFactory&) = delete;
    ManagerFactory& operator=(const ManagerFactory&) = delete;
    ManagerFactory(ManagerFactory&&) = delete;
    ManagerFactory& operator=(ManagerFactory&&) = delete;
    
    // Factory Methods
    
    /// @brief Create PanelManager with all dependencies wired
    /// @return Unique pointer to configured PanelManager instance or nullptr on failure
    /// @note Creates all required providers and services internally
    std::unique_ptr<PanelManager> CreatePanelManager();
    
    /// @brief Create StyleManager with optional theme
    /// @param theme Initial theme to apply (defaults to "DAY")
    /// @return Unique pointer to configured StyleManager instance or nullptr on failure
    std::unique_ptr<StyleManager> CreateStyleManager(const char* theme = nullptr);
    
    /// @brief Create PreferenceManager
    /// @return Unique pointer to configured PreferenceManager instance or nullptr on failure
    std::unique_ptr<PreferenceManager> CreatePreferenceManager();
    
    /// @brief Initialize InterruptManager singleton with dependencies
    /// @return Pointer to configured InterruptManager singleton or nullptr on failure
    /// @note Creates PolledHandler and QueuedHandler with GPIO provider
    InterruptManager* CreateInterruptManager();
    
    /// @brief Initialize ErrorManager singleton
    /// @return Pointer to configured ErrorManager singleton or nullptr on failure
    ErrorManager* CreateErrorManager();
    
    /// @brief Create all managers in correct initialization order
    /// @param[out] panelManager Receives created PanelManager
    /// @param[out] styleManager Receives created StyleManager  
    /// @param[out] preferenceManager Receives created PreferenceManager
    /// @param[out] interruptManager Receives InterruptManager singleton pointer
    /// @param[out] errorManager Receives ErrorManager singleton pointer
    /// @return true if all managers created successfully, false otherwise
    bool CreateAllManagers(
        std::unique_ptr<PanelManager>& panelManager,
        std::unique_ptr<StyleManager>& styleManager,
        std::unique_ptr<PreferenceManager>& preferenceManager,
        InterruptManager*& interruptManager,
        ErrorManager*& errorManager
    );

private:
    /// @brief Register all system interrupts with InterruptManager
    /// @param interruptManager The InterruptManager instance to register with
    void RegisterSystemInterrupts(InterruptManager* interruptManager);
    
    /// @brief Provider factory for creating hardware providers
    IProviderFactory* providerFactory_;
    
    /// @brief Cached providers for manager creation
    std::unique_ptr<IGpioProvider> gpioProvider_;
    std::unique_ptr<IDisplayProvider> displayProvider_;
    std::unique_ptr<IDeviceProvider> deviceProvider_;
    
    /// @brief Get or create GPIO provider
    IGpioProvider* GetGpioProvider();
    
    /// @brief Get or create display provider
    IDisplayProvider* GetDisplayProvider();
    
    /// @brief Get or create device provider
    IDeviceProvider* GetDeviceProvider();
};