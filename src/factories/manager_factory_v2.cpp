#include "factories/manager_factory_v2.h"
#include "managers/panel_manager.h"
#include "managers/style_manager.h"
#include "managers/preference_manager.h"
#include "managers/interrupt_manager.h"
#include "managers/error_manager.h"
#include "interfaces/i_gpio_provider.h"
#include "interfaces/i_display_provider.h"
#include "interfaces/i_device_provider.h"
#include "utilities/interrupt_callbacks.h"
#include <Arduino.h>

#include "esp32-hal-log.h"

ManagerFactory::ManagerFactory(IProviderFactory* providerFactory)
    : providerFactory_(providerFactory)
{
    log_v("ManagerFactory() constructor called");
    if (!providerFactory_)
    {
        log_e("ManagerFactory created with null IProviderFactory");
    }
}

std::unique_ptr<PanelManager> ManagerFactory::CreatePanelManager()
{
    log_v("CreatePanelManager() called");
    
    // Get required providers
    auto* gpio = GetGpioProvider();
    auto* display = GetDisplayProvider();
    
    if (!gpio || !display)
    {
        log_e("Failed to create PanelManager - missing required providers");
        return nullptr;
    }
    
    // Get required services
    auto styleService = CreateStyleManager();
    auto preferenceService = CreatePreferenceManager();
    
    if (!styleService || !preferenceService)
    {
        log_e("Failed to create PanelManager - missing required services");
        return nullptr;
    }
    
    // Get InterruptManager singleton
    auto* interruptManager = &InterruptManager::Instance();
    
    try
    {
        auto panelManager = std::make_unique<PanelManager>(
            display, 
            gpio, 
            styleService.get(),
            preferenceService.get(),
            interruptManager
        );
        
        if (panelManager)
        {
            // Release ownership of services to PanelManager
            styleService.release();
            preferenceService.release();
            
            panelManager->Init();
            log_d("Successfully created and initialized PanelManager");
            return panelManager;
        }
    }
    catch (const std::exception& e)
    {
        log_e("Exception creating PanelManager: %s", e.what());
    }
    
    return nullptr;
}

std::unique_ptr<StyleManager> ManagerFactory::CreateStyleManager(const char* theme)
{
    log_v("CreateStyleManager() called with theme: %s", theme ? theme : "default");
    
    try
    {
        auto styleManager = std::make_unique<StyleManager>();
        if (styleManager)
        {
            styleManager->Init(theme);
            log_d("Successfully created and initialized StyleManager");
            return styleManager;
        }
    }
    catch (const std::exception& e)
    {
        log_e("Exception creating StyleManager: %s", e.what());
    }
    
    return nullptr;
}

std::unique_ptr<PreferenceManager> ManagerFactory::CreatePreferenceManager()
{
    log_v("CreatePreferenceManager() called");
    
    try
    {
        auto preferenceManager = std::make_unique<PreferenceManager>();
        if (preferenceManager)
        {
            preferenceManager->Init();
            log_d("Successfully created and initialized PreferenceManager");
            return preferenceManager;
        }
    }
    catch (const std::exception& e)
    {
        log_e("Exception creating PreferenceManager: %s", e.what());
    }
    
    return nullptr;
}

InterruptManager* ManagerFactory::CreateInterruptManager()
{
    log_v("CreateInterruptManager() called");
    
    auto* gpio = GetGpioProvider();
    if (!gpio)
    {
        log_e("Failed to create InterruptManager - GPIO provider is null");
        return nullptr;
    }
    
    try
    {
        auto& interruptManager = InterruptManager::Instance();
        interruptManager.Init(gpio);
        
        // Register system interrupts
        RegisterSystemInterrupts(&interruptManager);
        
        log_d("Successfully initialized InterruptManager singleton");
        return &interruptManager;
    }
    catch (const std::exception& e)
    {
        log_e("Exception initializing InterruptManager: %s", e.what());
    }
    
    return nullptr;
}

ErrorManager* ManagerFactory::CreateErrorManager()
{
    log_v("CreateErrorManager() called");
    
    try
    {
        auto& errorManager = ErrorManager::Instance();
        log_d("Successfully obtained ErrorManager singleton");
        return &errorManager;
    }
    catch (const std::exception& e)
    {
        log_e("Exception obtaining ErrorManager: %s", e.what());
    }
    
    return nullptr;
}

bool ManagerFactory::CreateAllManagers(
    std::unique_ptr<PanelManager>& panelManager,
    std::unique_ptr<StyleManager>& styleManager,
    std::unique_ptr<PreferenceManager>& preferenceManager,
    InterruptManager*& interruptManager,
    ErrorManager*& errorManager)
{
    log_v("CreateAllManagers() called");
    
    // Create managers in dependency order
    errorManager = CreateErrorManager();
    if (!errorManager)
    {
        log_e("Failed to create ErrorManager");
        return false;
    }
    
    styleManager = CreateStyleManager();
    if (!styleManager)
    {
        log_e("Failed to create StyleManager");
        return false;
    }
    
    preferenceManager = CreatePreferenceManager();
    if (!preferenceManager)
    {
        log_e("Failed to create PreferenceManager");
        return false;
    }
    
    interruptManager = CreateInterruptManager();
    if (!interruptManager)
    {
        log_e("Failed to create InterruptManager");
        return false;
    }
    
    panelManager = CreatePanelManager();
    if (!panelManager)
    {
        log_e("Failed to create PanelManager");
        return false;
    }
    
    log_i("Successfully created all managers");
    return true;
}

void ManagerFactory::RegisterSystemInterrupts(InterruptManager* interruptManager)
{
    log_v("RegisterSystemInterrupts() called");
    
    if (!interruptManager)
    {
        log_e("Cannot register interrupts - InterruptManager is null");
        return;
    }
    
    // Register all system interrupts using the InterruptCallbacks utility
    InterruptCallbacks::RegisterAllInterrupts(*interruptManager);
    
    log_d("Successfully registered all system interrupts");
}

IGpioProvider* ManagerFactory::GetGpioProvider()
{
    if (!gpioProvider_ && providerFactory_)
    {
        gpioProvider_ = providerFactory_->CreateGpioProvider();
    }
    return gpioProvider_.get();
}

IDisplayProvider* ManagerFactory::GetDisplayProvider()
{
    if (!displayProvider_ && providerFactory_)
    {
        displayProvider_ = providerFactory_->CreateDisplayProvider();
    }
    return displayProvider_.get();
}

IDeviceProvider* ManagerFactory::GetDeviceProvider()
{
    if (!deviceProvider_ && providerFactory_)
    {
        deviceProvider_ = providerFactory_->CreateDeviceProvider();
    }
    return deviceProvider_.get();
}